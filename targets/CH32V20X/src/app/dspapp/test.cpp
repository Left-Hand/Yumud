#include "src/testbench/tb.h"

#include "sys/clock/time.hpp"

#include "FFT.hpp"
#include "liir.hpp"
#include "sys/debug/debug.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "ButterSideFilter.hpp"

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
        std::array<uint16_t, 4> fl;
        std::array<uint16_t, 4> fh;
        uint fs;
    };

    DoubleToneMultiFrequencySiggen(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        fl_ = cfg.fl;
        fh_ = cfg.fh;

        delta_ = iq_t<24>(1) / cfg.fs;
    }

    void reset(){
        fl_index_ = 0;
        fh_index_ = 0;
        time_ = 0;
    }

    void update(){
        time_ += delta_;

        const auto fl = fl_[fl_index_];
        const auto fh = fh_[fh_index_];

        result_ = sinpu(fl * time_) + sinpu(fh * time_);
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

    std::array<uint16_t, 4> fl_;
    std::array<uint16_t, 4> fh_;

    iq_t<24> time_;
    iq_t<24> delta_;
    iq_t<30> result_;
};

void dtmf_tb(const uint fs){
    using DTMF = DoubleToneMultiFrequencySiggen;
    DTMF dtmf = {{
        .fl = {70, 77, 85, 94}, 
        .fh = {120, 133, 148, 163}, 
        .fs = fs
    }};

    const real_t fl = 70;
    const real_t fh = 120;
    
    using Filter = dsp::ButterBandpassFilter<real_t, 4>;
    using Config = typename Filter::Config;

    static constexpr auto Qbw = real_t(0.5);
    static constexpr auto Qfc = real_t(0.2);

    real_t side_bw = Qbw * (fh - fl) / 2;
    real_t fc = Qfc * (fh + fl) / 2;

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
        dtmf.update();
        const auto wave = real_t(dtmf.result());

        l_filter.update(wave);
        h_filter.update(wave);

        DEBUG_PRINTLN(
            wave,
            l_filter.result(), 
            h_filter.result()
        );
    });
}

void dsp_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");


    // using T = float; 
    using T = iq_t<16>; 

    constexpr T fl = T(200);
    constexpr T fh = T(500);
    constexpr uint fs = 2000;

    constexpr size_t n = 2;

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
        if(!out) return 8 * sin(fh * rad);
        return 8 * sin(fl * rad);
        // return 0.01_r * sin(20 * rad);
        // return 0.0001_r;
    };

    // butterworth_bandpass_tb<T, n>(sig_in, fl, fh, fs);
    // butterworth_bandstop_tb<T, n>(sig_in, fl, fh, fs);

    // butterworth_highpass_tb<T, n>(sig_in, fh, fs);
    dtmf_tb(fs);
    // butterworth_lowpass_tb<T, n>(sig_in, fl, fs);
    
    // bpsk_tb<T, n>(sig_in, fl, fh, fs);

    {
        // /* calculate the d coefficients */
        // static constexpr auto dcof = dsp::dcof_bwbp<std::decay_t<decltype(f1f)>, n>(f1f, f2f );

        // /* calculate the c coefficients */
        // static constexpr auto ccof = dsp::ccof_bwbp<n>();
    }
}