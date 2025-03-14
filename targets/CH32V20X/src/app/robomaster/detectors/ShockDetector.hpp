#pragma once

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/rc/HighpassFilter.hpp"

namespace ymd::dsp{


template<typename T>
class BandpassFilter{
public:
    struct Config{
        T fl;
        T fh;
        uint fs;
    };

    BandpassFilter(const Config & config){
        reconf(config);
        reset();
    }

    void reconf(const Config & config){
        lpf_.reconf({
            .fc = config.fl,
            .fs = config.fs
        });

        hpf_.reconf({
            .fc = config.fh,
            .fs = config.fs
        });
    }

    void update(const T x){
        lpf_.update(x);
        hpf_.update(lpf_.result());
    }

    T result() const{
        return hpf_.result();
    }

    void reset(){
        lpf_.reset();
        hpf_.reset();
    }
private:
    using Lpf = LowpassFilter_t<T>;
    using Hpf = HighpassFilter_t<T>;

    Lpf lpf_;
    Hpf hpf_;
};


// template<typename T>
// class ShookDetector{
// public:
//     struct Config{
//         T fc;
//         uint fs;
//     };

//     ShookDetector(const Config & config){
//         reconf(config);
//         reset();
//     }

//     void reconf(const Config & config){
//         baseline_lpf_.reconf({
//             .fc = config.fc,
//             .fs = config.fs
//         });

//         wavegen_lpf_.reconf({
//             .fc = config.fc,
//             .fs = config.fs
//         });
//     }

//     void reset(){
//         baseline_lpf_.reset();
//         wavegen_lpf_.reset();
//     }

//     void update(const T x){
//         // baseline_lpf_.update(x);
//         wavegen_lpf_.update(x);
//         result_ = wavegen_lpf_.result();
//         // const T x_filtered = baseline_lpf_.result();

//         // const T abs_err = ABS(x - x_filtered);
//         // wavegen_lpf_.update(abs_err);
//     }

//     T result() const {
//         // return wavegen_lpf_.result();
//         // return baseline_lpf_.result();
//         return result_;
//     }
// private:
//     using Lpf = LowpassFilter_t<T>;

//     // Lpf baseline_lpf_ = {};
//     // Lpf wavegen_lpf_ = {};
//     // T result_ = {};
//     Lpf baseline_lpf_;
//     Lpf wavegen_lpf_ ;
//     T result_;
// };


}