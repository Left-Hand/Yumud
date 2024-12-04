#pragma once

#include "motion_module.hpp"
#include "../autodrive/Estimator.hpp"
#include "wheel/wheels.hpp"
#include "wheel/wheel.hpp"



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
    class RotationCtrl{
    public:
        struct Config{
            real_t kp;
            real_t kd;  
        };

    protected:
        const Config & config_;
    public:
        RotationCtrl(const Config & config):config_(config){;}
        RotationCtrl(const RotationCtrl & other) = delete;
        RotationCtrl(RotationCtrl && other) = delete;
        
    };

    class PositionCtrl{
    public:
        struct Config{
            real_t kp;
            real_t kd;
        };

    protected:
        const Config & config_;
    public:
        PositionCtrl(const Config & config):config_(config){;}
        DELETE_COPY_AND_MOVE(PositionCtrl)
    };

    struct Config{
        Mecanum4Solver::Config solver_config;
        Estimator::Config est_config;

        real_t max_acc;
        real_t max_spd;
        
        real_t max_agr;
        real_t max_spr;
    };

    class FeedBacker{
    public:
        virtual Vector2 pos() = 0;
        virtual real_t rad() = 0;
    };
    

    class FeedBackerOpenLoop:public FeedBacker{

    };

    class FeedBackerCloseLoop:public FeedBacker{

    };

    const Config & config_;
    Wheels wheels_;
    Mecanum4Solver solver_{config_.solver_config};
    // Estimator est_{config_.est_config};
    Estimator & est_;
protected:

    Vector2 expect_pos;
    real_t expect_rad;
    void closeloop();
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


    Vector2 pos();
    real_t rad();
    Ray gest();
    bool arrived();

    Ray feedback();
    const auto & config()const {return config_;}
};

}