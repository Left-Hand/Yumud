#pragma once

#include <numbers>
#include "sys/math/real.hpp"
#include <cmath>

namespace ymd::dsp{

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
            A_[i] = 1/s_;
            d1_[i] = 4*a*(1+b*r_)/s_;
            d2_[i] = 2*(b2-2*a2-1)/s_;
            d3_[i] = 4*a*(1-b*r_)/s_;
            d4_[i] = -(b2 - 2*b*r_ + 1)/s_;
        }
        r_ = 4*a;
        s_ = 4*a2+2;
    }

    void reset(){
        w0_.fill(0);
        w1_.fill(0);
        w2_.fill(0);
        w3_.fill(0);
        w4_.fill(0);
    }

protected:
// public:
// private:
    struct StateVector{
        T A;
        T d1, d2, d3, d4;
        T w0, w1, w2, w3, w4;
    };

    // std::array<T, N / 4> states = {};

    std::array<T, N / 4> A_ = {};
    std::array<T, N / 4> d1_ = {};
    std::array<T, N / 4> d2_ = {};
    std::array<T, N / 4> d3_ = {};
    std::array<T, N / 4> d4_ = {};

    std::array<T, N / 4> w0_ = {};
    std::array<T, N / 4> w1_ = {};
    std::array<T, N / 4> w2_ = {};
    std::array<T, N / 4> w3_ = {};
    std::array<T, N / 4> w4_ = {};

    T r_;
    T s_;
    T result_;
};

}

template<arithmetic T, size_t N, typename Super = details::ButterBandFilterBase<T, N>>
requires (N % 4 == 0)
class ButterBandstopFilter:public Super{
public:
    using Config = Super::Config;
    using StateVector = Super::StateVector;

    void update(T x){
        auto & self = *this;

        for(size_t i = 0; i < N / 4; ++i){
            self.w0_[i] = 
                self.d1_[i] * self.w1_[i] + 
                self.d2_[i] * self.w2_[i] + 
                self.d3_[i] * self.w3_[i] + 
                self.d4_[i] * self.w4_[i] + x;

            x = self.A_[i]*(
                self.w0_[i] - 
                self.r_ * self.w1_[i] + 
                self.s_ * self.w2_[i]- 
                self.r_ * self.w3_[i] + 
                self.w4_[i]
            );

            self.w4_[i] = self.w3_[i];
            self.w3_[i] = self.w2_[i];
            self.w2_[i] = self.w1_[i];
            self.w1_[i] = self.w0_[i];
        }

        self.result_ = x;
    }

    T operator ()(const T x){
        update(x);
        return this->result_;
    }
};

template<arithmetic T, size_t N, typename Super = details::ButterBandFilterBase<T, N>>
requires (N % 4 == 0)
class ButterBandpassFilter:public Super{
public:
    using Config = Super::Config;
    using StateVector = Super::StateVector;

    void update(T x){
        auto & self = *this;

        for(size_t i = 0; i < N/4; ++i){
            self.w0_[i] = 
                self.d1_[i] * self.w1_[i] + 
                self.d2_[i] * self.w2_[i] + 
                self.d3_[i] * self.w3_[i] + 
                self.d4_[i] * self.w4_[i] + x;

            x = self.A_[i]*(
                + self.w0_[i] 
                - 2 * self.w2_[i]
                + self.w4_[i]
            );

            self.w4_[i] = self.w3_[i];
            self.w3_[i] = self.w2_[i];
            self.w2_[i] = self.w1_[i];
            self.w1_[i] = self.w0_[i];
        }

        self.result_ = x;
    }

    T operator ()(const T x){
        update(x);
        return this->result_;
    }
};

}