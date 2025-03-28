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

using namespace ymd::hal;

#define TARG_UART hal::uart2



static constexpr size_t ISR_FREQ = 19200;
static constexpr real_t SAMPLE_RES = 0.1_r;
static constexpr real_t INA240_BETA = 100;
static constexpr real_t VOLT_BAIS = 1.65_r;

template<size_t Q>
static constexpr iq_t<Q> tpzpu(const iq_t<Q> x){
    return abs(4 * frac(x - iq_t<Q>(0.25)) - 2) - 1;
}

real_t volt_2_current(real_t volt){
    static constexpr auto INV_SCALE = 1 / (SAMPLE_RES * INA240_BETA);
    return (volt - VOLT_BAIS) *INV_SCALE;
}


[[maybe_unused]] static void ws2812_tb(hal::GpioIntf & gpio){
    drivers::WS2812 led{gpio};
    led.init();
    while(true){
        led = Color_t<real_t>::from_hsv(0.5_r + 0.5_r * sin(time()),1,1,0.2_r);
        DEBUG_PRINTLN(millis());
        delay(10);
    }
}

using LowpassFilter = dsp::ButterLowpassFilter<q16, 2>;
using HighpassFilter = dsp::ButterHighpassFilter<q16, 2>;
using BandpassFilter = dsp::ButterBandpassFilter<q16, 4>;



// class ExtendedStateObserver{

//     void update(const real_t u, const real_t y){
//         const real_t e = z1 - y;


//         const real_t z1  = state_[0];
//         const real_t z2  = state_[1];
//         const real_t z3  = state_[2];

//         state_[0] = z1 + h *(z2-belta01*e);
//         state_[1] = z2 + h *(z3-belta02*fal(e,0.5,delta)+b*u);
//         state_[2] = z3 + h *(-belta03*fal(e,0.25,delta));
//     }
// private:
//     using State = StateVector<q20, 3>;
//     State state_;
// }




[[maybe_unused]] static void at8222_tb(){
    // hal::UartSw uart{portA[5], NullGpio}; uart.init(19200);
    // DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();

    // TARG_UART.init(6_MHz);

    auto & timer = hal::timer3;

    //因为是中心对齐的顶部触发 所以频率翻�?
    timer.init(ISR_FREQ * 2, TimerMode::CenterAlignedUpTrig);

    auto & pwm_pos = timer.oc(1);
    auto & pwm_neg = timer.oc(2);

    
    pwm_pos.init();
    pwm_neg.init();

    pwm_pos.sync();
    pwm_neg.sync();

    pwm_pos.set_polarity(true);
    pwm_neg.set_polarity(true);
    
    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
        }
    );

    adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    adc1.enable_auto_inject(false);
    adc1.set_pga(AdcPga::X16);

    timer.set_trgo_source(TimerTrgoSource::OC4R);

    timer.oc(4).init(TimerOcMode::UpValid, false)
        .set_output_state(true)
        .set_idle_state(false);

    // timer.oc(4).cvr() = timer.arr() - 1; 
    // timer.oc(4).cvr() = int(timer.arr() * 0.1_r); 
    timer.oc(4).cvr() = int(1); 
    
    LowpassFilter lpf{LowpassFilter::Config{
        .fc = 140,
        .fs = ISR_FREQ
    }};

    
    // LowpassFilter lpf_mid{LowpassFilter::Config{
    //     .fc = 140,
    //     .fs = ISR_FREQ
    // }};

    BandpassFilter bpf{BandpassFilter::Config{
        // .fl = 300,
        // .fh = 800,
        .fl = 150,
        .fh = 400,
        .fs = ISR_FREQ
    }};


    dsp::EdgeCounter ect;
    
    real_t curr = 0;
    [[maybe_unused]]real_t curr_mid = 0;

    // IController pi_ctrl{
    //     IController::Config{
    //         .ki = 0.4_r,
    //         .out_min = 0.7_r,
    //         .out_max = 0.97_r,
    //         .fs = ISR_FREQ
    //     }
    // };

    // myPIController pi_ctrl{
    //     myPIController::Config{
    //         .kp = 0.0_r,
    //         .ki = 0.2_r,
    //         .out_min = 0.7_r,
    //         .out_max = 0.97_r,
    //         .fs = ISR_FREQ
    //     }
    // };

    // dsp::SlidingModeController pi_ctrl{{
    //     .c = 0.16_q24,
    //     .q = 0.0005_q24,
    //     // .c = 01.6_q24,
    //     // .q = 0.0000005_q24,

    //     .out_min = 0.7_r,
    //     .out_max = 0.97_r,

    //     .fs = ISR_FREQ
    // }};
    
    dsp::DeltaPdController pi_ctrl{{
        .kp = 0.8_r,
        .kd = 0.00_r,

        .out_min = 0.7_r,
        .out_max = 0.97_r,

        .fs = ISR_FREQ
    }};

    dsp::TrackingDifferentiatorByOrders<2> td{{
        // .r = 14.96_r,
        // .r = 7.9_r,
        // .r = 7.99_r,
        // .r = 48.00_r,
        // .r = 38.00_r,
        .r = 125.5_r,
        // .r = 6.5_r,
        .fs = ISR_FREQ
    }};

    // volatile uint32_t exe_micros = 0;
    real_t spd_targ = 0;
    real_t pos_targ = 0;

    real_t trackin_sig = 0;
    real_t volt = 0;

    auto & watch_gpio = portA[3];
    watch_gpio.outpp();

    adc1.attach(AdcIT::JEOC, {0,0}, [&](){
        watch_gpio.toggle();
        volt = adc1.inj(1).get_voltage();
        const auto curr_raw = volt_2_current(volt);

        lpf.update(curr_raw);
        curr = lpf.get();
        watch_gpio.toggle();
        // bpf.update(curr_raw);

        bpf.update(curr_raw);
        // curr_mid = lpf_mid.get();

        ect.update(bool(bpf.get() > 0));

        const auto pos = ect.count() * 0.01_r;
        td.update(pos);
        const auto spd = td.get()[1];

        // static constexpr auto kp = 267.0_r;
        // static constexpr auto kd = 0.0_r;
        // const auto spd_cmd = kp * (pos_targ - pos) + kd * (spd_targ - spd);
        pi_ctrl.update(spd_targ, spd);
        // pwm_pos = pi_ctrl.get();
        // pwm_pos = 0.87_r * abs(sinpu(time()));
        // pwm_pos = 0.7_r + 0.17_r * abs(sinpu(time()));
        // pwm_pos = 0.13_r + 0.817_r * abs(sinpu(time()));
        pwm_pos = 0.8_r;
        pwm_neg = 0_r;



        // uart.tick();
    });


    drivers::AT8222 motdrv{
        pwm_pos, pwm_neg, hal::NullGpio
    };

    motdrv.init();


    while(true){
        // const auto duty = sin(4 * time()) * 0.2_r + 0.7_r;
        // motdrv = duty;
        // pwm_neg = 0.5_r;
        // pwm_pos = ABS(duty);

        // pwm_pos = 0.9_r + 0.1_r * sin(5 * time());
        // spd_targ = 7.0_r + 3 * sin(5 * time());
        // spd_targ = 8.0_r + 1.0_r * ((sin(2.0_r * time())) > 0 ? 1 : -1);
        // spd_targ = 8.0_r + 1.0_r * sinpu(2.0_r * time());
        const auto t = time();

        #define TEST_MODE 1

        #if TEST_MODE == 0
        spd_targ = 12;
        pos_targ = 10.0_r * t + 2*frac(t);
        #elif TEST_MODE == 1
        spd_targ = 7.0_r + 1.0_r * sinpu(1.3_r * t);
        pos_targ = 7.0_r * t + real_t(-1.0/6) * cospu(1.3_r * t);
        #endif
        // spd_targ = 9.0_r + 1.0_r * ((sin(1.0_r * time())) > 0 ? 1 : ;
        // spd_targ = 9.0_r + 1.0_r * -1;
        // spd_targ = 16.57_r;
        // trackin_sig = sign(sin(t * 3));
        // trackin_sig = real_t(int(sin(t * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * sin(t * 3) * 32)) / 32;
        // trackin_sig = real_t(int(0.2_r * t * 32)) / 32;
        // trackin_sig = 1/(1 + exp(4 * tpzpu(3 * t)));
        trackin_sig = 10 * CLAMP2(sinpu(7 * t), 0.5_r);
        // trackin_sig = tpzpu(t);
        
        // DEBUG_PRINTLN_IDLE(pos_targ, spd_targ, bpf.get(), volt, pi_ctrl.get(), bpf.get(), , exe_micros);
        // DEBUG_PRINTLN_IDLE(td.get(), lpf.get() * 90 ,volt,spd_targ, exe_micros);
        DEBUG_PRINTLN_IDLE(curr * 100, bpf.get(), volt);
        // DEBUG_PRINTLN_IDLE(trackin_sig, td.get());
        // DEBUG_PRINTLN(bool(pwm_pos.io()), bool(pwm_neg.io()));
        
    }
}

#include "drivers/Audio/JQ8900/JQ8900.hpp"
#include "drivers/Recognition/U13T/U13T.hpp"
#include "core/utils/hash_func.hpp"

namespace gxm{
    struct StationData{
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

        static constexpr size_t STAT_COUNT = __END;
        static constexpr size_t STR_LEN = 16;

        // bool match_line(const )
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

        static constexpr uint32_t calc_hash_of_line(std::span<const uint8_t, STR_LEN> line){
            return Hasher::hash_djb(
                (&line[0]), 
                line.size()
            );
        }

        static constexpr std::array<uint32_t, STAT_COUNT> calc_hash_table(const GbkData & data){
            using Ret = std::array<uint32_t, STAT_COUNT>;

            Ret ret;
            for(size_t i = 0; i < STAT_COUNT; ++i){
                const auto & line = data[i];
                ret[i] = calc_hash_of_line(line);
            }
            return ret;
        }
    };

    class Station final:public StationData{
    public:
        constexpr Station(Kind name):name_(name){;}

        constexpr size_t to_index() const {
            return size_t(name_);
        }

        constexpr std::span<const uint8_t, STR_LEN> to_gbk() const{
            return std::span<const uint8_t, STR_LEN>(DATA[to_index()]);
        }

        static constexpr
        std::optional<Station> from_gbk(std::span<const uint8_t, STR_LEN> code){
            constexpr auto TABLE = StationData::calc_hash_table(StationData::DATA);
            const auto code_hash = StationData::calc_hash_of_line(code);

            for(size_t i = 0; i < TABLE.size(); i++)
                if(TABLE[i] == code_hash) return Station(static_cast<Kind>(i));
            return std::nullopt;
        }
    public:
        const Kind name_;
    };
    
    class SatationBoardcaster final{
    public:
        using Inst = drivers::JQ8900;
        SatationBoardcaster(Inst & inst):
            inst_(inst){
        };

        void play(const Station & sta){
            inst_.set_disc(sta.to_index());
        }
    private:
        Inst & inst_;
    };

    class StationDetector final{
    private:
        using Inst = drivers::U13T;
        Inst & inst_;
    public: 
        StationDetector(Inst & inst):
            inst_(inst){
        }

        void init(){

        }
    };
}
namespace ymd{
    OutputStream & operator << (OutputStream & os, const gxm::Station & sta){
        return os << sta.to_index();
    }
}

void jq8900_tb(){

    TARG_UART.init(6_MHz, CommStrategy::Nil);
    // TARG_UART.init(576000);
    DEBUGGER.retarget(&TARG_UART);

    auto & tx = portB[1];

    // auto & tx = portA[1];
    drivers::JQ8900 jq{tx};
    // auto & pwm_timer = timer4;
    // pwm_timer.init(1000);
    // pwm_timer.oc(3).init();
    // pwm_timer.oc(3) = 0.3_r;
    // while(true){
    //     DEBUG_PRINTLN(millis());
    // }

    tx.outpp(HIGH);
    // tx.outod(HIGH);

    // timer1.init(20000);
    // timer1.attach(TimerIT::Update, {0,0}, [&]{
    //     DEBUG_PRINTLN(bool(tx));
    // });

    jq.init();
    jq.set_vol(3);
    jq.set_disc(1);
    drivers::WS2812 led{tx};
    led.init();
    uint8_t i = 0;
    while(true){
        // delay(2000);
        led = Color_t<real_t>::from_hsv(0.5_r + 0.5_r * sin(time()),1,1,0.2_r);
        tx.set();
        delay(80);
        jq.set_disc(i = (i+ 1) % 14);
    }
}

void u13t_tb(){
    hal::uart2.init(9600);

    hal::UartSw uart{portA[5], NullGpio}; uart.init(19200);
    DEBUGGER.retarget(&uart);
    DEBUGGER.no_brackets();
    
    Color_t<real_t> color;
    drivers::WS2812 led{portB[1]};
    led.init();

    timer1.init(9600);
    timer1.attach(TimerIT::Update, {0,0}, [&](){
        uart.tick();
    });
    bool cmd_en = false;
    hal::uart2.bind_post_rx_cb([&]{
        led = Color_t<real_t>::from_hsv(0.5_r + 0.5_r * sin(time()),1,1,0.2_r);

        std::vector<uint8_t> ret;
        while(uart2.available()){
            char chr;
            uart2.read1(chr);
            ret.push_back(chr);
        }
        DEBUG_PRINTS(std::hex, std::showbase, ret);
        // 0xd1,0xd3,0xb0,0xb2
        std::fill(ret.begin(),ret.end(),0);
        ret[0] = 0xd1;
        ret[1] = 0xd3;
        ret[2] = 0xb0;
        ret[3] = 0xb2;
        if(ret.size() > 6)cmd_en = true;
        if(ret.size() > 16){
            const auto data = std::span<const uint8_t, 16>(std::span<const uint8_t>(ret).subspan(11,16));
            DEBUG_PRINTS("station is: ", gxm::Station::from_gbk(data));
        }
    });

    while(true){
        // delay(2000);
        // tx.set();
        delay(400);
        if(cmd_en){
            static constexpr auto cmds = std::to_array<char>({0x7f, 0x04, 0x00, 0x11, 0x04, 0x11});
            uart2.writeN(cmds.begin(), cmds.size());
            cmd_en = false;
        }
        // DEBUG_PRINTLN(millis());
        // jq.set_disc(i = (i+ 1) % 14);
    }
}

void ws2812_main(){


    // while(true){
    //     DEBUG_PRINTLN(1243550000,1243550000);
    //     delay(1);
    // }
    // ws2812_tb(hal::portB[1]);
    // at8222_tb();
    // jq8900_tb();
    u13t_tb();
}