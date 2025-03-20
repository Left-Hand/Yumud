#pragma once

#include "core/math/realmath.hpp"

namespace ymd::dsp{


namespace details{


template<arithmetic T, size_t N>
requires (N % 2 == 0)
class ButterSideFilterBase{
public:
    struct Config{
        T fc;
        uint fs;
    };

    ButterSideFilterBase(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reconf(const Config & cfg){
        const T a = std::tan((T(PI) * cfg.fc) / cfg.fs);
        const T a2 = a*a;

        for(size_t i = 0; i < N / 2; ++i){
            auto & state = states[i];
            const T r = std::sin((T(PI)*(2*i+1)) / (2 * N));
            const T s = a2 + 2 * a * r + 1;

            state.A = a2/s;
            state.d1 = 2*(1 - a2) / s;
            state.d2 = -(a2 - 2 * a * r + 1)/s;
        }
    }

    void reset(){
        for(auto & state : states){
            state.reset();
        }
        result_ = 0;
    }
// private:
protected:
    struct StateVector{
        T A;
        T d1;
        T d2;
        T w0;
        T w1;
        T w2;

        void reset(){
            w0 = w1 = w2 = 0;
        }

        T conv(const T x) const {
            return d1 * w1 + d2 * w2 + x;
        }
        
        void shift(){
            w2 = w1;
            w1 = w0;
        }
    };

    std::array<StateVector, N/2> states;
    T result_;
};
}

template<arithmetic T, size_t N>
class ButterHighpassFilter : public details::ButterSideFilterBase<T, N>{
public:
    using Super = details::ButterSideFilterBase<T, N>;
    using Super::Super;

    void update(T x){
        for(auto & state : this->states){
            state.w0 = state.conv(x);

            x = state.A * (
                +    state.w0 
                -2 * state.w1
                + state.w2
            );

            state.shift();
        }
        this->result_ = x;
    }

    T result() const {
        return this->result_;
    }
    T operator()(const T x){
        update(x);
        return result();
    }
};


template<arithmetic T, size_t N>
class ButterLowpassFilter : public details::ButterSideFilterBase<T, N>{
public:
    using Super = details::ButterSideFilterBase<T, N>;
    using Super::Super;

    void update(T x){
        for(auto & state : this->states){
            state.w0 = state.conv(x);

            x = state.A * (
                +    state.w0 
                + (2 * state.w1)
                + state.w2
            );

            state.shift();
        }
        this->result_ = x;
    }

    T result() const {
        return this->result_;
    }
    T operator()(const T x){
        update(x);
        return result();
    }
};


}