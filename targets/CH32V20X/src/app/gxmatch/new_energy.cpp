#include "src/testbench/tb.h"

#include <queue>

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/string/StringView.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/timer.hpp"
#include "hal/bus/uart/uartsw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/adc/adcs/adc1.hpp"

#include "drivers/CommonIO/Led/WS2812/ws2812.hpp"
#include "drivers/Actuator/Bridge/AT8222/at8222.hpp"
#include "drivers/Audio/JQ8900/JQ8900.hpp"
#include "drivers/Recognition/U13T/U13T.hpp"
#include "drivers/Encoder/ABEncoder.hpp"

#include "core/utils/hash_func.hpp"
#include "core/utils/immutable.hpp"
#include "core/utils/Option.hpp"

#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

static constexpr auto DBG_UARTSW_BAUD = 38400; 
static constexpr auto TTS_UARTSW_BAUD = 9600; 
static constexpr auto U13T_BAUD = 115200; 

static constexpr size_t STAT_COUNT = 16;
static constexpr size_t STR_LEN = 16;

//防止短期内多次触发 设定最小死区时间
static constexpr uint32_t DETECT_DEAD_ZONE_MS = 300;
static constexpr size_t RESPONSE_DELAY_MS = 0;

static constexpr uint8_t U13T_HEADER_TOKEN = 0x7f;
static constexpr size_t U13T_MAX_PAYLOAD_SIZE = 32;

static constexpr size_t BLINK_DUR_MS = 60;
static constexpr size_t POWERON_BLINK_TIMES = 2;
static constexpr real_t LED_SUSTAIN_TIME_S = 0.5_r;
static constexpr real_t LED_BLANKING_TIME_S = 0.1_r;

static constexpr size_t MOTOR_ISR_FREQ = 5000;
static constexpr real_t SAMPLE_RES = 0.005_r;
static constexpr real_t INA240_BETA = 100;
static constexpr real_t VOLT_BAIS = 1.65_r;

static constexpr real_t MIN_DUTY = 0.1_r;
static constexpr real_t MAX_DUTY = 0.94_r;
static constexpr real_t PI_KP = 10.6_r;

static constexpr uint CURRENT_LPF_CUTOFF_FREQ_HZ = 10;

static constexpr auto PERIOD_PER_CHAR = 0.2_r;


namespace motorctl{

template<size_t Q>
static constexpr iq_t<Q> tpzpu(const iq_t<Q> x){
    return abs(4 * frac(x - iq_t<Q>(0.25)) - 2) - 1;
}

real_t volt_2_current(real_t volt){
    static constexpr auto INV_SCALE = 1 / (SAMPLE_RES * INA240_BETA);
    return (volt - VOLT_BAIS) *INV_SCALE;
}

class CurrentSensor final{
public:
    using Filter = dsp::LowpassFilter_t<q20>;
    using Config = Filter::Config;
    CurrentSensor(
        AnalogInIntf & a_sense
    ):  
        a_sense_(a_sense){;}

    void update(){
        lpf_.update(volt_2_current(real_t(a_sense_)));
        current_ = lpf_.get();
    };

    const real_t & get() const{
        return current_;
    }
private:
    AnalogInIntf & a_sense_;
    real_t current_ = 0;

    static constexpr Config CFG = {
        .fc = CURRENT_LPF_CUTOFF_FREQ_HZ,
        .fs = MOTOR_ISR_FREQ
    };

    Filter lpf_{CFG};
};

class Motor final{
public:
public:
    using Chopper = drivers::AT8222;
    using Sensor = CurrentSensor;

    Motor(Chopper & drv, CurrentSensor & cs):
        drv_(drv),
        cs_(cs)
        {;}

    void set_torque(const real_t torque){
        static constexpr auto TORQUE_2_CURRENT_RATIO = 1.0_r;
        set_current(torque * TORQUE_2_CURRENT_RATIO);
    }

    void set_current(const real_t current){
        targ_current_ = current;
    }
    
    void update(){
        cs_.update();
        const auto meas_current = cs_.get();
        ctrl_.update(targ_current_, meas_current);
        // set_duty(ctrl_.get());
        set_duty(0.7_r);
    }

    const auto & get_duty() const {
        return ctrl_.get();
    }

private:
    void set_duty(const real_t duty){
        drv_ = CLAMP(duty, MIN_DUTY, MAX_DUTY);
    }

    using Ctrl = dsp::DeltaPController;
    using CtrlCfg = Ctrl::Config;

    static constexpr CtrlCfg PI_CFG = {
        .kp = PI_KP,
        .out_min = MIN_DUTY,
        .out_max = MAX_DUTY,
        .fs = MOTOR_ISR_FREQ
    };

    Chopper & drv_;
    CurrentSensor & cs_;

    real_t targ_current_ = 0;
    Ctrl ctrl_{PI_CFG};
};

}

namespace gxm{


class DelayedSemphr final{
public:
    DelayedSemphr(const uint32_t delay_ms):
        delay_ms_(delay_ms){;}
    void give(){
        last_millis_ = Some(millis());
    }

    bool take(){
        if(last_millis_.is_some() and last_millis_.unwrap() + delay_ms_ < millis()){
            last_millis_ = None;
            return true;
        }
        return false;
    }
    private:
        immutable_t<uint32_t> delay_ms_ = 0;
        Option<uint32_t> last_millis_ = None;
};

struct StationName final{
    enum Kind:uint8_t {
        RuiJin = 0,
        FengSuoXian,
        XiangJiang,
        WuJiang,
        
        ZunYi,
        ChiShui,
        JinShaJiang,
        DaDuHe,
        
        LuDingQiao,
        XueShang,
        MaoGong,
        CaoDi,
        
        LaZiKou,
        HuiNing,
        WuQiZhen,
        YanAn,

        __END
    };

    constexpr StationName(const Kind kind = __END):
        kind_(kind){;}

    constexpr StationName(const StationName & other) = default;
    constexpr StationName(StationName && other) = default;
    constexpr StationName & operator = (const StationName & other) = default;
    constexpr StationName & operator = (StationName && other) = default;


    constexpr size_t to_index() const {
        return size_t(kind_);
    }

    StringView to_gbk_str() const;

    constexpr Kind kind() const {
        return kind_;
    }

    bool operator ==(const Kind kind) const {
        return kind_ == kind;
    }

    static constexpr
    Option<StationName> from_gbk(std::span<const uint8_t, STR_LEN> code);
private:
    Kind kind_;
};

struct StationData final{

    using GbkLine = std::array<uint8_t, STR_LEN>;
    using GbkData = std::array<GbkLine, STAT_COUNT>;
    using HashTable = std::array<uint32_t, STAT_COUNT>;

    static constexpr const GbkData DATA = {
        std::to_array<uint8_t>({0xc8,0xf0,0xbd,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xcd,0xbb,0xc6,0xc6,0xb5,0xda,0xc8,0xfd,0xb5,0xc0,0xb7,0xe2,0xcb,0xf8,0xcf,0xdf}),
        std::to_array<uint8_t>({0xd1,0xaa,0xd5,0xbd,0xcf,0xe6,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xc7,0xbf,0xb6,0xc9,0xce,0xda,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xd5,0xbc,0xc1,0xec,0xd7,0xf1,0xd2,0xe5,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xcb,0xc4,0xb6,0xc9,0xb3,0xe0,0xcb,0xae,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xc7,0xc9,0xb6,0xc9,0xbd,0xf0,0xc9,0xb3,0xbd,0xad,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xc7,0xbf,0xb6,0xc9,0xb4,0xf3,0xb6,0xc9,0xba,0xd3,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xb7,0xc9,0xb6,0xe1,0xe3,0xf2,0xb6,0xa8,0xc7,0xc5,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xc5,0xc0,0xd1,0xa9,0xc9,0xbd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xed,0xae,0xb9,0xa6,0xbb,0xe1,0xca,0xa6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xb9,0xfd,0xb2,0xdd,0xb5,0xd8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xbc,0xa4,0xd5,0xbd,0xc0,0xb0,0xd7,0xd3,0xbf,0xda,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xbb,0xe1,0xc4,0xfe,0xb4,0xf3,0xbb,0xe1,0xca,0xa6,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xce,0xe2,0xc6,0xf0,0xd5,0xf2,0xbb,0xe1,0xd2,0xe9,0x00,0x00,0x00,0x00,0x00,0x00}),
        std::to_array<uint8_t>({0xd1,0xd3,0xb0,0xb2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),
    };

    static constexpr 
    uint32_t calc_hash_of_line(std::span<const uint8_t, STR_LEN> line){
        return Hasher::hash_djb(
            (&line[0]), 
            line.size()
        );
    }

    static constexpr 
    std::array<uint32_t, STAT_COUNT> calc_hash_table(const GbkData & data){
        using Ret = std::array<uint32_t, STAT_COUNT>;

        Ret ret;
        for(size_t i = 0; i < STAT_COUNT; ++i){
            const auto & line = data[i];
            ret[i] = calc_hash_of_line(line);
        }
        return ret;
    }
};

template<typename T>
struct StationDict{

    using Data = std::array<T, STAT_COUNT>;

    T & operator [](const StationName & name){
        return data_[name.to_index()];
    }

    const T & operator [](const StationName & name) const {
        return data_[name.to_index()];
    }
private:
    Data data_ = {0};
};

static constexpr auto TABLE = StationData::calc_hash_table(StationData::DATA);
StringView StationName::to_gbk_str() const {
    const auto idx = this->to_index();
    const auto line = reinterpret_cast<const char *>(static_cast<const uint8_t *>(StationData::DATA[idx].data()));
    const auto line_len = strnlen(line, StationData::DATA[idx].size());
    return StringView(line, line_len);
}
constexpr
Option<StationName> StationName::from_gbk(std::span<const uint8_t, STR_LEN> code){
    const auto code_hash = StationData::calc_hash_of_line(code);

    for(size_t i = 0; i < TABLE.size(); i++)
        if(TABLE[i] == code_hash) return Some(StationName(static_cast<Kind>(i)));
    return None;
}


class MotorTask final{
public:
    using Motor = motorctl::Motor;
    using Chopper = Motor::Chopper;
    using Sensor = Motor::Sensor;

    struct Config{
        //开始缓启动的时间
        real_t start_time; // S

        //缓启动加速的时间
        real_t accelerate_time; // S

        //缓启动最终恒定输出的力
        real_t final_torque; // N / m 
    };



    MotorTask(const Config & cfg){
        reconf(cfg);
    }

    const auto & get_duty() const {
        return motor_.get_duty();
    }

    const auto & get_current() const {
        return sensor_.get();
    }

    void reconf(const Config & cfg){
        start_time_ = cfg.start_time; // S
        accelerate_time_ = cfg.accelerate_time; // S
        final_torque_ = cfg.final_torque; // N / m 
    }

    void init(){

        //因为是中心对齐的顶部触发 所以频率翻倍
        timer.init(MOTOR_ISR_FREQ * 2, TimerCountMode::CenterAlignedUpTrig);

        pwm_pos.init();
        pwm_neg.init();
    
        pwm_pos.set_sync();
        pwm_neg.set_sync();
    
        pwm_pos.set_polarity(false);
        pwm_neg.set_polarity(false);

        
        adc1.init(
            {
                {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
            },{
                {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            }
        );

        adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
        adc1.enable_auto_inject(false);

        timer3.set_trgo_source(TimerTrgoSource::OC4R);

        timer3.oc(4).init(TimerOcMode::UpValid, false)
            .set_output_state(true)
            .set_idle_state(false);

        timer3.oc(4).cvr() = int(1);
    }

    void process(const real_t t){
        const auto torque = CLAMP(
            (t - start_time_) * final_torque_ / accelerate_time_, 
            0, final_torque_);

        motor_.set_torque(torque);
    }

    void tick(){
        motor_.update();
    }
    
private:
    hal::GenericTimer & timer = hal::timer3;
            
    hal::TimerOC & pwm_pos = timer.oc(1);
    hal::TimerOC & pwm_neg = timer.oc(2);

    Chopper chopper_{
        pwm_pos, pwm_neg, hal::NullGpio
    };

    Sensor sensor_{hal::adc1.inj(1)};

    Motor motor_{chopper_, sensor_};

    //开始缓启动的时间
    real_t start_time_; // S

    //缓启动加速的时间
    real_t accelerate_time_; // S

    //缓启动最终恒定输出的力
    real_t final_torque_; // N / m 
};

class CnTTS final{
public:
    CnTTS(hal::Uart & uart):uart_(uart){;}

    void play_disc(const StationName sta){
        const auto str = sta.to_gbk_str();
        uart_.writeN(str.data(), str.size());
    }

    void set_volume(const size_t vol){
        //pass
    }
private:
    hal::Uart & uart_;

    void play_gbk_str(const StringView str){
        uart_.writeN(str.data(), str.size());
    }
};
class BoardcastTask final{
public:
    // using Inst = drivers::JQ8900;
    using Inst = CnTTS;
    using Item = std::pair<StationName, real_t>;
    using StationQueue = std::queue<Item>;

    BoardcastTask(Inst & inst):
        inst_(inst){
    };

    void add_play(const StationName & sta){
        play_list_.push(name_2_item(sta));
    }

    void process(const real_t t){
        if(playing_.is_none()){
            push_next(t);
        }else{
            if(playing_.unwrap().is_timeout(t)){
                push_next(t);
            }
        }
    }
private:
    Inst inst_;

    static Item name_2_item(const StationName name){
        const auto str_len = name.to_gbk_str().size();
        const auto time = str_len * PERIOD_PER_CHAR;
        return {name, time}; 
    }

    void push_next(const real_t t){
        if(play_list_.empty()) return;
        const auto front = play_list_.front();
        playing_ = Some(Playing{
            front.first, front.second, t
        });
        inst_.play_disc(playing_.unwrap().name);
        play_list_.pop();
    }

    StationQueue play_list_;
    struct Playing{
        StationName name;
        real_t sustain;
        real_t start_time;
        real_t has_been_played(const real_t t) const {
            return t - start_time;
        }

        bool is_timeout(const real_t t) const {
            return has_been_played(t) > sustain;
        }
    };

    Option<Playing> playing_ = None;
};

class DetectTask final{
private:
    using Inst = drivers::U13T;
    hal::UartHw & uart_;
    // Inst inst_ = Inst{uart_};
    Option<StationName> last_sta_ = None;

    //上次播放的时间 提供死区参考
    uint32_t last_detected_ms_ = 0;

    class FrameDecoder final{
    public:
        enum class State{
            Idle,
            Len,
            Payload
        };

        FrameDecoder(){;}
        void feed(const char chr){
            switch(state_){
                case State::Idle:
                    if(chr == U13T_HEADER_TOKEN){
                        state_ = State::Len;
                    }
                    break;
                case State::Len:{
                    const size_t len = chr;
                    if(len > U13T_MAX_PAYLOAD_SIZE){
                        end_packet();
                    }else{
                        len_ = len;
                        payload_buffer_.clear();
                        state_ = State::Payload;
                    }
                    break;
                }
                case State::Payload:
                    payload_buffer_.push_back(chr);
                    if(payload_buffer_.size() >= len_){
                        end_packet();
                    }
                    break;
                default:
                    break;
            }
        }

        void end_packet(){
            state_ = State::Idle;
        }

        std::optional<std::span<const uint8_t>> get_payload() const{
            if(payload_buffer_.size() < len_) return {};
            return std::span(payload_buffer_.begin(), payload_buffer_.size());
        }
    private:
        State state_ = State::Idle;
        sstl::vector<uint8_t, U13T_MAX_PAYLOAD_SIZE> payload_buffer_{};
        uint8_t len_ = 0;
    };

    FrameDecoder frame_decoder_{};
    DelayedSemphr semphr_{RESPONSE_DELAY_MS};

    Option<StationName> match_context(std::span<const uint8_t, 16> data) const {
        return StationName::from_gbk(data);
    }

    static constexpr auto is_pkt_payload(const std::span<const uint8_t> pdata){
        return pdata.size() == 10;
    }

    static constexpr auto is_identity_payload(const std::span<const uint8_t> pdata){
        return pdata.size() == 26;
    }

    void update_decoder(){
        while(uart_.available()){
            char chr;
            uart_.read1(chr);
            frame_decoder_.feed(chr);
        }
    }

    void on_payload(const std::span<const uint8_t> payload){
        if(is_pkt_payload(payload)){
            semphr_.give();
        }else if(is_identity_payload(payload)){
            const auto line = (std::span(payload).subspan<9,16>());

            if(millis() > last_detected_ms_ + DETECT_DEAD_ZONE_MS){
                last_detected_ms_ = millis();
                last_sta_ = match_context(line);
            }
        }
    }


public: 
    DetectTask(UartHw & uart):
        uart_(uart){
    }

    void init(){
    }

    void process(const real_t t){
        if(semphr_.take()){
            static constexpr auto cmds = std::to_array<char>({0x7f, 0x04, 0x00, 0x11, 0x04, 0x11});
            uart_.writeN(cmds.begin(), cmds.size());
        }

        if(!uart_.available()) return;
        update_decoder();
        
        const auto payload_opt = frame_decoder_.get_payload();
        if(!payload_opt) return;
        on_payload(payload_opt.value());
    }

    Option<StationName> get_station() {
        return last_sta_;
    }

    void clear(){
        last_sta_ = None;
    }

    void end_packet(){
        frame_decoder_.end_packet();
    }
};

class StatLed{
public:

    StatLed(hal::GpioIntf & gpio):gpio_(gpio){;}

    void init(){
        gpio_.outpp();
    }

    void blink(const size_t times){
        for(size_t i = 0; i < times; i++){
            on();
            delay(BLINK_DUR_MS);
            off();
            delay(BLINK_DUR_MS);
        }
    }

    void on(){
        gpio_.set();
    }

    void off(){
        gpio_.clr();
    }

private:
    hal::GpioIntf & gpio_;
};

class LedTask{
public:
    LedTask(StatLed & led):
        led_(led){;}

    void add_point(const real_t t){
        invoke_t_ = Some(t);
    }

    void process(const real_t t){
        if(!invoke_t_.is_some()) return;
        const auto dt = t - invoke_t_.unwrap();
        if(dt < LED_BLANKING_TIME_S) led_.off();
        else if(dt > LED_SUSTAIN_TIME_S) led_.off();
        else led_.on();
    }
private:
    StatLed & led_;
    Option<real_t> invoke_t_ = None;
};
}



[[maybe_unused]] static
void app(){
    auto & DBG_UARTSW_GPIO = portA[5];
    auto & TTS_UARTSW_GPIO = portB[1];
    auto & LED_GPIO = portB[8];
    auto & UARTHW = uart2;

    hal::UartSw dbg_uart{DBG_UARTSW_GPIO, NullGpio}; 
    hal::UartSw tts_uart{TTS_UARTSW_GPIO, NullGpio}; 

    UARTHW.init(U13T_BAUD);
    dbg_uart.init(DBG_UARTSW_BAUD);
    tts_uart.init(TTS_UARTSW_BAUD);

    gxm::CnTTS tts{tts_uart};

    DEBUGGER.retarget(&dbg_uart);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();
    DEBUGGER.set_splitter(',');

    drivers::AbEncoderByGpio ab_enc{portA[0], portA[1]};

    
    gxm::MotorTask motor_task{{
        //开始缓启动的时间
        .start_time = 0.3_r, // S

        //缓启动加速的时间
        .accelerate_time = 1.0_r, // S

        //缓启动最终恒定输出的力
        .final_torque = 0.07_r, // N / m 
    }};

    motor_task.init();


    // drivers::U13T u13t{UARTHW};
    gxm::DetectTask detect_task{UARTHW};
    UARTHW.bind_post_rx_cb([&](){
        detect_task.end_packet();
    });

    detect_task.init();

    gxm::BoardcastTask boardcast_task{tts};
    // boardcast_task.init();

    gxm::StatLed led{LED_GPIO};
    led.init();
    led.blink(POWERON_BLINK_TIMES);
    gxm::LedTask led_task{led};

    timer1.init(DBG_UARTSW_BAUD);
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        motor_task.tick();
        dbg_uart.tick();
        ab_enc.update();
    });

    timer2.init(TTS_UARTSW_BAUD);
    timer2.attach(TimerIT::Update, {0,0}, [&]{
        tts_uart.tick();
    });

    adc1.attach(AdcIT::JEOC, {0,0}, [&](){
        motor_task.tick();
    });

    
    bindSystickCb([&]{
        const auto t = time();
        motor_task.process(t);
    });
    
    gxm::StationDict<bool> played{};
    DEBUG_PRINTLN("app start");
    delay(10);

    while(true){
        const auto t = time();

        boardcast_task.process(t);
        detect_task.process(t);
        led_task.process(t);

        detect_task.get_station().inspect([&](const gxm::StationName name){
            if(played[name] == true) return;
            played[name] = true;

            if(name == gxm::StationName::ChiShui || name == gxm::StationName::HuiNing){
                boardcast_task.add_play(name);
            }
            led_task.add_point(t);
            detect_task.clear();
        });

        DEBUG_PRINTLN(motor_task.get_current(), motor_task.get_duty());

        delay(1);
    }
}

void gxm_new_energy_main(){
    app();
}



namespace ymd{
    OutputStream & operator << (OutputStream & os, const gxm::StationName & sta){
        switch(sta.kind()){
            case gxm::StationName::RuiJin: return os << "RuiJin"; 
            case gxm::StationName::FengSuoXian: return os << "FengSuoXian"; 
            case gxm::StationName::XiangJiang: return os << "XiangJiang"; 
            case gxm::StationName::WuJiang: return os << "WuJiang"; 
            case gxm::StationName::ZunYi: return os << "ZunYi"; 
            case gxm::StationName::ChiShui: return os << "ChiShui"; 
            case gxm::StationName::JinShaJiang: return os << "JinShaJiang"; 
            case gxm::StationName::DaDuHe: return os << "DaDuHe"; 
            case gxm::StationName::LuDingQiao: return os << "LuDingQiao"; 
            case gxm::StationName::XueShang: return os << "XueShang"; 
            case gxm::StationName::MaoGong: return os << "MaoGong";
            case gxm::StationName::CaoDi: return os << "CaoDi";
            case gxm::StationName::LaZiKou: return os << "LaZiKou";
            case gxm::StationName::HuiNing: return os << "HuiNing";
            case gxm::StationName::WuQiZhen: return os << "WuQiZhen";
            case gxm::StationName::YanAn: return os << "YanAn";
            default: __builtin_unreachable();
        }
    }
}
