#pragma once

#include "device.hpp"
#include "ctrl.hpp"

namespace nudec::_2023E{

//描述了一组关节解
struct GimbalSolution final{
    real_t yaw_rad;
    real_t pitch_rad;

    friend OutputStream & operator << (OutputStream & os, const GimbalSolution & self){
        return os << os.brackets<'('>() 
            << self.yaw_rad << os.splitter() 
            << self.pitch_rad << os.brackets<')'>();
    }
};

//执行器 封装了对不同执行器的调用 屏蔽底层差异
class GimbalActuatorIntf{
public:
    virtual void set_gest(const GimbalSolution solu) = 0;
};


class GimbalActuatorByMock  final :public GimbalActuatorIntf {
public:
    void set_gest(const GimbalSolution solu){
        DEBUG_PRINTLN(solu);
    }
};

class GimbalActuatorByLambda  final :public GimbalActuatorIntf {
public:
    using Setter = std::function<void(real_t)>;

    struct Params{
        Setter yaw_setter;
        Setter pitch_setter;
    };


    GimbalActuatorByLambda (Params params) :
        yaw_setter_     (params.yaw_setter), 
        pitch_setter_   (params.pitch_setter)
    {}

    void set_gest(const GimbalSolution solu){
        yaw_setter_     (solu.yaw_rad);
        pitch_setter_   (solu.pitch_rad);
    }
private:
    Setter yaw_setter_;
    Setter pitch_setter_;
};


//动力学 用于快速追踪目标关节角
class GimbalDynamics final{
public:
    using Config = TdVec2::Config;

    bool done(){
        TODO();
        return false;
    }
    
    //impure fn
    GimbalDynamics(const Config & cfg, GimbalActuatorIntf & actuator):
        actuator_(actuator), td_(cfg){
            reconf(cfg);
        }

    //impure fn
    void reconf(const Config & cfg){
        td_.reconf({
            .r          = cfg.r,
            .max_spd    = cfg.max_spd,
            .fs         = cfg.fs
        });
    }

    //impure fn
    void reset(){
        td_.reset();
    }

    //impure fn
    void set_target_gest(const GimbalSolution solu){
        targ_ = solu;
    }

    void tick(){
        td_.update({targ_.yaw_rad, targ_.pitch_rad});
        const auto [ref_yaw, ref_pitch] = td_.get()[0];
        ref_ = GimbalSolution{ref_yaw, ref_pitch};
        actuator_.set_gest(ref_);
    }
private:
    GimbalActuatorIntf & actuator_;
    TdVec2 td_;
    GimbalSolution targ_;
    GimbalSolution ref_;

};

//运动学 用于求解目标关节角
class GimbalKinematics final{
public:
    struct Config{
        real_t gimbal_base_height;
        real_t gimbal_dist_to_screen;
        real_t screen_width;
        real_t screen_height;
    };

    void reconf(const Config & cfg){
        cfg_ = cfg;
    }
private:
    GimbalKinematics(const Config & cfg){
        cfg_ = cfg;
    }

    Config cfg_;
    using Gesture = ymd::Vector2_t<real_t>;
    using Solution = GimbalSolution;
    static constexpr Solution inverse(const Config & cfg, const Gesture & gesture){
        const auto x = gesture.x;
        const auto y = gesture.y - cfg.gimbal_base_height;

        const auto yaw_rad = std::atan2(x, cfg.gimbal_dist_to_screen);
        const auto dist = cfg.gimbal_dist_to_screen / std::cos(yaw_rad);
        const auto pitch_rad = std::atan2(y, dist);
        
        return Solution{
            .yaw_rad = yaw_rad, 
            .pitch_rad = pitch_rad
        };
    }
};

//规划器 顺序执行需要途径的曲线
class GimbalPlanner final{
    struct Config{
        GimbalKinematics    kine_cfg;
        GimbalDynamics      dyna_cfg;
    };

    struct LineAction{
        Vector2_t<real_t> start;
        Vector2_t<real_t> stop;
    };

    struct DelayAction{
        real_t seconds;
    };

    size_t pending() const {
        TODO();
        return 0;
    }
private:

    Config cfg_;
};

}