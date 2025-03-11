#include "src/testbench/tb.h"

#include "sys/clock/time.hpp"

#include "FFT.hpp"
#include "liir.hpp"
#include "sys/debug/debug.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "ButterSideFilter.hpp"
#include "Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "ParticleSwarmOptimization.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "dsp/filter/LowpassFilter.hpp"


namespace ymd::dsp{
    template<typename FnIn, typename FnFt>
    void evaluate_func(const uint times, FnIn && fn_in, FnFt && fn_ft){
        
        const auto begin_m = micros();

        for(size_t i = 0; i < times; ++i){
            const auto x = real_t(i) / times;
            std::forward<FnFt>(fn_ft)(x);
        }

        const auto end_m = micros();
        
        DEBUG_PRINTS(real_t(uint32_t(end_m - begin_m)) / times, "us per call");
        delay(20);
        std::terminate();
    }

    template<typename FnIn, typename FnFt>
    void run_func(const uint fs, FnIn && fn_in, FnFt && fn_ft){
        
        hal::timer1.init(fs);
        
        hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
            const auto x = std::forward<FnIn>(fn_in)();
            const auto y = std::forward<FnFt>(fn_ft)(x);

            DEBUG_PRINTLN(x, y);
        });
        
        while(true);
    }

    namespace samples{
        template<arithmetic T>
        T sinwave(const T t){
            return std::sin(t);
        }
    }
}

using namespace ymd;
// using namespace ymd::dsp;

template<typename T, size_t n>
void butterworth_bandpass_coeff_tb(const T f1f, const T f2f, const bool scale_en = false){

    // n = order of the filter
    // fc1 = lower cutoff frequency as a fraction of Pi [0,1]
    // fc2 = upper cutoff frequency as a fraction of Pi [0,1]
    // sf = 1 to scale c coefficients for normalized response
    // sf = 0 to not scale c coefficients

    /* calculate the c coefficients */
    auto ccof = dsp::ccof_bwbp<n>();

    /* calculate the d coefficients */
    auto dcof = dsp::dcof_bwbp<T, n>(f1f, f2f );

    // const T sf = dsp::sf_bwbp<T, n>(f1f, f2f ); /* scaling factor for the c coefficients */

    DEBUG_PRINTLN(ccof);
    delay(20);
    DEBUG_PRINTLN(dcof);
    delay(20);

    std::terminate();
}

template<typename T, size_t n>
void butterworth_bandpass_tb(auto && fn_in, const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandpassFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fl = fl,
        .fh = fh,
        .fs = fs,
    }};

    dsp::run_func(fs, fn_in, filter);
    // dsp::evaluate_func(fs, fn_in, filter);
}
 
template<typename T, size_t n>
void butterworth_bandstop_tb(auto && fn_in, const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandstopFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fl = fl,
        .fh = fh,
        .fs = fs,
    }};

    dsp::run_func(fs, fn_in, filter);
}



template<arithmetic T>
class EnvelopeFilter{
public:
    using Lpf = dsp::LowpassFilter_t<T>;
    using Config = Lpf::Config;

    EnvelopeFilter(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        lpf_.reconf(cfg);
    }

    void reset(){
        lpf_.reset();
    }

    void update(const T in){
        lpf_.update(in * in);
    }

    T result() const{
        return std::sqrt(lpf_.result());
    }

    T operator ()(const T x){
        update(x);
        return result();
    }
private:
    Lpf lpf_ = {};
};

template<arithmetic T, size_t N>
class SumReducer{
public:
    using Config = std::array<T, N>;

    void update(const std::span<T, N> in){
        T result = 0;
        for(size_t i = 0; i < N; ++i){
            result += in[i] * cfg_[i];
        }
        result_ = result;
    }

    T result() const{
        return result_;
    }

    T operator ()(const std::span<T, N> in){
        update(in);
        return result();
    }
private:
    Config cfg_;
    T result_;
};

template<arithmetic T, size_t N>
class EvelopeBandpassFilter{
public:
    using Bpf = typename dsp::ButterBandpassFilter<T, N>;
    using Lpf = typename dsp::LowpassFilter_t<T>;
    Bpf bpf_;
    Lpf lpf_;
    struct Config{
        T fc;
        T fl;
        T fh;
        uint fs;
    };

    EvelopeBandpassFilter(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        bpf_.reconf(typename Bpf::Config{
            .fl = cfg.fl,
            .fh = cfg.fh,
            .fs = cfg.fs
        });

        lpf_.reconf(typename Lpf::Config{
            .fc = cfg.fc,
            .fs = cfg.fs
        });
    }

    void reset(){
        bpf_.reset();
    }

    void update(const T x){
        const auto h = bpf_(x);
        lpf_.update(h * h);
    }

    T result() const{
        return sqrt(lpf_.result());
    }

    T operator ()(const T x){
        update(x);
        return result();
    }
private:

};

template<typename T, size_t n>
void bpsk_tb(auto && fn_in, const T fl, const T fh, const uint fs){
    using Filter = EvelopeBandpassFilter<T, n>;
    using Config = Filter::Config;

    static constexpr auto Qbw = T(0.5);
    static constexpr auto Qfc = T(0.2);

    T side_bw = Qbw * (fh - fl) / 2;
    T fc = Qfc * (fh + fl) / 2;

    Filter l_filter = {Config{
        .fc = fc,
        .fl = fl - side_bw,
        .fh = fl + side_bw,
        .fs = fs,
    }};

    Filter h_filter = {Config{
        .fc = fc,
        .fl = fh - side_bw,
        .fh = fh + side_bw,
        .fs = fs,
    }};

    dsp::run_func(fs, fn_in, [&](auto && x){
        l_filter.update(x);
        h_filter.update(x);
        return - l_filter.result() + h_filter.result();
    });
}


template<typename T, size_t n>
void butterworth_lowpass_tb(auto && fn_in, const T fc, const uint fs){
    using Filter = dsp::ButterLowpassFilter<T, n>;
    using Config = typename Filter::Config;

    Filter filter = {Config{
        .fc = fc,
        .fs = fs,
    }};

    dsp::run_func(fs, fn_in, filter);
    // dsp::evaluate_func(fs, fn_in, filter);
}

template<typename T, size_t n>
void butterworth_highpass_tb(auto && fn_in, const T fc, const uint fs){
    using Filter = dsp::ButterHighpassFilter<T, n>;
    using Config = typename Filter::Config;

    Filter filter = {Config{
        .fc = fc,
        .fs = fs,
    }};

    dsp::run_func(fs, fn_in, filter);
    // dsp::evaluate_func(fs, fn_in, filter);
}



class DoubleToneMultiFrequencySiggen{
public:
    struct Config{
        std::array<uint16_t, 4> fl_map;
        std::array<uint16_t, 4> fh_map;
        uint fs;
    };

    DoubleToneMultiFrequencySiggen(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        fl_map_ = cfg.fl_map;
        fh_map_ = cfg.fh_map;

        delta_ = real_t(1) / cfg.fs;
    }

    void reset(){
        fl_index_ = 0;
        fh_index_ = 0;
        // time_ = 0;
    }

    
    void update(const real_t time_){
        // time_ += delta_;
        // time_ += 0.0001_q24;

        const auto fl_ = fl_map_[fl_index_];
        const auto fh_ = fh_map_[fh_index_];

        const auto rad = real_t(TAU) * frac(time_);
        result_ = sin(fl_ * rad) + sin(fh_ * rad);

        // const auto time = real_t(TAU) * myfrac(time_);
        // result_ = sinpu(fl_ * time) + sinpu(fh_ * time);

        // const auto rad = real_t(TAU) * time_;
        // result_ = sinpu(fl_ * time) + sinpu(fh_ * time);
        // result_ = rad;
        // result_ = time_;
        // return time_;
    }

    auto fl() const{
        return fl_map_[fl_index_];
    }

    auto fh() const{
        return fh_map_[fh_index_];
    }

    auto result() const{
        return result_;
    }

    auto operator ()() const{
        return result();
    }
private:
    uint8_t fl_index_ = 0;
    uint8_t fh_index_ = 0;

    std::array<uint16_t, 4> fl_map_;
    std::array<uint16_t, 4> fh_map_;

    // real_t time_;
    real_t delta_;
    real_t result_;
};

void dtmf_tb(const uint fs){
    using DTMF = DoubleToneMultiFrequencySiggen;
    DTMF dtmf = {{
        .fl_map = {70, 77, 85, 94}, 
        .fh_map = {120, 133, 148, 163}, 
        .fs = fs
    }};

    const real_t fl = dtmf.fl();
    const real_t fh = dtmf.fh();
    
    using Filter = dsp::ButterBandpassFilter<real_t, 4>;

    static constexpr auto Qbw = real_t(0.5);

    real_t side_bw = Qbw * (fh - fl) / 2;

    Filter l_filter {{
        .fl = fl - side_bw,
        .fh = fl + side_bw,
        .fs = fs
    }};

    Filter h_filter = {{
        .fl = fh - side_bw,
        .fh = fh + side_bw,
        .fs = fs
    }};

    {
        dtmf.reset();
        l_filter.reset();
        h_filter.reset();
    }

    hal::timer1.init(fs);
    hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
        const auto t = time();
        dtmf.update(t);
        const auto wave = real_t(dtmf.result());

        l_filter.update(wave);
        h_filter.update(wave);

        DEBUG_PRINTLN(
            // CLAMP(wave,0,1),
            t,
            wave,
            h_filter.result(),
            l_filter.result() 
        );
    });
}


using Particle = dsp::Particle<real_t, real_t>;

void pso_tb() __attribute((optimize(3,"Ofast","inline")));

void pso_tb(){
    using Particle = dsp::Particle<real_t, real_t>;
    using Pso = dsp::ParticleSwarmOptimization<Particle, real_t>;
    using Config = typename Pso::Config;


    // auto eval_func = [&](const Particle & p){
    //     return -ABS(p.x - 5.5_r + sin(t));
    //     // return -ABS(p.x - 0.5_r);
    // };

    Pso pso = {Config{
        .omega = 0.2_r,
        .c1 = 2.0_r,
        .c2 = 2.0_r,
        // .c1 = 2.0_r,
        // .c2 = 2.0_r,
        // .c1 = 1.3_r,
        // .c2 = 1.3_r,
        .n = 8,
        // .eval_func = eval_func
        
        // .reset_func = nullptr;
    }};

    pso.init(-10, 1);
    auto eval_func = [](const Particle & p){
        // return -ABS(p.x - 5.5_r + sin(time()));
        // return -ABS(p.x - 0.5_r);
        // const auto targ = sin(2 * time());
        // DEBUG_PRINTLN(targ);
        // return -ABS(p.x - 102.5_r);
    
        // if (ABS(p.x) <= 0.02_r) return 1;
        // return sin(p.x) / p.x;
    
        const auto x = p.x;
        return x * (2-x);
        // return -ABS(x);
        // return sin(x);
        // return CLAMP(1200 * cos(x) + (120-x) * x, 0, 10000);
        // return p.x;
        // return 0;
    };
    

    constexpr size_t loops = 200;

    const auto begin_m = micros();


    for(size_t i = 0; i < loops; i++){

        pso.update(eval_func);

        // const auto & particles = pso.particles();
        // for(const auto & p : particles){
            // DEBUG_PRINT(p.x);
            // DEBUG_PRINT(DEBUGGER.splitter());
            // delay(1);
        // }
        // delay(1);
        // DEBUG_PRINTLN(pso.gbest(), pso.geval());
    }
    DEBUG_PRINTLN(pso.gbest(), pso.geval(), micros() - begin_m);
}

void dsp_main(){
    // uart2.init(576000, CommMethod::Blocking);
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");


    // using T = float; 
    using T = iq_t<16>; 

    constexpr T fl = T(50);
    constexpr T fh = T(100);
    constexpr uint fs = 1000;

    constexpr size_t n = 4;

    auto sig_in = [&]() {
        // const T t = T(time());
        // const T rad = T(TAU) * T(time());

        // const T mf = 20;
        // const T f = 30;
        // const bool out = int(rad) % 2;
        // if(!out) return 0_r;
        // return sin(f * rad + mf * sin(rad)) + sin(350 * rad) + 10_r;
        // return + sin(500 * rad);
        // return 10_r* sin(rad);
        // return 0.002_r;

        const T rad = T(TAU) * T(time());

        static constexpr uint8_t code = 0x12;
        const auto index = int(10 * rad) % 8;
        const bool out = (code >> index) & 1;
        if(!out) return sin(fh * rad);
        return sin(fl * rad);
        // return 0.01_r * sin(20 * rad);
        // return 0.0001_r;
    };

    // butterworth_bandpass_tb<T, n>(sig_in, fl, fh, fs);
    // butterworth_bandstop_tb<T, n>(sig_in, fl, fh, fs);

    // butterworth_highpass_tb<T, n>(sig_in, fh, fs);
    // butterworth_lowpass_tb<T, n>(sig_in, fl, fs);
    // dtmf_tb(fs);
    pso_tb();
    
    // bpsk_tb<T, n>(sig_in, fl, fh, fs);

    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}