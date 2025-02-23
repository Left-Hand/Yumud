#pragma once

#include "../lti.hpp"


template<arithmetic T>
class LowpassFilter_t{
protected:
    using Point = LtiUtils::Point<T, T>;

    Point last_;
    const T inverse_tau;
    bool inited = false;

    void init(const T x){
        last_.x = x;
        inited = true;
    }
public:
    LowpassFilter_t(const T cutoff_freq) : last_(Point{T(0), T(0)}), inverse_tau(T(TAU) * T(cutoff_freq)) {;}

    T update(const T x, const T tm){
        if(!inited){
            init(x);
            last_.t = tm;
            return x;
        }

        T ret = forward(x, tm - last_.t);
        last_.t = tm;
        return ret;
    }

    T forward(const T x, const T delta){
        if(!inited){
            init(x);
            return x;
        }
        T b = delta * inverse_tau;
        T alpha = b / (b + 1);
        last_.x = last_.x + alpha * (x - last_.x);
        return last_.x;
    }
};

//Z域低通滤波器
template<arithmetic T>
struct LowpassFilterZ_t{
private:
    T alpha_;
    T last_;
    bool inited_;
public:
    constexpr LowpassFilterZ_t(){
        reset();
    }

    constexpr LowpassFilterZ_t(const auto fc, const auto fs) : LowpassFilterZ_t(solve_alaph(fc, fs)) {;}
    constexpr LowpassFilterZ_t(const auto alpha) : alpha_(static_cast<T>(alpha)) {reset();}

    constexpr void reconf(const auto fc, const auto fs){
        alpha_ = solve_alpha(fc, fs);
    }

    constexpr void reconf(const auto alpha){
        alpha_ = static_cast<T>(alpha);
    }

    constexpr void reset(){
        last_ = 0;
        inited_ = false;
    }

    constexpr T update(const T x){
        if(unlikely(inited_ == false)){
            last_ = x;
            inited_ = true;
        }else{
            last_ = (last_ * (1-alpha_) + (alpha_) * x);
        }
        return last_;
    }

    constexpr T result() const {
        return last_;
    }

    static constexpr T solve_alpha(const auto fc, const auto fs){
        return T(628 * fc) / (100 * fs + 628 * fc);
    }
};

using Lpf = LowpassFilter_t<real_t>;
using LpfZ = LowpassFilterZ_t<real_t>;