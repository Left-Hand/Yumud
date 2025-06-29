#pragma once

#include "robots/kinematics/Mecanum4/mecanum4_solver.hpp"

#include "types/regions/ray2/Ray2.hpp"
#include "types/vectors/vector2/Vector2.hpp"

#include "chassis_ctrl.hpp"
#include "drivers/IMU/IMU.hpp"
#include "dsp/filter/kalman/KalmanFilter.hpp"


namespace ymd::robots{

class ChassisModule;

namespace ChassisActions{

class ChassisAction:public ActionBase{
protected:
    using Inst = ChassisModule; 
    Inst & inst_;
    ChassisAction(size_t s, Callback && func, Inst & inst):
        ActionBase(s, std::move(func)),
        inst_(inst){;}
public:
};


}


class ChassisModule:public MotionModule{
public:


    struct Config{
        Mecanum4Solver_t<real_t>::Config solver_config;
        // Estimator::Config est_config;

        RotationCtrl::Config rot_ctrl_config;
        PositionCtrl::Config pos_ctrl_config;

        real_t max_acc;
        real_t max_spd;
        
        real_t max_agr;
        real_t max_spr;

        real_t still_time;
    };

    enum class CtrlMode{
        NONE,
        SPIN,
        SHIFT,
        STRICT_SPIN,
        STRICT_SHIFT
    };


protected:
    const Config & config_;
    Wheels wheels_;
    Mecanum4Solver_t<real_t> solver_{config_.solver_config};

    drivers::AccelerometerIntf & acc_sensor_;
    drivers::GyroscopeIntf & gyr_sensor_;
    drivers::MagnetometerIntf & mag_sensor_;

    Ray2q<16> current_jny_;
    real_t gyr_;
    real_t gyr_sum_;

    std::array<real_t, 4>last_motor_positions;

    RotationCtrl rot_ctrl_{config_.rot_ctrl_config};
    PositionCtrl pos_ctrl_{config_.pos_ctrl_config};

    CtrlMode ctrl_mode_ = CtrlMode::NONE;

    Ray2q<16> target_jny_;
    real_t target_rot_;

    Vector2q<16> spd_;
    Vector2q<16> last_pos_;
    

    void closeloop();
public:
    ChassisModule(const Config & config, 
            const Wheels & wheels,
            drivers::AccelerometerIntf & acc_sensor,
            drivers::GyroscopeIntf & gyr_sensor,
            drivers::MagnetometerIntf & mag_sensor
        ):
            config_(config), 
            wheels_(wheels),
            acc_sensor_(acc_sensor),
            gyr_sensor_(gyr_sensor),
            mag_sensor_(mag_sensor)
        {}


    void reset_rot();
    void reset_journey();
    void trim(const Ray2q<16> & ray);

    void recalibrate(const Ray2q<16> & ray);
    void init();
    void tick800();

    void setCurrent(const Ray2q<16> & ray);
    void setPosition(const Ray2q<16> & ray);
    void setPosition(const std::tuple<real_t, real_t, real_t, real_t> pos);

    void setMode(const CtrlMode mode){
        ctrl_mode_ = mode;
    }

    real_t rad(){return gyr_sum_ * real_t(0.005);}
    real_t gyr(){return gyr_;}
    auto jny(){return current_jny_;}
    auto diff(){return current_jny_.org;}
    auto spd(){return spd_;}

    void entry_spin();
    void entry_shift();
    void entry_strict_spin();
    void entry_strict_shift();
    void freeze();

    //侧向移动
    void sideways(const real_t dist);

    //径向移动
    void straight(const real_t dist);

    //平移
    void shift(const Vector2q<16> & diff);

    void follow(const Ray2q<16> & to);

    //旋转
    void spin(const real_t ang);
    void strict_spin(const real_t ang);
    void strict_shift(const Vector2q<16> & offs);

    // void wait(const real_t dur);

    void set_target_rad(const real_t rad){
        this->target_rot_ = rad;
    }

    void set_target_jny(const Ray2q<16> & jny){
        this->target_jny_ = jny;
    }

    // bool arrived();
    const auto & config()const {return config_;}
};

}