#ifndef __LINEAR_OBSERVER_HPP__

#define __LINEAR_OBSERVER_HPP__

#include "../lti.hpp"

template <typename real, typename time>
class LinearObersver_t:public Observer_t<real, time>{
// protected:
public:
    using Point = LtiUtils::Point<real, time>;

    Point prev = Point{.x = real(0), .t = time(0)};
    Point last = Point{.x = real(0), .t = time(0)};
    real dx_dt = real(0);
public:
    LinearObersver_t() = default;
    real update(const real & x, const time & t) override{
        time dt = t - last.t;
        real dx = x - last.x;

        prev = last;
        last = Point{.x = x,.t = t};

        if(dt) dx_dt = dx / dt;
        return dx_dt;
    }

    real update(const auto & x, const auto & t){
        return update(static_cast<real>(x), static_cast<time>(t));
    }

    real predict(const time & t) override{
        time delta = t - last.t;
        return last.x + delta * dx_dt;
    }

    real predict(const real & x, const time & t) {
        time delta = t - last.t;
        return x + delta * dx_dt;
    }

    real getDerivative(){
        return dx_dt;
    }
};

#endif