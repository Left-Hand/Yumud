#pragma once

#include "device.hpp"
#include "ctrl.hpp"

using namespace ymd;
namespace nuedc::_2023E{

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
    using Setter = std::function<void(MotorCmd)>;

    struct Params{
        Setter yaw_setter;
        Setter pitch_setter;
    };


    GimbalActuatorByLambda (Params params) :
        yaw_setter_     (params.yaw_setter), 
        pitch_setter_   (params.pitch_setter)
    {}

    void set_gest(const GimbalSolution solu){
        yaw_setter_     ({solu.yaw_rad, 0});
        pitch_setter_   ({solu.pitch_rad, 0});
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
    using Gesture = ymd::Vector2<real_t>;
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
            .yaw_rad = yaw_rad, 
            .pitch_rad = pitch_rad
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
        Vector2<real_t> start;
        Vector2<real_t> stop;
    };

    struct DelayAction{
        real_t seconds;
    };

    size_t pending() const {
        TODO();
        return 0;
    }

public:
    GimbalPlanner(const Config & cfg, GimbalActuatorIntf & actuator):
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