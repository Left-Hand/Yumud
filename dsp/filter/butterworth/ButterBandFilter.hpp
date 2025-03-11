#pragma once

#include <numbers>
#include "sys/math/real.hpp"
#include <cmath>

namespace ymd::dsp{

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

template<size_t N>
__inline constexpr
iq_t<N> conv(auto&& ... args){
    return iq_t<N>(_iq<N>::from_i32(_conv((args.value)...) >> N));
}



namespace details{
template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandFilterBase{
public:
    struct Config{
        T fl;
        T fh;
        uint fs;
    };

    ButterBandFilterBase() = default;

    ButterBandFilterBase(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        
        // T fl_norm = cfg.fl / (cfg.fs / 2);
        // T fh_norm = cfg.fh / (cfg.fs / 2);
        // T a = std::cos(T(PI)*(fh_norm + fl_norm)) / std::cos(T(PI)*(fh_norm - fl_norm));
        // T b = std::tan(T(PI)*(fh_norm - fl_norm));
        
        const T a = std::cos(T(PI)*(cfg.fh+cfg.fl)/cfg.fs) / std::cos(T(PI)*(cfg.fh-cfg.fl)/cfg.fs);
        const T b = std::tan(T(PI)*(cfg.fh-cfg.fl)/cfg.fs);
        const T a2 = a*a;
        const T b2 = b*b;

        
        for(size_t i = 0; i < N / 4; ++i){
            r_ = std::sin(T(PI)*(2*i+1)/N);
            s_ = b2 + 2*b*r_ + 1;
            states_[i].a = 1/s_;
            states_[i].d1 = 4*a*(1+b*r_)/s_;
            states_[i].d2 = 2*(b2-2*a2-1)/s_;
            states_[i].d3 = 4*a*(1-b*r_)/s_;
            states_[i].d4 = -(b2 - 2*b*r_ + 1)/s_;
        }
        r_ = 4*a;
        s_ = 4*a2+2;
    }

    T result() const{ return this->result_; }

    void reset(){
        for(auto & state:states_){
            state.reset();
        }
    }

protected:
// public:
// private:
    struct StateVector{
        T a;
        T d1, d2, d3, d4;
        T w0, w1, w2, w3, w4;

        void reset(){
            w0 = w1 = w2 = w3 = w4 = 0;
        }

        T conv(const T x) const {
            return (
                + d1 * w1 
                + d2 * w2 
                + d3 * w3 
                + d4 * w4 
                + x
            );

            // return dsp::conv<16>(
            //     d1, w1,
            //     d2, w2,
            //     d3, w3,
            //     d4, w4,
            //     x
            // );
        }

        void shift(){
            w4 = w3;
            w3 = w2;
            w2 = w1;
            w1 = w0;
        }
    };

    std::array<StateVector, N / 4> states_ = {};

    T r_;
    T s_;
    T result_;
};

}

template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandstopFilter:public details::ButterBandFilterBase<T, N>{
public:

    using Super = details::ButterBandFilterBase<T, N>;
    using Config = typename Super::Config;
    using StateVector = typename Super::StateVector;

    ButterBandstopFilter() = default;
    ButterBandstopFilter(const Config & cfg):Super(cfg){}
    void update(T x){
        auto & self = *this;

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

        self.result_ = x;
    }

    void reconf(const Config & cfg){
        Super::reconf(cfg);
    }
    
    T operator ()(const T x){
        update(x);
        return this->result_;
    }
};

template<arithmetic T, size_t N>
requires (N % 4 == 0)
class ButterBandpassFilter:public details::ButterBandFilterBase<T, N>{
public:

    using Super = details::ButterBandFilterBase<T, N>;
    using Config = typename Super::Config;
    using StateVector = typename Super::StateVector;

    ButterBandpassFilter() = default;
    ButterBandpassFilter(const Config & cfg):Super(cfg){}
    
    void update(T x){
        auto & self = *this;

        for(size_t i = 0; i < N/4; ++i){
            self.states_[i].w0 = self.states_[i].conv(x);

            x = self.states_[i].a*(
                + self.states_[i].w0 
                - 2 * self.states_[i].w2
                + self.states_[i].w4
            );

            self.states_[i].shift();
        }

        self.result_ = x;
    }

    void reconf(const Config & cfg){
        Super::reconf(cfg);
    }

    T operator ()(const T x){
        update(x);
        return this->result_;
    }
};

}