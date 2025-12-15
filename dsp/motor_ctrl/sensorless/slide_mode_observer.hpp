#pragma once

// https://blog.csdn.net/lijialin_bit/article/details/104263194
// https://geekdaxue.co/read/aiyanjiudexiaohutongxue@oo4p7l/pdapg4cn8io7ic6h

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"
#include "digipw/prelude/abdq.hpp"

namespace ymd::dsp::motor_ctl{

class SlideModeObserver{
public:
    struct Meas{
        digipw::AlphaBetaCoord<iq20> i;
        digipw::AlphaBetaCoord<iq20> e;


        constexpr void reset(){
            i = digipw::AlphaBetaCoord<iq20>(0, 0);
            e = digipw::AlphaBetaCoord<iq20>(0, 0);

        }
    };
    struct State{
        digipw::AlphaBetaCoord<iq20> i;
        digipw::AlphaBetaCoord<iq20> e;
        digipw::AlphaBetaCoord<iq20> z;
        constexpr void reset(){
            i = digipw::AlphaBetaCoord<iq20>(0, 0);
            e = digipw::AlphaBetaCoord<iq20>(0, 0);
            z = digipw::AlphaBetaCoord<iq20>(0, 0);
        }

        constexpr Angular<iq20> angle() const {
            return e.angle();
        }
    };
    struct  Config{
        iq16 f_para;
        iq16 g_para;
        iq16 kslide;   
        iq16 kslf;   
    };

    constexpr SlideModeObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reset(){
        state_.reset();
        turns_ = 0;
    }



    // 更新函数
    constexpr void update(const Meas & meas){

        const auto est_i = (f_para_ * state_.i) + (g_para_ * (meas.e - state_.e - state_.z));

        // 当前电流误差
        const auto i_err = est_i - meas.i;

        state_.z = i_err.map([this](auto x){return sat(x);});

        state_.e = state_.e + (Kslf_ * (state_.z - state_.e));

        turns_ = math::frac(math::atan2pu(-state_.e.alpha, state_.e.beta));
    }

    constexpr void reconf(const Config & cfg){
        f_para_ = cfg.f_para;
        g_para_ = cfg.g_para;
        Kslide_ = cfg.kslide;
        Kslf_ = cfg.kslf;
    }


    // 获取估计的转子角度
    Angular<iq16> angle() const {return Angular<iq16>::from_turns(turns_);}

private:
    iq16 f_para_ = 0;
    iq16 g_para_ = 0;
    iq16 Kslide_ = 0;
    iq16 Kslf_ = 0;
public:
    State state_;

    iq16 turns_ = 0;

    // 滑模阈值
    static constexpr iq16 E0 = iq16(1.5);
    // 滑模阈值的倒数
    static constexpr iq16 invE0 = iq16(1/1.5);

    constexpr iq16 sat(const iq16 x) const {
        if(x > E0) return Kslide_;
        else if (x < -E0) return -Kslide_;
        else return Kslide_ * x * invE0;
    }
};

} // namespace ymd