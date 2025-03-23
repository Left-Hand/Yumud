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

using namespace ymd::hal;

#define TARG_UART hal::uart2



static constexpr size_t ISR_FREQ = 19200;
static constexpr real_t SAMPLE_RES = 0.1_r;
static constexpr real_t INA240_BETA = 100;
static constexpr real_t VOLT_BAIS = 1.65_r;

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


class EdgeCounter{
public:
    void update(bool state){
        if(state != state_){
            count_ ++;
        }
        state_ = state;
    }

    auto count() const{
        return count_;
    }
private:
    size_t count_ = 0;
    bool state_ = false;
};

struct SpeedEstimator{
public:
    struct Config{
        real_t err_threshold = 0.02_r;
        size_t est_freq = ISR_FREQ;
        size_t max_cycles = ISR_FREQ >> 7;
    };

    Config config = Config();
protected:

    struct Vars{
        real_t last_position;
        real_t last_raw_speed;
        real_t last_speed;
        size_t cycles;
        void reset(){
            last_position = 0;
            last_raw_speed = 0;
            last_speed = 0;
            cycles = 1;
        }
    };

    Vars vars;


    real_t update_raw(const real_t position){
        real_t delta_pos = position - vars.last_position;
        real_t abs_delta_pos = ABS(delta_pos);
        real_t this_speed = (delta_pos * int(config.est_freq) / int(vars.cycles));
    
        if(abs_delta_pos > config.err_threshold){
        
            vars.cycles = 1;
            vars.last_position = position;
            return vars.last_raw_speed = this_speed;
        }else{
            vars.cycles++;
            if(vars.cycles > config.max_cycles){
                
                vars.cycles = 1;
                vars.last_position = position;
                return vars.last_raw_speed = this_speed;
            }
        }
    
        return vars.last_raw_speed;
    }
    
public:
    SpeedEstimator(){
        reset();
    }

    void reset(){
        vars.reset();
    }

    void update(const real_t position){
        auto this_spd = update_raw(position);
        vars.last_speed = (vars.last_speed * 127 + this_spd) >> 7;
        // vars.last_speed = (vars.last_speed * 63 + this_spd) >> 6;
    }
    real_t get() const {return vars.last_speed;} 
};

class myPIController{
public:
    struct Config{
        q24 kp;
        q24 ki;
        q24 out_min;
        q24 out_max;
        uint fs;
    };
protected:
    q24 kp_;
    q24 ki_by_fs_;
    q24 out_min_;
    q24 out_max_;

    q24 i_out_;
    q24 output_;
public:
    myPIController(const Config & cfg){
        reconf(cfg);
        reset();    
    }

    void reset(){
        i_out_ = 0;
        output_ = out_min_;
    }
    void reconf(const Config & cfg){
        kp_ = cfg.kp;
        ki_by_fs_ = cfg.ki / cfg.fs;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
    }

    static q24 calc_forward_feedback(const q24 targ){
        return sqrt((targ + 5.3_q24) * q24(1.0 / 18));
    }
    
    void update(const q24 targ, const q24 meas){
        const q24 err = targ - meas;

        // const q24 p_out = kp_ * err;

        // if(unlikely(p_out >= out_max_)){
        //     i_out_ = 0;
        //     output_ = out_max_;
        //     return;
        // }else if(unlikely(p_out <= out_min_)){
        //     i_out_ = 0;
        //     output_ = out_min_;
        //     return;
        // }else{
            // i_out_ = CLAMP(i_out_ + err * ki_by_fs_, (out_min_ - p_out), (out_max_- p_out));
            i_out_ = CLAMP(i_out_ + err * ki_by_fs_, -1, 1);
            // output_ = CLAMP(p_out + i_out_, out_min_, out_max_);
            output_ = CLAMP(i_out_ + calc_forward_feedback(targ), out_min_, out_max_);
            // output_ = CLAMP(i_out_, out_min_, out_max_);
            return;
        // }
    }

    real_t get() const {
        return output_;
    }
};

class IController{
public:
    struct Config{
        q24 ki;
        q24 out_min;
        q24 out_max;
        uint fs;
    };
protected:
    q24 ki_by_fs_;
    q24 out_min_;
    q24 out_max_;
    q24 output_;
public:
    IController(const Config & cfg){
        reconf(cfg);
        reset();
    }


    void reset(){
        output_ = out_min_;
    }

    void reconf(const Config & cfg){
        ki_by_fs_ = cfg.ki / cfg.fs;
        out_min_ = cfg.out_min;
        out_max_ = cfg.out_max;
    }

    void update(const q24 targ, const q24 meas){
        const q24 err = targ - meas;

        const auto temp_output = output_ + ki_by_fs_ * err;
        output_ = CLAMP(temp_output, out_min_, out_max_);
    }

    q24 output() const {
        return output_;
    }
};

template<typename T>
class immutable_t{
public:
    constexpr immutable_t(const auto & value):
        value_(static_cast<T>(value)){;}

    constexpr immutable_t(auto && value):
        value_(static_cast<T>(value)){;}

    constexpr operator const T () const {
        return value_;
    }

    constexpr const T & get() const {
        return value_;
    }

    constexpr T & borrow_mut(){
        return value_;
    }
private:
    T value_;
};

class SlidingModeController {
public:
    SlidingModeController(){}
    void update(const q24 targ,const q24 meas) {
        static constexpr q24 c = 0.06_q24;
        static constexpr q24 q = 0.0006_q24;

        const q24 x1 = targ - meas;
        const q24 x2 = x1 - err_prev_.get();
        err_prev_.borrow_mut() = x1;

        const q24 s = c * x1 + x2;
        const q24 delta = c * x2 + q * s; 

        output_.borrow_mut() = CLAMP(output_.get() + delta, out_min_.get(), out_max_.get());
    }

    auto get() const {return output_.get();}

private:
    immutable_t<q24> output_  = 0;
    
    immutable_t<q24> out_min_ = 0.7_q24;
    immutable_t<q24> out_max_ = 0.9_q24;

    immutable_t<q24> err_prev_ = 0;
};


[[maybe_unused]] static void at8222_tb(){
    // hal::UartSw uart{portA[5], NullGpio}; uart.init(19200);
    // DEBUGGER.retarget(&uart);

    // TARG_UART.init(6_MHz);

    auto & timer = hal::timer3;

    //因为是中心对齐的顶部触发 所以频率翻倍
    timer.init(ISR_FREQ * 2, TimerMode::CenterAlignedDualTrig);

    auto & pwm_pos = timer.oc(1);
    auto & pwm_neg = timer.oc(2);

    
    pwm_pos.init();
    pwm_neg.init();

    pwm_pos.sync();
    pwm_neg.sync();
    
    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
        }
    );

    adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
    adc1.enable_auto_inject(false);

    timer.set_trgo_source(TimerTrgoSource::OC4R);

    timer.oc(4).init(TimerOcMode::UpValid, false)
        .set_output_state(true)
        .set_idle_state(false);

    timer.oc(4).cvr() = timer.arr() - 1;
    
    LowpassFilter lpf{LowpassFilter::Config{
        // .fc = 220,
        .fc = 2000,
        .fs = ISR_FREQ
    }};

    
    LowpassFilter lpf_mid{LowpassFilter::Config{
        .fc = 140,
        .fs = ISR_FREQ
    }};

    BandpassFilter bpf{BandpassFilter::Config{
        // .fl = 300,
        // .fh = 800,
        .fl = 150,
        .fh = 400,
        .fs = ISR_FREQ
    }};

    SpeedEstimator spe = SpeedEstimator();

    EdgeCounter ect;
    
    real_t curr = 0;
    real_t curr_mid = 0;

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

    SlidingModeController pi_ctrl;


    volatile uint32_t exe_micros = 0;
    real_t spd_targ = 0;
    adc1.attach(AdcIT::JEOC, {0,0}, [&](){
        const auto begin_micros = micros();
        const auto volt = adc1.inj(1).get_voltage();
        const auto curr_raw = volt_2_current(volt);

        lpf.update(curr_raw);
        // lpf_mid.update(curr_raw);
        curr = lpf.result();

        bpf.update(curr);
        curr_mid = lpf_mid.result();

        ect.update(bool(bpf.result() > 0));
        spe.update(ect.count() * 0.01_r);
        pi_ctrl.update(spd_targ, spe.get());
        pwm_pos = pi_ctrl.get();
        exe_micros = micros() - begin_micros;

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
        spd_targ = 8.0_r + 1.0_r * sinpu(2.0_r * time());
        // spd_targ = 7.0_r + 1.0_r * sign(sin(2.0_r * time()));
        // spd_targ = 9.0_r + 1.0_r * ((sin(1.0_r * time())) > 0 ? 1 : ;
        // spd_targ = 9.0_r + 1.0_r * -1;
        // spd_targ = 16.57_r;
        DEBUG_PRINTLN_IDLE(spd_targ, curr * 10, curr_mid * 10, pi_ctrl.get(), bpf.result() * 10, spe.get(), bool(bpf.result() > 0), exe_micros);
        // DEBUG_PRINTLN(duty, bool(pwm_pos.io()), bool(pwm_neg.io()));
        
    }
}

void ws2812_main(){
    TARG_UART.init(6_MHz);
    DEBUGGER.retarget(&TARG_UART);
    // ws2812_tb(hal::portB[1]);
    at8222_tb();
}