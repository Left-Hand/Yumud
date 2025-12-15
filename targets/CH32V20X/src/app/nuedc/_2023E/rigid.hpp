#pragma once


#include "actuator.hpp"

using namespace ymd;
namespace nuedc::_2023E{
//跟踪微分器 用于平滑输入
class [[nodiscard]] TdVec2{
public:
    struct Config{
        iq12 kp;
        iq12 kd;
        iq16 x2_limit;
        iq16 x3_limit;
        uint32_t fs;
    };

    using E = iq16;
    using T = Vec2<iq16>;

    using State = std::array<T, 3>;
    using Self = TdVec2;
    
    constexpr TdVec2(const Config & cfg){
        reset();
        reconf(cfg);
    }

    //impure fn
    constexpr void update(const T targ){
        state_ = forward(*this, state_, targ);
    }

    //impure fn
    constexpr void reset(){
        // pass
    }

    //impure fn
    constexpr void reconf(const Config & cfg){
        kp_ = cfg.kp;
        kd_ = cfg.kd;
        dt_ = 1_iq16 / cfg.fs;
        max_x2_ = cfg.x2_limit;
        max_x3_ = cfg.x3_limit;
    }
    
    //impure fn
    [[nodiscard]]
    constexpr const auto & state() const {
        return state_;
    }
private:
    iq12 kp_;
    iq12 kd_;
    iq16 dt_;
    iq16 max_x2_;
    iq16 max_x3_;
    State state_ {T::ZERO, T::ZERO, T::ZERO};

    //pure fn
    [[nodiscard]]
    // static constexpr State 
    static constexpr State 
    forward(const Self & self, const State state, const T u){
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        // const auto x2_limit = self.max_x2_;
        // const auto x3_limit = self.max_x3_;

        const auto pos = state[0];
        const auto spd = state[1];
        // const auto acc = state[2];

        // const auto raw_a = ((iq12(self.kp_) * (Vec2<iq12>(u - pos)))
        //      - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_x3_);

        const auto e1 = Vec2<iq12>(u - pos);
        const auto dist = e1.length();
        const auto norm_e1 = e1 / dist;
        const auto expect_spd = std::sqrt(2 * self.max_x3_ * dist);

        // DEBUG_PRINTLN(spd.normalized() * expect_spd);
        return {
            pos + spd * dt, 
            spd.move_toward(norm_e1 * expect_spd, self.max_x3_ * self.dt_)
            .clampmax(self.max_x2_)
            ,
            norm_e1 * expect_spd
            // raw_a
            // {CLAMP2(raw_a.x, self.max_x3_), CLAMP2(raw_a.y, self.max_x3_)}
            // .clampmax(self.max_x3_)
            // .clampmax(10.0_r)
            // (spd + acc * dt).clampmax(x2_limit),
            // (acc + (-self.kd * spd, -self.x3_limit * (pos - u)))
            // (state[1] + (- 2 * r * spd - r_2 * (pos - u)) * dt)
        };


    }

    
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
            .x2_limit     = cfg.x2_limit,
            .x3_limit     = cfg.x3_limit,
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
    TdVec2 td_;
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