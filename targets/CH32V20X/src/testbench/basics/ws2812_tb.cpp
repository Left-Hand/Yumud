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
        static constexpr q24 c = 0.12_q24;
        static constexpr q24 q = 0.0004_q24;
        static constexpr q24 e = 0.000027_q24;

        const q24 x1 = targ - meas;
        const q24 x2 = x1 - err_prev_.get();
        err_prev_.borrow_mut() = x1;

        const q24 s = c * x1 + x2;
        const q24 delta = c * x2 + q * s + e * sign(s); 

        output_.borrow_mut() = CLAMP(output_.get() + delta, out_min_.get(), out_max_.get());
    }

    auto get() const {return output_.get();}

private:
    immutable_t<q24> output_  = 0;
    
    immutable_t<q24> out_min_ = 0.7_q24;
    immutable_t<q24> out_max_ = 0.9_q24;

    immutable_t<q24> err_prev_ = 0;
};

static constexpr real_t ssqrt(real_t x){
    return sign(x) * sqrt(ABS(x));
}



real_t fal(const real_t e, const real_t alpha, const real_t delta){

    const auto fabsf_e = abs(e);
    
    if(delta>=fabsf_e)
        return e/pow(delta,1-alpha);
    else
        return pow(fabsf_e,alpha)*sign(e);
}


template<typename T, size_t N>
class StateVector{
public:
    StateVector(){
        reset();
    }

    void reset(){
        // data_.fill(T(0));
        for(size_t i=0;i<N;i++) data_[i] = 0;
    }

    StateVector(const std::initializer_list<T> list){
        std::copy(list.begin(), list.end(), data_.begin());
    }

    const T & operator [](const size_t idx){
        return data_[idx];
    }

    StateVector & operator += (const StateVector & other){
        for(size_t i=0;i<N;i++)
            data_[i] += other.data_[i];
        return *this;
    }

    const auto & get() const{return data_;}
    const T & back() const{return data_.back();}
private:    
    std::array<T, N> data_;
};

class TrackingDifferentiator{
public:
    struct Config{
        q24 h;
        q24 r;
        uint fs;
    };

    TrackingDifferentiator(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reset(){
        state_.reset();
    }


    void update(const q24 v){
        const auto h = h_.get();
        const auto r = r_.get();

        const auto x1 = state_[0];
        const auto x2 = state_[1];

        state_ += {
            h * x2, h * fhan(x1 - v,x2,r, h)
        };
    }

    void update2(const q24 v){
        const auto h = h_.get();

        const auto x1 = state_[0];
        const auto x2 = state_[1];

        state_ += {
            h * x2, h * fhan2(x1 - v,x2)
        };
    }
    
    static constexpr q24 fhan(q24 x1,q24 x2,q24 r,q24 h){

        const auto deltaa = r*h;
        const auto deltaa0 = deltaa*h;
        const auto y = x1 + (x2*h);
        const auto a0 = sqrt(deltaa*deltaa+8*r*abs(y));
        const auto a = ((abs(y)<=deltaa0) ? (x2+y/h) : (x2+0.5_r*(a0-deltaa)*sign(y)));
    
        if(abs(a)<=deltaa)
            return -r*a/deltaa;
        else
            return -r*sign(a);
    }

    __fast_inline
    constexpr q24 fhan2(q24 x1,q24 x2) const{
        const auto deltaa_squ = square(deltaa_.get());
        const auto r = r_.get();
        const auto inv_h = inv_h_.get();

        const auto y = x1 + x2 * h_.get();
        const auto a0 = sqrt(deltaa_squ + 8 * r * abs(y));
        const auto a = ((abs(y)<=deltaa0_.get()) ? 
            (x2+y * inv_h) : 
            (x2+0.5_r*(a0-deltaa_.get())*sign(y)));
    
        if(abs(a)<=deltaa_.get())
            return -a * inv_h;
        else
            return -r*sign(a);
    }

    const auto & get() const {return state_.get();}


    void reconf(const Config & cfg){
        h_.borrow_mut() = cfg.h;
        r_.borrow_mut() = cfg.r;

        deltaa_.borrow_mut() = cfg.r*cfg.h;
        deltaa0_.borrow_mut() = cfg.r*cfg.h*cfg.h;
        inv_h_.borrow_mut() = 1/cfg.h;
        // d_lmt_.borrow_mut() = - cfg.r 
    }
private:
    immutable_t<q24> h_ = 0;
    immutable_t<q24> r_ = 0;
    
    immutable_t<q24> deltaa_ = 0;
    immutable_t<q24> deltaa0_ = 0;
    immutable_t<q24> inv_h_ = 0;
    // immutable_t<q24> d_lmt_ = 0;

    using State = StateVector<q24, 2>;
    State state_;
};


template<size_t N>
class TrackingDifferentiatorByOrders;

template<>
class TrackingDifferentiatorByOrders<2>{
public:
    struct Config{
        q24 r;
        uint fs;
    };

    TrackingDifferentiatorByOrders(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reset(){
        state_.reset();
    }


    void update(const q24 u){
        const auto r = r_.get();
        const auto r_squ = r * r;
        const auto inv_fs = inv_fs_.get();

        const auto x1 = state_[0];
        const auto x2 = state_[1];

        state_ += {
            x2 * inv_fs, 
            (- 2 * r * x2 - r_squ * (x1 - u)) * inv_fs
        };
    }


    void reconf(const Config & cfg){
        r_.borrow_mut() = cfg.r;
        inv_fs_.borrow_mut() = 1_q24 / cfg.fs;
    }
private:
    immutable_t<q24> r_ = 0;
    immutable_t<q24> inv_fs_ = 0;

    using State = StateVector<q24, 2>;
    State state_;
};

template<>
class TrackingDifferentiatorByOrders<3>{
public:
    struct Config{
        q24 r;
        uint fs;
    };

    TrackingDifferentiatorByOrders(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reset(){
        state_.reset();
    }


    void update(const q24 u){
        const auto r = r_.get();
        const auto r_squ = r * r;
        const auto inv_fs = inv_fs_.get();

        const auto x1 = state_[0];
        const auto x2 = state_[1];
        const auto x3 = state_[2];

        state_ += {
            x2 * inv_fs, 
            x3 * inv_fs,
            (- 3 * x3 - 3 * r * x2 - r_squ * (x1 - u)) * r * inv_fs
        };
    }


    void reconf(const Config & cfg){
        r_.borrow_mut() = cfg.r;
        inv_fs_.borrow_mut() = 1_q24 / cfg.fs;
    }
private:
    immutable_t<q24> r_ = 0;
    immutable_t<q24> inv_fs_ = 0;

    using State = StateVector<q24, 3>;
    State state_;
};


template<>
class TrackingDifferentiatorByOrders<4>{
public:
    struct Config{
        q24 r;
        uint fs;
    };

    TrackingDifferentiatorByOrders(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reset(){
        state_.reset();
    }


    void update(const q24 u){
        const auto r = r_.get();
        const auto r_squ = r * r;
        const auto inv_fs = inv_fs_.get();

        const auto x1 = state_[0];
        const auto x2 = state_[1];
        const auto x3 = state_[2];
        const auto x4 = state_[3];

        state_ += {
            x2 * inv_fs, 
            x3 * inv_fs,
            x3 * inv_fs,
            (-4 * x4 - 6 * x3 - 4 * r * x2 - r_squ * r * (x1 - u)) * r * inv_fs
        };
    }


    void reconf(const Config & cfg){
        r_.borrow_mut() = cfg.r;
        inv_fs_.borrow_mut() = 1_q24 / cfg.fs;
    }

    const auto & back() const {return state_.back();}
private:
    immutable_t<q24> r_ = 0;
    immutable_t<q24> inv_fs_ = 0;

    using State = StateVector<q24, 4>;
    State state_;
};




[[maybe_unused]] static void at8222_tb(){
    // hal::UartSw uart{portA[5], NullGpio}; uart.init(19200);
    // DEBUGGER.retarget(&uart);
    DEBUGGER.noBrackets();

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
    TrackingDifferentiator td{{
        .h = 0.002_q24,
        .r = 5.96_r
    }};

    volatile uint32_t exe_micros = 0;
    real_t spd_targ = 0;
    real_t pos_targ = 0;

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

        const auto pos = ect.count() * 0.01_r;
        spe.update(pos);
        const auto spd = spe.get();

        static constexpr auto kp = 87.0_r;
        static constexpr auto kd = 0.0_r;
        const auto spd_cmd = kp * ssqrt(pos_targ - pos) + kd * (spd_targ - spd);
        pi_ctrl.update(spd_cmd, spd);
        pwm_pos = pi_ctrl.get();
        exe_micros = micros() - begin_micros;

        const auto t = time();
        const auto s = sin(t * 10);
        td.update(s);
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

        #define TEST_MODE 0

        #if TEST_MODE == 0
        spd_targ = 12;
        pos_targ = 10.0_r * t + 2*frac(t);
        #elif TEST_MODE == 1
        spd_targ = 7.0_r + 1.0_r * sin(6 * t);
        pos_targ = 7.0_r * t + real_t(-1.0/6) * cos(6 * t);
        #endif
        // spd_targ = 9.0_r + 1.0_r * ((sin(1.0_r * time())) > 0 ? 1 : ;
        // spd_targ = 9.0_r + 1.0_r * -1;
        // spd_targ = 16.57_r;
        // DEBUG_PRINTLN_IDLE(pos_targ, spd_targ, curr * 10,  pi_ctrl.get(), bpf.result(), spe.get(), ect.count() * 0.01_r, exe_micros);


        DEBUG_PRINTLN_IDLE(td.get());
        // DEBUG_PRINTLN(duty, bool(pwm_pos.io()), bool(pwm_neg.io()));
        
    }
}

void ws2812_main(){
    TARG_UART.init(6_MHz);
    DEBUGGER.retarget(&TARG_UART);
    // ws2812_tb(hal::portB[1]);
    at8222_tb();
}