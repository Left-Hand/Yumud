#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "FFT.hpp"
#include "liir.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/filter/butterworth/Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/sigproc/tunning_filter.hpp"

#include "ParticleSwarmOptimization.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;
using namespace ymd::hal;

struct Evaluator{
    Evaluator() = default;

    template<typename FnIn, typename FnProc>
    static void evaluate_func(const uint times, FnIn && fn_in, FnProc && fn_proc){
        
        const auto begin_m = clock::micros();

        for(size_t i = 0; i < times; ++i){
            const auto x = real_t(i) / times;
            std::forward<FnProc>(fn_proc)(x);
        }

        const auto end_m = clock::micros();
        
        DEBUG_PRINTS(real_t((end_m - begin_m).count()) / times, "us per call");
        clock::delay(20ms);
        std::terminate();
    }

    template<typename FnIn, typename FnProc>
    void run_func(const uint32_t f_isr, FnIn && fn_in, FnProc && fn_proc){


        hal::timer1.init({
            // .freq = fs_.expect("you have not set fs yet")
            .freq = f_isr
        });
        hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
            x_ = std::forward<FnIn>(fn_in)(time_);
            y_ = std::forward<FnProc>(fn_proc)(x_);
            time_ += delta_;
        });
        
    }

    constexpr auto get_xy() const {
        return std::make_tuple(x_,y_);
    }

    void set_fs(const uint32_t fs){
        fs_ = Some(fs);
        time_ = 0;
        delta_ = 1_q24 / fs;
    }

    auto time() const {
        return time_;
    }

    auto delta() const {
        return delta_;
    }

private:
    Option<uint32_t> fs_ = None;
    q24 time_ = 0;
    q24 delta_ = 0;

    q16 x_;
    q16 y_;
};


using namespace ymd;
// using namespace ymd::dsp;



template<arithmetic T>
class EnvelopeFilter{
public:
    using Lpf = dsp::LowpassFilter<T>;
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
    using Lpf = typename dsp::LowpassFilter<T>;
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
static void butterworth_bandpass_coeff_tb(const T f1f, const T f2f, const bool scale_en = false){

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
    clock::delay(20ms);
    DEBUG_PRINTLN(dcof);
    clock::delay(20ms);

    std::terminate();
}

template<typename T, size_t n>
static auto make_butterworth_bandpass(const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandpassFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fl = fl,
        .fh = fh,
        .fs = fs,
    }};

    return filter;

    // Evaluator::run_func(fn_in, filter);
    // dsp::evaluate_func(fs, fn_in, filter);
}

template<typename T, size_t n>
static auto make_butterworth_bandstop(const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandstopFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fl = fl,
        .fh = fh,
        .fs = fs,
    }};

    return filter;
}


template<typename T, size_t n>
static auto make_bpsk(const T fl, const T fh, const uint fs){
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

    return [=](auto && x){
        l_filter.update(x);
        h_filter.update(x);
        return - l_filter.result() + h_filter.result();
    };
}


template<typename T, size_t B>
static auto butterworth_lowpass(const T fc, const uint fs){
    using Filter = dsp::ButterLowpassFilter<T, B>;
    using Config = typename Filter::Config;

    Filter filter = {Config{
        .fc = fc,
        .fs = fs,
    }};

    return filter;
}

template<typename T, size_t N>
static auto make_butterworth_highpass(const T fc, const uint fs){
    using Filter = dsp::ButterHighpassFilter<T, N>;
    using Config = typename Filter::Config;

    Filter filter = {Config{
        .fc = fc,
        .fs = fs,
    }};

    return filter;
}

template<typename T>
static auto make_tunning_filter(const T delay){
    using Filter = dsp::TunningFilter<T>;
    using Config = typename Filter::Config;
    Filter filter {Config{
        .delay = delay,
    }};

    return filter;
}


#define DBG_UART hal::uart2
void dsp_main(){
    // uart2.init(576000, CommStrategy::Blocking);
    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");


    // using T = float; 
    using T = iq_t<16>; 

    [[maybe_unused]] constexpr T FREQ_LOW = T(250);
    [[maybe_unused]] constexpr T FREQ_HIGH = T(400);
    constexpr uint FREQ_SAMPLE = 4000;
    // constexpr uint FREQ_SAMPLE = 1000;

    [[maybe_unused]]
    constexpr size_t N = 4;

    #if 0
    auto sig_in = [&](const real_t t) {
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

        const T rad = T(TAU) * t;

        static constexpr uint8_t code = 0x12;
        const auto index = int(10 * rad) % 8;
        const bool out = (code >> index) & 1;
        if(!out) return sin(FREQ_HIGH * rad);
        return sin(FREQ_LOW * rad);
        // return 0.01_r * sin(20 * rad);
        // return 0.0001_r;
    };
    #else
    auto sig_in = [](const real_t t){
        // return sinpu(75 * t);
        // return sinpu(15 * t);
        return frac(75 * t) * 0.2_r;
    };

    #endif

    auto && sig_proc = make_butterworth_bandpass<q20, N>(FREQ_LOW, FREQ_HIGH, FREQ_SAMPLE);
    // auto && sig_proc = make_tunning_filter<T>(1.0_r);


    Evaluator eva;
    eva.set_fs(FREQ_SAMPLE);
    eva.run_func(
        FREQ_SAMPLE / 4,
        sig_in, 
        sig_proc
    );

    while(true){
        const auto t = eva.time();
        const auto [x,y] = eva.get_xy();
        DEBUG_PRINTLN_IDLE(t, x, y);
    }
    // butterworth_bandstop_tb<T, n>(sig_in, FREQ_LOW, FREQ_HIGH, FREQ_SAMPLE);

    // butterworth_highpass_tb<T, n>(sig_in, FREQ_HIGH, FREQ_SAMPLE);
    // butterworth_lowpass_tb<T, n>(sig_in, FREQ_LOW, FREQ_SAMPLE);
    // dtmf_tb(FREQ_SAMPLE);
    // pso_tb();
    
    // bpsk_tb<T, n>(sig_in, FREQ_LOW, FREQ_HIGH, FREQ_SAMPLE);

    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}