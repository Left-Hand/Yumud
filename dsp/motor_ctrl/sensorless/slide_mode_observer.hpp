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
        state.reset();
        turns_ = 0;
    }



    // 更新函数
    constexpr void update(const Meas & meas){

        const auto est_i = (f_para_ * state.i) + (g_para_ * (meas.e - state.e - state.z));

        // 当前电流误差
        const auto i_err = est_i - meas.i;

        state.z = i_err.map([this](auto x){return sat(x);});

        state.e = state.e + (kslf_ * (state.z - state.e));
    }

    constexpr void reconf(const Config & cfg){
        f_para_ = cfg.f_para;
        g_para_ = cfg.g_para;
        kslide_ = cfg.kslide;
        kslf_ = cfg.kslf;
    }


    // 获取估计的转子角度
    Angular<iq16> angle() const noexcept {return Angular<iq16>::from_turns(turns_);}

private:
    iq16 f_para_ = 0;
    iq16 g_para_ = 0;
    iq16 kslide_ = 0;
    iq16 kslf_ = 0;
public:
    State state;

    iq16 turns_ = 0;

    // 滑模阈值
    static constexpr iq16 E0 = iq16(1.5);
    // 滑模阈值的倒数
    static constexpr iq16 invE0 = iq16(1/1.5);

    constexpr iq16 sat(const iq16 x) const noexcept {
        if(x > E0) return kslide_;
        else if (x < -E0) return -kslide_;
        else return kslide_ * x * invE0;
    }
};

} // namespace ymd