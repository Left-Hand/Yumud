#pragma once

#include "chassis_ctrl.hpp"
namespace gxm{

class ChassisModule;

namespace ChassisActions{

class ChassisAction:public Action{
protected:
    using Inst = ChassisModule; 
    Inst & inst_;
    ChassisAction(size_t s, Callback && func, Inst & inst):
        Action(s, std::move(func)),
        inst_(inst){;}
public:
};


}


class ChassisModule:public MotionModule{
public:


    struct Config{
        Mecanum4Solver::Config solver_config;
        Estimator::Config est_config;

        RotationCtrl::Config rot_ctrl_config;
        PositionCtrl::Config pos_ctrl_config;

        real_t max_acc;
        real_t max_spd;
        
        real_t max_agr;
        real_t max_spr;
    };

    enum class CtrlMode{
        NONE,
        SPIN,
        SHIFT
    };


protected:
    const Config & config_;
    Wheels wheels_;
    Mecanum4Solver solver_{config_.solver_config};
    Estimator & est_;

    RotationCtrl rot_ctrl_{config_.rot_ctrl_config, *this};
    PositionCtrl pos_ctrl_{config_.pos_ctrl_config, *this};

    CtrlMode ctrl_mode_ = CtrlMode::NONE;

    Vector2 expect_pos_;
    real_t expect_rot_;

public:
    ChassisModule(const Config & config, 
            const Wheels & wheels,
            Estimator & est
        ):
        config_(config), 
        wheels_(wheels),
        est_(est){}

    void meta_rapid(const Ray & ray);
    void meta_rapid_shift(const Vector2 & pos);
    void meta_rapid_spin(const real_t rad);

    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & vel, const real_t spinrate);

    void calibratePosition(const Vector2 & pos);
    void calibrateRotation(const real_t rad);

    void test();
    void tick();

    void setCurrent(const Ray & ray);
    void closeloop();
    void setMode(const CtrlMode mode){
        ctrl_mode_ = mode;
    }

    Vector2 pos();
    Vector2 spd();
    real_t rot();
    real_t gyr();

    Ray gest();
    bool arrived();

    Ray feedback();
    const auto & config()const {return config_;}
};

}