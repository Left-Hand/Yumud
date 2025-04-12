#include "src/testbench/tb.h"

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

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

scexpr auto UARTSW_BAUD = 38400; 
scexpr auto U13T_BAUD = 9600; 




namespace gxm{


class DelayedSemphr final{
public:
    DelayedSemphr(const uint32_t delay_ms):
        delay_ms_(delay_ms){;}
    void give(){
        last_millis_ = millis();
    }

    bool take(){
        if(last_millis_ and last_millis_.value() + delay_ms_ < millis()){
            last_millis_ = std::nullopt;
            return true;
        }
        return false;
    }
    private:
        immutable_t<uint32_t> delay_ms_ = 0;
        std::optional<uint32_t> last_millis_ = std::nullopt;
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

    static constexpr size_t STAT_COUNT = __END;
    static constexpr size_t STR_LEN = 16;
    constexpr size_t to_index() const {
        return size_t(kind_);
    }

    constexpr Kind kind() const {
        return kind_;
    }

    static constexpr
    Option<StationName> from_gbk(std::span<const uint8_t, STR_LEN> code);
private:
    Kind kind_;
};

struct StationData final{
    static constexpr size_t STAT_COUNT = StationName::STAT_COUNT;
    static constexpr size_t STR_LEN = StationName::STR_LEN;

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

static constexpr auto TABLE = StationData::calc_hash_table(StationData::DATA);

constexpr
Option<StationName> StationName::from_gbk(std::span<const uint8_t, STR_LEN> code){
    const auto code_hash = StationData::calc_hash_of_line(code);

    for(size_t i = 0; i < TABLE.size(); i++)
        if(TABLE[i] == code_hash) return Some(StationName(static_cast<Kind>(i)));
    return None;
}


class MotorTask final{
public:
    using Chopper = drivers::AT8222;

    struct Config{
        //开始缓启动的时间
        real_t start_time; // S

        //缓启动加速的时间
        real_t accelrate_time; // S

        //缓启动最终恒定输出的力
        real_t final_torque; // N / m 
    };

    class CurrentSensor final{
    public:
        CurrentSensor(
            AnalogInIntf & a_sense
        ):  
            a_sense_(a_sense){;}

        void update(){

        };
    private:
        AnalogInIntf & a_sense_;
    };

    class Motor final{
    public:
        Motor(Chopper & drv, AnalogInIntf & a_sense):
            drv_(drv),
            cs_(a_sense)
            {;}

        void set_torque(const real_t torque){
            static constexpr auto TORQUE_2_CURRENT_RATIO = 1.0_r;
            set_current(torque * TORQUE_2_CURRENT_RATIO);
        }

        void set_current(const real_t current){
            static constexpr auto CURRENT_2_DUTY_RATIO = 1.0_r;
            set_duty(current * CURRENT_2_DUTY_RATIO);
        }
        
        void set_duty(const real_t duty){
            drv_ = CLAMP(duty, 0, 1);
        }

        void tick(){

        }
    private:
        Chopper & drv_;
        CurrentSensor cs_;
    };

    MotorTask(const Config & cfg){
        reconf(cfg);
    }

    void reconf(const Config & cfg){
        start_time_ = cfg.start_time; // S
        accelrate_time_ = cfg.accelrate_time; // S
        final_torque_ = cfg.final_torque; // N / m 
    }

    void init(){

        //因为是中心对齐的顶部触发 所以频率翻倍
        timer.init(10'000, TimerMode::CenterAlignedUpTrig);

        pwm_pos.init();
        pwm_neg.init();
    
        pwm_pos.set_sync();
        pwm_neg.set_sync();
    
        pwm_pos.set_polarity(true);
        pwm_neg.set_polarity(true);
    }

    void process(const real_t t){
        const auto torque = CLAMP(
            (t - start_time_) * final_torque_ / accelrate_time_, 
            0, final_torque_);

        motor_.set_torque(torque);
    }

    void tick(){
        motor_.tick();
    }
    
private:
    hal::GenericTimer & timer = hal::timer3;
            
    hal::TimerOC & pwm_pos = timer.oc(1);
    hal::TimerOC & pwm_neg = timer.oc(2);

    Chopper motdrv{
        pwm_pos, pwm_neg, hal::NullGpio
    };

    Motor motor_{motdrv, hal::adc1.inj(1)};

    //开始缓启动的时间
    real_t start_time_; // S

    //缓启动加速的时间
    real_t accelrate_time_; // S

    //缓启动最终恒定输出的力
    real_t final_torque_; // N / m 
};


class BoardcastTask final{
public:
    using Inst = drivers::JQ8900;

    BoardcastTask(Gpio & gpio):
        inst_(gpio){
    };

    void init(){
        inst_.init();
        inst_.set_vol(23);
    }

    void play(const StationName & sta){
        inst_.play_disc(sta.to_index() + 1);
    }
private:
    Inst inst_;
};

class DetectTask final{
private:
    using Inst = drivers::U13T;
    hal::UartHw & uart_;
    Inst inst_ = Inst{uart_};
    Option<StationName> last_sta_ = None;

    //防止短期内多次触发 设定最小死区时间
    static constexpr uint32_t DEAD_ZONE_MS = 1500;

    //上次播放的时间 提供死区参考
    uint32_t last_detected_ms_ = 0;

    class FrameDecoder final{
    public:
        enum class State{
            Idle,
            Len,
            Payload
        };

        static constexpr uint8_t HEADER_TOKEN = 0x7f;
        static constexpr size_t MAX_PAYLOAD_SIZE = 32;

        FrameDecoder(){;}
        void feed(const char chr){
            switch(state_){
                case State::Idle:
                    if(chr == HEADER_TOKEN){
                        state_ = State::Len;
                    }
                    break;
                case State::Len:{
                    const size_t len = chr;
                    ASSERT(len <= MAX_PAYLOAD_SIZE, "FrameDecoder: len too large");
                    len_ = len;
                    payload_buffer_.clear();
                    state_ = State::Payload;
                    break;
                }
                case State::Payload:
                    payload_buffer_.push_back(chr);
                    if(payload_buffer_.size() >= len_){
                        state_ = State::Idle;
                    }
                    break;
                default:
                    break;
            }
        }

        std::optional<std::span<const uint8_t>> get_payload() const{
            if(payload_buffer_.size() < len_) return {};
            return std::span(payload_buffer_.begin(), payload_buffer_.size());
        }
    private:
        State state_ = State::Idle;
        sstl::vector<uint8_t, MAX_PAYLOAD_SIZE> payload_buffer_{};
        uint8_t len_ = 0;
    };

    FrameDecoder frame_decoder_{};
    DelayedSemphr semphr_{50};

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

            if(millis() > last_detected_ms_ + DEAD_ZONE_MS){
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
};

class StatLed{
public:
    StatLed(hal::GpioIntf & gpio):gpio_(gpio){;}

    void init(){
        gpio_.outpp();
    }

    StatLed & operator =(const Color_t<real_t> & color){
        if(bool(color)) gpio_.set();
        else gpio_.clr();
        return *this;
    }
private:
    hal::GpioIntf & gpio_;
};
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


[[maybe_unused]] static 
void motor_tb(){
    hal::UartSw uart{portA[5], NullGpio}; 
    uart.init(UARTSW_BAUD);

    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();

    timer1.init(UARTSW_BAUD);
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        uart.tick();
    });

    gxm::MotorTask motor_task{{
        //开始缓启动的时间
        .start_time = 0.3_r, // S

        //缓启动加速的时间
        .accelrate_time = 3.0_r, // S

        //缓启动最终恒定输出的力
        .final_torque = 0.6_r, // N / m 
    }};

    motor_task.init();

    while(true){
        motor_task.process(time());
        delay(1);
    }
}


[[maybe_unused]] static
void detect_tb(){

    hal::UartSw uart{portA[5], NullGpio}; 
    uart.init(UARTSW_BAUD);

    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();

    timer1.init(UARTSW_BAUD);
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        uart.tick();
    });

    auto & UARTHW = uart2;
    uart2.init(U13T_BAUD);
    drivers::U13T u13t{UARTHW};
    gxm::DetectTask detect_task{UARTHW};
    

    detect_task.init();

    while(true){
        detect_task.process(time());
        delay(1);
    }
}

[[maybe_unused]] static
void boardcast_tb(){
    hal::UartSw uart{portA[5], NullGpio}; 
    uart.init(UARTSW_BAUD);

    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();

    timer1.init(UARTSW_BAUD);
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        uart.tick();
    });

    gxm::BoardcastTask boardcast_task{portB[1]};
    boardcast_task.init();

    while(true){
        boardcast_task.play(gxm::StationName::ChiShui);
        delay(1);
    }
}
[[maybe_unused]] static
void app(){
    auto & UARTSW_GPIO = portA[5];
    auto & UARTHW = uart2;

    hal::UartSw uart{UARTSW_GPIO, NullGpio}; 
    uart.init(UARTSW_BAUD);

    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();
    DEBUGGER.set_splitter(',');

    drivers::AbEncoderByGpio ab_enc{portA[0], portA[1]};

    timer1.init(UARTSW_BAUD);
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        uart.tick();
        ab_enc.update();
    });

    
    gxm::MotorTask motor_task{{
        //开始缓启动的时间
        .start_time = 0.3_r, // S

        //缓启动加速的时间
        .accelrate_time = 3.0_r, // S

        //缓启动最终恒定输出的力
        .final_torque = 0.97_r, // N / m 
    }};
    motor_task.init();



    UARTHW.init(U13T_BAUD);
    drivers::U13T u13t{UARTHW};
    gxm::DetectTask detect_task{UARTHW};
    detect_task.init();

    gxm::BoardcastTask boardcast_task{portB[1]};
    boardcast_task.init();

    gxm::StatLed led{portB[8]};
    led.init();


    delay(10);

    bindSystickCb([&]{
        const auto t = time();
        motor_task.process(t);
    });



    while(true){
        const auto t = time();

        detect_task.process(t);

        detect_task.get_station().inspect([&](auto && station_name){
            led = Color_t<real_t>(1, 1, 1, 1);

            boardcast_task.play(station_name);
            DEBUG_PRINTLN(station_name);

            delay(500);
            led = Color_t<real_t>(0, 0, 0, 0);

            detect_task.clear();
        });

        DEBUG_PRINTLN(ab_enc.get_cnt(), millis(), ab_enc.get_err_cnt(), ab_enc.get_code());
        delay(1);
    }
}

void gxm_new_energy_main(){
    app();
}