#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "FFT.hpp"

#include "dsp/filter/butterworth/band.hpp"
#include "dsp/filter/butterworth/side.hpp"
#include "dsp/filter/butterworth/Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "dsp/filter/firstorder/lpf.hpp"

#include "dsp/sigproc/tunning_filter.hpp"
#include "dsp/sigproc/comb_allpass.hpp"

#include "ParticleSwarmOptimization.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "func_eval.hpp"
#include "digipw/ctrl/qpr_controller.hpp"

using namespace ymd;

template<arithmetic T>
class EnvelopeFilter{
public:
    using Lpf = dsp::FirstOrderLowpassFilter<T>;
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
    using Lpf = typename dsp::FirstOrderLowpassFilter<T>;
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



#if 0
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
#endif

template<typename T, size_t n>
static auto make_butterworth_bandpass(const T fl, const T fh, const uint fs){
    using Filter = dsp::ButterBandpassFilter<T, n>;
    using Config = Filter::Config;

    Filter filter = {Config{
        .fs = fs,
        .fl = fl,
        .fh = fh,
    }};

    return filter;

    // TransferSysevtluator::run_func(fn_in, filter);
    // dsp::evtluate_func(fs, fn_in, filter);
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

// template<typename T>
// static auto make_delay_line(const T delay, const std::span<T> buffer){
//     return dsp::DelayLine()
// }

// template<typename T>

#define DBG_UART hal::uart2
void dsp_main(){
    // uart2.init(576000, CommStrategy::Blocking);
    DBG_UART.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");


    // using T = float; 
    using T = iq16; 

    [[maybe_unused]] constexpr T FREQ_LOW = T(250);
    [[maybe_unused]] constexpr T FREQ_HIGH = T(400);
    constexpr uint SAMPLE_FREQ = 4000;
    // constexpr uint SAMPLE_FREQ = 2000;
    // constexpr uint SAMPLE_FREQ = 1000;

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
        // return math::sinpu(75 * t);
        // return math::sinpu(15 * t);
        return iq16(math::frac(75 * t)) * 0.2_r;
    };

    #endif

    // auto && sig_proc = make_butterworth_bandpass<q20, N>(FREQ_LOW, FREQ_HIGH, SAMPLE_FREQ);
    auto && bpf = make_butterworth_bandpass<iq16, N>(FREQ_LOW, FREQ_HIGH, SAMPLE_FREQ);
    // auto && sig_proc = make_tunning_filter<T>(1.0_r);

    static constexpr size_t BUFFER_SIZE = 512;
    auto buffer = std::vector<iq16>(BUFFER_SIZE);
    auto && allpass = dsp::CombAllpass<iq16>(std::span(buffer));
    allpass.set_delay_ticks(10.6_r);
    auto && sig_proc = [&](const real_t t){
        const auto bpf_out = bpf(t);
        return allpass(bpf_out);
    };


    TransferSysEvaluator evt;
    evt.set_sample_freq(SAMPLE_FREQ);
    evt.run_func(
        SAMPLE_FREQ/4,
        sig_in, 
        sig_proc
    );
    // evt.evtluate_func(
    //     SAMPLE_FREQ,
    //     sig_in, 
    //     sig_proc
    // );

    while(true){
        // const auto t = evt.time();
        const auto [input,output] = evt.get_input_and_output();
        DEBUG_PRINTLN_IDLE(input, output, bpf.output(), output+bpf.output());
    }
    // butterworth_bandstop_tb<T, n>(sig_in, FREQ_LOW, FREQ_HIGH, SAMPLE_FREQ);

    // butterworth_highpass_tb<T, n>(sig_in, FREQ_HIGH, SAMPLE_FREQ);
    // butterworth_lowpass_tb<T, n>(sig_in, FREQ_LOW, SAMPLE_FREQ);
    // dtmf_tb(SAMPLE_FREQ);
    // pso_tb();
    
    // bpsk_tb<T, n>(sig_in, FREQ_LOW, FREQ_HIGH, SAMPLE_FREQ);

    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}