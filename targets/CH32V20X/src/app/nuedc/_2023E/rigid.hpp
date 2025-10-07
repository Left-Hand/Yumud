#pragma once


#include "actuator.hpp"

using namespace ymd;
namespace nuedc::_2023E{


//动力学 用于快速追踪目标关节角
class GimbalDynamics final{
public:
    using Config = dsp::TdVec2::Config;

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
            .kp          = cfg.kp,
            .kd          = cfg.kd,
            .max_spd    = cfg.max_spd,
            .max_acc    = cfg.max_acc,
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
        td_.update({targ_.yaw, targ_.pitch});
        const auto [ref_yaw, ref_pitch] = td_.state()[0];
        ref_ = GimbalSolution{ref_yaw, ref_pitch};
        actuator_.set_gest(ref_);
    }
private:
    GimbalActuatorIntf & actuator_;
    dsp::TdVec2 td_;
    GimbalSolution targ_;
    GimbalSolution ref_;
};

//运动学 用于求解目标关节角
class GimbalKinematics final{
public:
    using Gesture = ymd::Vec2<real_t>;
    using Solution = GimbalSolution;

    struct Config{
        real_t gimbal_base_height;
        real_t gimbal_dist_to_screen;
        real_t screen_width;
        real_t screen_height;
    };

    constexpr GimbalKinematics(const Config & cfg){
        reconf(cfg);
    }

    constexpr void reconf(const Config & cfg){
        cfg_ = cfg;
    }

    constexpr Solution inverse(const Gesture & gesture) const {
        const auto x = gesture.x;
        const auto y = gesture.y - cfg_.gimbal_base_height;

        const auto yaw_rad = std::atan2(x, cfg_.gimbal_dist_to_screen);
        const auto dist = cfg_.gimbal_dist_to_screen / std::cos(yaw_rad);
        const auto pitch_rad = std::atan2(y, dist);
        
        return Solution{
            .yaw = yaw_rad, 
            .pitch = pitch_rad
        };
    }
private:
    Config cfg_;
};

//规划器 顺序执行需要途径的曲线
class GimbalPlanner final{
public:
    struct Config{
        GimbalDynamics::Config      dyna_cfg;
        GimbalKinematics::Config    kine_cfg;
    };

    struct LineAction{
        Vec2<real_t> start;
        Vec2<real_t> stop;
    };

    struct DelayAction{
        real_t seconds;
    };

    size_t pending() const {
        TODO();
        return 0;
    }

public:
    GimbalPlanner(
        const Config & cfg, 
        GimbalActuatorIntf & actuator
    ):
        actuator_(actuator),
        dynamics_(cfg.dyna_cfg, actuator), 
        kinematics_(cfg.kine_cfg)
    {
        reconf(cfg);
    }

    void reconf(const Config & cfg){
        // cfg_ = cfg;
        dynamics_.reconf(cfg.dyna_cfg);
        kinematics_.reconf(cfg.kine_cfg);
    }

    void tick(){
        //pass
        ;
    }
private:
    GimbalActuatorIntf & actuator_;
    GimbalDynamics dynamics_;
    GimbalKinematics kinematics_;
};




}