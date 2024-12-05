#pragma once

#include "chassis_module.hpp"
#include "../autodrive/sequence/TrapezoidSolver_t.hpp"


namespace gxm{

namespace ChassisActions{

class RapidShiftAction:public ChassisAction{
protected:
    Vector2 dest_;
public:
    ACTION_NAME(rapid_shift)
    RapidShiftAction(Inst & inst, const Vector2 & pos):
        ChassisAction(UINT_MAX, [this](){
            if(inst_.arrived()){
                this->kill();
            }
        }, inst),
        dest_(pos){};
};

class RapidSpinAction:public ChassisAction{
protected:
    real_t dest_;
public:
    ACTION_NAME(rapid_spin)
    RapidSpinAction(Inst & inst, const real_t rad):
        ChassisAction(UINT_MAX, [this](){
            if(inst_.arrived()){
                this->kill();
            }
        }, inst),
        dest_(rad){};
};


class ShiftAction:public ChassisAction{
protected:
    using CtrlMode = ChassisModule::CtrlMode;
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;

    real_t dest_;


    void init(){
        const auto & config = inst_.config(); 

        solver_.emplace(
            TrapezoidSolver{config.max_acc, config.max_spd, dest_}
        );
    }
public:
    ACTION_NAME(shift)
    ShiftAction(Inst & inst, const real_t dest):
        ChassisAction(UINT_MAX, [this](){
            if(first()){
                init();
                inst_.entry_shift();
            }

            auto time = this->time();
            auto fronting = solver_->forward(time);
            inst_.set_target_jny({{0,fronting}, 0});

            if(time > solver_->period() + real_t(0.4)){
                this->kill();
            }

        }, inst),
        dest_(dest){};
};

class SpinAction:public ChassisAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;
    using CtrlMode = ChassisModule::CtrlMode;
    real_t dest_;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;

    void init(){
        const auto & config = inst_.config(); 

        solver_.emplace(
            TrapezoidSolver{config.max_agr, config.max_spr, dest_}
        );
    }
public:
    ACTION_NAME(spin)
    SpinAction(Inst & inst, const real_t rad):
        ChassisAction(UINT_MAX, [this](){
            if(first()){
                init();
                inst_.entry_spin();
            }

            auto time = this->time();
            auto rading = solver_->forward(time);
            inst_.set_target_rad(rading);

            if(time > solver_->period() + real_t(0.4)){
                this->kill();
            }
        }, inst),
        dest_(rad){};
};


class WaitArriveAction:public ChassisAction{
public:
    ACTION_NAME(wait_arrive)
    WaitArriveAction(Inst & inst):
        ChassisAction(UINT_MAX, [this](){
            if(inst_.arrived()) kill();
        }, inst)
        {};
};



class TrimAction:public ChassisAction{
protected:
    Ray trim_;
public:
    ACTION_NAME(trim)
    TrimAction(Inst & inst, const Ray & trim):
        ChassisAction(UINT_MAX, [this](){
            // if(inst_.arrived()) kill();
            // TODO("csd");
        }, inst),
        trim_(trim){};
};

}
};