
#include "src/testbench/tb.h"

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "drivers/CommonIO/Led/WS2812/ws2812.hpp"
#include "drivers/Modem/dshot/dshot.hpp"

#include "drivers/Actuator/Bridge/AT8222/at8222.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"

#include "dsp/filter/homebrew/DigitalFilter.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/homebrew/edge_counter.hpp"
#include "dsp/controller/sliding_mode_ctrl.hpp"

#include "drivers/Audio/JQ8900/JQ8900.hpp"
#include "drivers/Recognition/U13T/U13T.hpp"
#include "core/utils/hash_func.hpp"

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
            FengSuoXian = 1,
            XiangJiang = 2,
            WuJiang = 3,
            
            ZunYi = 4,
            ChiShui = 5,
            JinShaJiang = 6,
            DaDuHe = 7,
            
            LuDingQiao = 8,
            XueShang = 9,
            MaoGong = 10,
            CaoDi = 11,
            
            LaZiKou = 12,
            HuiNing = 13,
            WuQiZhen = 14,
            YanAn = 15,

            __END
        };

        StationName(const Kind kind):
            kind_(kind){;}

        static constexpr size_t STAT_COUNT = __END;
        static constexpr size_t STR_LEN = 16;
        constexpr size_t to_index() const {
            return size_t(kind_);
        }

        constexpr Kind kind() const {
            return kind_;
        }

        constexpr 
        std::span<const uint8_t, STR_LEN> to_gbk() const;
    
        static constexpr
        std::optional<StationName> from_gbk(std::span<const uint8_t, STR_LEN> code);
    private:
        const Kind kind_;
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



    constexpr 
    std::span<const uint8_t, StationName::STR_LEN> StationName::to_gbk() const{
        return std::span<const uint8_t, STR_LEN>(StationData::DATA[to_index()]);
    }

    constexpr
    std::optional<StationName> StationName::from_gbk(std::span<const uint8_t, STR_LEN> code){
        constexpr auto TABLE = StationData::calc_hash_table(StationData::DATA);
        const auto code_hash = StationData::calc_hash_of_line(code);

        for(size_t i = 0; i < TABLE.size(); i++)
            if(TABLE[i] == code_hash) return StationName(static_cast<Kind>(i));
        return std::nullopt;
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
            real_t final_force; // N / m 
        };
    
        class Motor final{
        public:
            Motor(Chopper & drv):
                drv_(drv){;}
    
            void set_force(const real_t force){
                drv_ = force;
            }
        private:
            Chopper & drv_;
        };
    
        MotorTask(const Config & cfg){
            reconf(cfg);
        }
    
        void reconf(const Config & cfg){
            //开始缓启动的时间
            start_time_ = cfg.start_time; // S
    
            //缓启动加速的时间
            accelrate_time_ = cfg.accelrate_time; // S
    
            //缓启动最终恒定输出的力
            final_force_ = cfg.final_force; // N / m 
        }
    
        void init(){
    
            //因为是中心对齐的顶部触发 所以频率翻�?
            timer.init(20'000, TimerMode::CenterAlignedUpTrig);
    
            
            pwm_pos.init();
            pwm_neg.init();
        
            pwm_pos.sync();
            pwm_neg.sync();
        
            pwm_pos.set_polarity(true);
            pwm_neg.set_polarity(true);
        }
    
        void process(const real_t t){
            const auto force = CLAMP(
                (t - start_time_) * final_force_ / accelrate_time_, 
                0, final_force_);

            motor_.set_force(force);
        }
        
    private:
        hal::GenericTimer & timer = hal::timer3;
                
        hal::TimerOC & pwm_pos = timer.oc(1);
        hal::TimerOC & pwm_neg = timer.oc(2);
    
        Chopper motdrv{
            pwm_pos, pwm_neg, hal::NullGpio
        };

        Motor motor_{motdrv};
    
        //开始缓启动的时间
        real_t start_time_; // S
    
        //缓启动加速的时间
        real_t accelrate_time_; // S
    
        //缓启动最终恒定输出的力
        real_t final_force_; // N / m 
    };
    

    class BoardcastTask final{
    public:
        using Inst = drivers::JQ8900;

        BoardcastTask(Gpio & gpio):
            inst_(gpio){
        };

        void init(){
            inst_.init();
            inst_.set_vol(3);
            // inst_.set_disc(1);
        }

        void play(const StationName & sta){
            inst_.set_disc(sta.to_index());
            // DEBUG_PRINTS("play:", sta);
        }
    private:
        Inst inst_;
    };

    class DetectTask final{
    private:
        using Inst = drivers::U13T;
        hal::UartHw & uart_;
        Inst inst_ = Inst{uart_};


        DelayedSemphr semphr_{50};

        std::optional<StationName> match_context(std::span<const uint8_t, 16> data) const {
            // DEBUG_PRINTS("station is: ", gxm::StationName::from_gbk(data));
            return StationName::from_gbk(data);
        }

        static constexpr auto is_pkt_response(const std::span<const uint8_t> pdata){
            return pdata.size() == 12;
        }

        static constexpr auto is_payload_response(const std::span<const uint8_t> pdata){
            return pdata.size() == 28;
        }

    public: 
        DetectTask(UartHw & uart):
            uart_(uart){
        }

        void init(){
            Color_t<real_t> color;
            drivers::WS2812 led{portB[1]};
            led.init();
        }

        void rx_callback(){

            std::vector<uint8_t> line;
            while(uart_.available()){
                char chr;
                uart_.read1(chr);
                line.push_back(chr);
            }

            if(is_pkt_response(line)){
                semphr_.give();
            }else if(is_payload_response(line)){
                const auto payload = std::span<const uint8_t, 16>(std::span(line).subspan(11,16));
                
                DEBUG_PRINTLN(match_context(payload), payload);
                // match_context(std::span(StationData::DATA[1]));
            }

        };

        void test(){
            std::array<uint8_t, 16> ret;
            if(true){//mock data
            // if(false){//mock data
                std::fill(ret.begin(),ret.end(),0);
                const auto & data = StationData::DATA[1];
                std::copy(data.begin(),data.end(), ret.begin());
                // ret[0] = 0xd1;
                // ret[1] = 0xd3;
                // ret[2] = 0xb0;
                // ret[3] = 0xb2;
            }

            match_context(std::span(ret));
        }

        void process(const real_t t){
            if(semphr_.take()){
                static constexpr auto cmds = std::to_array<char>({0x7f, 0x04, 0x00, 0x11, 0x04, 0x11});
                uart_.writeN(cmds.begin(), cmds.size());
            }
        }
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
        .final_force = 0.6_r, // N / m 
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

    auto & uarthw = uart2;
    uart2.init(U13T_BAUD);
    drivers::U13T u13t{uarthw};
    gxm::DetectTask detect_task{uarthw};
    
    uarthw.bind_post_rx_cb([&]{
        detect_task.rx_callback();
    });

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
    scexpr auto UARTSW_BAUD = 38400; 

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
        .final_force = 0.6_r, // N / m 
    }};
    motor_task.init();



    auto & uarthw = uart2;
    uart2.init(U13T_BAUD);
    drivers::U13T u13t{uarthw};
    gxm::DetectTask detect_task{uarthw};
    detect_task.init();

    uarthw.bind_post_rx_cb([&]{
        detect_task.rx_callback();
    });

    gxm::BoardcastTask boardcast_task{portB[1]};
    boardcast_task.init();

    // boardcast_task.play(gxm::StationName::ChiShui);
    while(true){
        const auto t = time();
        motor_task.process(t);
        detect_task.process(t);
        delay(1);
    }
}

void gxm_new_energy_main(){


    // while(true){
    //     DEBUG_PRINTLN(1243550000,1243550000);
    //     delay(1);
    // }
    // ws2812_tb(hal::portB[1]);
    // at8222_tb();
    // motor_tb();
    // detect_tb();
    // boardcast_tb();
    app();
    // jq8900_tb();
    // u13t_tb();
}