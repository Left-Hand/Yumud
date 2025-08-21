#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/stdrange.hpp"

#include "digipw/prelude/abdq.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"



namespace ymd{


struct CurrentSensor{

    static constexpr real_t CURRENT_CUTOFF_FREQ = 400;

    struct Config{
        uint32_t fs;
    };

    constexpr explicit CurrentSensor(const Config & cfg):
        mid_filter_({CURRENT_CUTOFF_FREQ, cfg.fs}){
    }

    constexpr void reconf(const Config & cfg){
        mid_filter_.reconf({CURRENT_CUTOFF_FREQ, cfg.fs});
    }



    constexpr void update(const real_t u, const real_t v, const real_t w){
        uvw_raw_ = {u,v,w};

        mid_filter_.update((uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) * q16(1.0/3));

        mid_curr_ = mid_filter_.get();
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);

        ab_curr_ = digipw::AbCurrent::from_uvw(uvw_curr_);
    }

    constexpr void reset(){
        uvw_curr_ = {0, 0, 0};
        uvw_bias_ = {0, 0, 0};
        ab_curr_ = {0, 0};
    }


    constexpr const auto &  raw()const {return uvw_raw_;}
    constexpr const auto &  mid() const {return mid_curr_;}
    constexpr const auto &  uvw()const{return uvw_curr_;}
    constexpr const auto & ab()const{return ab_curr_;}

private:

    digipw::UvwCurrent uvw_bias_;
    digipw::UvwCurrent uvw_raw_;
    digipw::UvwCurrent uvw_curr_;
    real_t mid_curr_;
    digipw::AbCurrent ab_curr_;
    dsp::ButterLowpassFilter<q16, 4> mid_filter_;
};




}


namespace ymd{

}



template<typename Iter>
static constexpr size_t count_iter(Iter && iter){
    size_t cnt = 0;
    while(true){
        if(iter.has_next()){
            cnt++;
        }else{
            break;
        }
        (void)iter.next();
    }
    return cnt;
}


// struct TurnSolver{
//     uint16_t ta = 0;
//     uint16_t tb = 0;
//     real_t pa = 0;
//     real_t pb = 0;
//     real_t va = 0;
//     real_t vb = 0;
// };

// [[maybe_unused]] static real_t demo(uint milliseconds){
//     // using Vec2<real_t> = CubicInterpolation::Vec2<real_t>;
//     static TurnSolver turnSolver;
    
//     uint32_t turnCnt = milliseconds % 2667;
//     uint32_t turns = milliseconds / 2667;
    
//     scexpr real_t velPoints[7] = {
//         real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
//     };
    
//     scexpr real_t posPoints[7] = {
//         real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
//     };

//     scexpr uint tickPoints[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tickPoints[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tickPoints[i];
//     turnSolver.tb = tickPoints[i + 1];
//     auto dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     real_t dp = turnSolver.pb - turnSolver.pa;

//     real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
//     real_t temp = (real_t)dt / 1000 / dp; 

//     real_t yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = CubicInterpolation::forward(
//             Vec2<real_t>{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, 
//             Vec2<real_t>(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
//     else
//         yt = _t;

//     real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }
