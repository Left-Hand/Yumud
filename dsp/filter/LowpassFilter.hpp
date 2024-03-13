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
public:
    LowpassFilter_t(const real & cutoff_freq) : last(Point{real(0), time(0)}), inverse_tau(TAU * cutoff_freq) {;}

    real update(const real & x, const time & t) override {
        if(!inited){
            last.x = x;
            last.t = t;
            inited = true;
            return last.x;
        }
        real k = (t - last.t)* inverse_tau;
        last.x = k * x + (1 - k) * last.x;
        last.t = t;
        return last.x;
    }
};

#endif