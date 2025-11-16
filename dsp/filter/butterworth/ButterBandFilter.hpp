#pragma once

#include "core/math/realmath.hpp"

namespace ymd::dsp{
#if 0
namespace details{
template<size_t N>
__inline constexpr
int64_t _conv(const _iq<N> x){
    return int64_t(x.to_i32());
}

template<size_t N>
__inline constexpr
int64_t _conv(const _iq<N> x, const _iq<N> h){
    return int64_t(int64_t(x.to_i32()) * int64_t(h.to_i32()));
}


template<size_t N>
__inline constexpr
int64_t _conv(const _iq<N> x, const _iq<N> h, auto&& ... rest){
    return _conv(x,h) + _conv((rest) ...);
}


}

template<size_t N>
__inline constexpr
fixed_t<N> conv(auto&& ... args){
    return fixed_t<N>(_iq<N>::from_i32(details::_conv((args.value)...) >> N));
}

#endif

namespace details{
template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandFilterBase{
public:
    struct Config{
        uint fs;
        T fl;
        T fh;
    };

    constexpr ButterBandFilterBase() = default;

    constexpr ButterBandFilterBase(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        
        // T fl_norm = cfg.fl / (cfg.fs / 2);
        // T fh_norm = cfg.fh / (cfg.fs / 2);
        // T a = std::cos(T(PI)*(fh_norm + fl_norm)) / std::cos(T(PI)*(fh_norm - fl_norm));
        // T b = std::tan(T(PI)*(fh_norm - fl_norm));
        const auto [s_sum, c_sum] = sincospu(T(cfg.fh+cfg.fl) / (cfg.fs * 2));
        const auto [s_delta, c_delta] = sincospu(T(cfg.fh-cfg.fl) / (cfg.fs * 2));
        const T a = c_sum / c_delta;
        const T b = s_delta / c_delta;
        const T a2 = a*a;
        const T b2 = b*b;

        
        for(size_t i = 0; i < N / 4; ++i){
            r_ = sinpu(T(2*i+1)/(2 * N));
            s_ = b2 + 2*b*r_ + 1;
            const auto inv_s = 1 / s_;
            states_[i].a = 1 * inv_s;
            states_[i].d1 = 4*a*(1+b*r_) * inv_s;
            states_[i].d2 = 2*(b2-2*a2-1) * inv_s;
            states_[i].d3 = 4*a*(1-b*r_) * inv_s;
            states_[i].d4 = -(b2 - 2*b*r_ + 1) * inv_s;
        }

        r_ = 4 * a;
        s_ = 4 * a2+2;
    }

    constexpr const T & output() const{ return this->output_; }

    constexpr void reset(){
        for(auto & state:states_){
            state.reset();
        }
    }

protected:
    struct StateVector{
        T a;
        T d1, d2, d3, d4;
        T w0, w1, w2, w3, w4;

        __fast_inline constexpr void reset(){
            w0 = w1 = w2 = w3 = w4 = 0;
        }

        __fast_inline constexpr T conv(const T x) const {
            return (
                + d1 * w1 
                + d2 * w2 
                + d3 * w3 
                + d4 * w4 
                + x
            );
        }

        __fast_inline constexpr void shift(){
            w4 = w3;
            w3 = w2;
            w2 = w1;
            w1 = w0;
        }
    };

    std::array<StateVector, N / 4> states_ = {};

    T r_;
    T s_;
    T output_;
};

}

template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandstopFilter:public details::ButterBandFilterBase<T, N>{
public:

    using Super = details::ButterBandFilterBase<T, N>;
    using Config = typename Super::Config;
    using StateVector = typename Super::StateVector;

    constexpr ButterBandstopFilter() = default;
    constexpr ButterBandstopFilter(const Config & cfg):Super(cfg){}
    constexpr void update(T x){
        auto & self = *this;

        
        #pragma GCC unroll 1
        for(size_t i = 0; i < N / 4; ++i){
            self.states_[i].w0 = self.states_[i].conv(x);

            x = self.states_[i].a*(
                self.states_[i].w0 - 
                self.r_ * self.states_[i].w1 + 
                self.s_ * self.states_[i].w2- 
                self.r_ * self.states_[i].w3 + 
                self.states_[i].w4
            );

            self.states_[i].shift();
        }

        self.output_ = x;
    }

    constexpr void reconf(const Config & cfg){
        Super::reconf(cfg);
    }
    
    constexpr T operator ()(const T x){
        update(x);
        return this->output_;
    }

    constexpr const T & output() const{ return this->output_; }
};

template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandpassFilter:public details::ButterBandFilterBase<T, N>{
public:

    using Super = details::ButterBandFilterBase<T, N>;
    using Config = typename Super::Config;
    using StateVector = typename Super::StateVector;

    constexpr ButterBandpassFilter() = default;
    constexpr ButterBandpassFilter(const Config & cfg):Super(cfg){}
    
    constexpr void update(T x){
        auto & self = *this;

        #pragma GCC unroll 1
        for(size_t i = 0; i < N/4; ++i){
            self.states_[i].w0 = self.states_[i].conv(x);

            x = self.states_[i].a*(
                + self.states_[i].w0 
                - 2 * self.states_[i].w2
                + self.states_[i].w4
            );

            self.states_[i].shift();
        }

        self.output_ = x;
    }

    constexpr void reconf(const Config & cfg){
        Super::reconf(cfg);
    }

    constexpr T operator ()(const T x){
        update(x);
        return this->output_;
    }
    
    constexpr const T & output() const{ return this->output_; }
};

}