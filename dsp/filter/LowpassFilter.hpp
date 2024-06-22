#ifndef __LOWPASS_FILTER_HPP__

#define __LOWPASS_FILTER_HPP__

#include "../lti.hpp"


template<typename real, typename time>
class LowpassFilter_t:public Filter_t<real, time>{
protected:
    using Point = LtiUtils::Point<real, time>;

    Point last;
    const real inverse_tau;
    bool inited = false;

    void init(const real & x){
        // if(!inited){
            last.x = x;
            inited = true;
            // return last.x;
        // }
    }
public:
    template<typename U>
    LowpassFilter_t(const U & cutoff_freq) : last(Point{real(0), time(0)}), inverse_tau(real(TAU * cutoff_freq)) {;}

    real update(const real & x, const time & t) override {
        if(!inited){
            init(x);
            last.t = t;
            return x;
        }
        real ret = forward(x, t - last.t);
        last.t = t;
        return ret;
    }

    real forward(const real & x, const time & delta){
        if(!inited){
            init(x);
            return x;
        }
        real b = delta * inverse_tau;
        real alpha = b / (b + 1);
        last.x = last.x + alpha * (x - last.x);
        return last.x;
    }
};

template<typename real>
class LowpassFilterZ_t{
// protected:
public:
    real k;
    real last;
    bool inited = false;
public:
    LowpassFilterZ_t(const auto & cutoff_freq) : k(cutoff_freq), last(real()){;}

    real update(const real & x){
        if(!inited){
            last = real();
            inited = true;
        }
        last = last * k + (1-k) * x;
        return last;
    }
};

#endif