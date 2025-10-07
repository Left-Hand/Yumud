#pragma once

#include "utils.hpp"
#include "core/math/realmath.hpp"

namespace ymd::dsp{


class TrackingDifferentiator{
public:
    struct Config{
        q20 h;
        q20 r;
        uint fs;
    };

    TrackingDifferentiator(const Config & cfg){
        reconf(cfg);
        reset();
    }

    void reset(){
        state_.reset();
    }


    void update(const q20 v){
        const auto h = h_;
        const auto r = r_;

        const auto x1 = state_[0];
        const auto x2 = state_[1];

        state_ += {
            h * x2, h * fhan(x1 - v,x2,r, h)
        };
    }

    void update2(const q20 v){
        const auto h = h_;

        const auto x1 = state_[0];
        const auto x2 = state_[1];

        state_ += {
            h * x2, h * fhan2(x1 - v,x2)
        };
    }
    
    static constexpr q20 fhan(q20 x1,q20 x2,q20 r,q20 h){

        const auto deltaa = r*h;
        const auto deltaa0 = deltaa*h;
        const auto y = x1 + (x2*h);
        const auto a0 = sqrt(deltaa*deltaa+8*r*abs(y));
        const auto a = ((abs(y)<=deltaa0) ? (x2+y/h) : (x2+0.5_r*(a0-deltaa)*sign(y)));
    
        if(abs(a)<=deltaa)
            return -r*a/deltaa;
        else
            return -r*sign(a);
    }

    __fast_inline
    constexpr q20 fhan2(q20 x1,q20 x2) const{
        const auto deltaa_squ = square(deltaa_);
        const auto r = r_;
        const auto inv_h = inv_h_;

        const auto y = x1 + x2 * h_;
        const auto a0 = sqrt(deltaa_squ + 8 * r * abs(y));
        const auto a = ((abs(y)<=deltaa0_) ? 
            (x2+y * inv_h) : 
            (x2+0.5_r*(a0-deltaa_)*sign(y)));
    
        if(abs(a)<=deltaa_)
            return -a * inv_h;
        else
            return -r*sign(a);
    }

    const auto & state() const {return state_;}


    void reconf(const Config & cfg){
        h_ = cfg.h;
        r_ = cfg.r;

        deltaa_ = cfg.r*cfg.h;
        deltaa0_ = cfg.r*cfg.h*cfg.h;
        inv_h_ = 1/cfg.h;
        // d_lmt_ = - cfg.r 
    }
private:
    q20 h_ = 0;
    q20 r_ = 0;
    
    q20 deltaa_ = 0;
    q20 deltaa0_ = 0;
    q20 inv_h_ = 0;
    // q20 d_lmt_ = 0;

    using State = StateVector<q20, 2>;
    State state_;
};

class  _TrackingDifferentiatorByOrders_Base{
public:
    struct Config{
        q16 r;
        uint fs;
    };

    void reconf(const Config & cfg){
        r_ = cfg.r;
        dt_ = 1_q24 / cfg.fs;
    }

protected:
    q16 r_ = 0;
    q24 dt_ = 0;
};

template<size_t N>
class TrackingDifferentiatorByOrders :public _TrackingDifferentiatorByOrders_Base{
public:


    constexpr TrackingDifferentiatorByOrders(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reset(){
        state_.reset();
    }


    constexpr void update(const q20 u){
        const auto r = r_;
        const auto r_2 = r * r;
        const auto r_3 = r_2 * r;
        const auto dt = dt_;

        if constexpr (N == 2){
            const auto x1 = state_[0];
            const auto x2 = state_[1];

            state_[0] += x2 * dt; 
            state_[1] += (- 2 * x2 * r - (x1 - u) * r_2) * dt;

        }else if constexpr(N == 3){
            const auto x1 = state_[0];
            const auto x2 = state_[1];
            const auto x3 = state_[2];

            state_ += {
                x2 * dt, 
                x3 * dt,
                (((- 3 * x3) * r) - ((3 * x2) * r_2) - ((x1 - u) * r_3)) * dt
            };
        }else if constexpr (N ==4){
            const auto x1 = state_[0];
            const auto x2 = state_[1];
            const auto x3 = state_[2];
            const auto x4 = state_[3];

            state_ += {
                x2 * dt, 
                x3 * dt,
                x4 * dt,
                (-4 * x4 - 6 * x3 - 4 * r * x2 - r_2 * r * (x1 - u)) * r * dt
            };
        }
    }

    constexpr const auto & back() const {return state_.back();}
    constexpr const auto & state() const {return state_;}


private:

    using State = StateVector<q20, N>;
    State state_;
};


// class ExtendedStateObserver{

//     void update(const real_t u, const real_t y){
//         const real_t e = z1 - y;


//         const real_t z1  = state_[0];
//         const real_t z2  = state_[1];
//         const real_t z3  = state_[2];

//         state_[0] = z1 + h *(z2-belta01*e);
        // state_[1] = z2 + h *(z3-belta02*fal(e,0.5,delta)+b*u);
//         state_[2] = z3 + h *(-belta03*fal(e,0.25,delta));
//     }
// private:
//     using State = StateVector<q20, 3>;
//     State state_;
// }

}