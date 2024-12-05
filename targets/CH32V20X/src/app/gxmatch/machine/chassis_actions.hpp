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
            inst_.meta_rapid_shift(dest_);
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

    Vector2 from_;
    Vector2 dest_;

    real_t dist_;
    Vector2 norm_;

    void init(){
        const auto & config = inst_.config(); 

        from_ = inst_.feedback().org;
        dist_ = (dest_ - from_).length();
        norm_ = (dest_ - from_) / dist_;

        solver_.emplace(
            TrapezoidSolver{config.max_acc, config.max_spr, dist_}
        );
    }
public:
    ACTION_NAME(shift)
    ShiftAction(Inst & inst, const Vector2 & pos):
        ChassisAction(UINT_MAX, [this](){
            if(first()){
                init();
            }
            if(inst_.arrived()){
                this->kill();
            }

            auto time = this->time();
            inst_.meta_rapid_shift(from_ + norm_ * solver_->forward(time));

        }, inst),
        dest_(pos){};
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

            // DEBUG_PRINTLN(inst_.rad());
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