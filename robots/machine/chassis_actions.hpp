#pragma once

#include "chassis_module.hpp"
#include "../autodrive/sequence/TrapezoidSolver_t.hpp"


namespace ymd::robots{

namespace ChassisActions{


scexpr real_t safe_wait_time = 0.4_r;

class StraightAction:public ChassisAction{
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
    ACTION_NAME(straight)

    StraightAction(Inst & inst, const real_t dest):
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_shift();
            }

            auto time = this->time();
            auto fronting = solver_->forward(time);
            inst_.set_target_jny({{0,fronting}, 0});

            if(time > solver_->period() + safe_wait_time){
                this->kill();
            }

        }, inst),
        dest_(dest){};
};


class SideAction:public ChassisAction{
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
    ACTION_NAME(side)
    
    SideAction(Inst & inst, const real_t dest):
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_shift();
            }

            auto time = this->time();
            auto fronting = solver_->forward(time);
            inst_.set_target_jny({{fronting, 0}, 0});

            if(time > solver_->period() + safe_wait_time){
                inst_.freeze();
                this->kill();
            }

        }, inst),
        dest_(dest){};
};

class ShiftAction:public ChassisAction{
protected:
    using CtrlMode = ChassisModule::CtrlMode;
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;

    Vector2 dest_;
    Vector2 norm_;
    real_t dist_;

    void init(){
        const auto & config = inst_.config(); 
        dist_ = dest_.length();
        norm_ = dest_ / dist_;

        solver_.emplace(
            TrapezoidSolver{config.max_acc, config.max_spd, dist_}
        );
    }
public:
    ACTION_NAME(shift)
    
    ShiftAction(Inst & inst, const Vector2 & dest):
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_shift();
            }

            auto time = this->time();
            auto fronting = solver_->forward(time);
            inst_.set_target_jny({fronting * norm_, 0});

            if(time > solver_->period() + safe_wait_time){
                inst_.freeze();
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
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_spin();
            }

            auto time = this->time();
            auto rading = solver_->forward(time);
            inst_.set_target_rad(rading);

            if(time > solver_->period() + safe_wait_time){
                inst_.freeze();
                this->kill();
            }
        }, inst),
        dest_(rad){};
};


class StrictSpinAction:public ChassisAction{
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
    ACTION_NAME(strict_spin)
    StrictSpinAction(Inst & inst, const real_t rad):
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_strict_spin();
            }

            auto time = this->time();
            auto rading = solver_->forward(time);
            inst_.set_target_rad(rading);
            // DEBUG_PRINTLN(rading);
            if(time > solver_->period() + safe_wait_time){
                inst_.freeze();
                this->kill();
            }
        }, inst),
        dest_(rad){};
};

class StrictShiftAction:public ChassisAction{
protected:
    using CtrlMode = ChassisModule::CtrlMode;
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;

    Vector2 dest_;
    Vector2 norm_;
    real_t dist_;

    void init(){
        const auto & config = inst_.config(); 
        dist_ = dest_.length();
        norm_ = dest_ / dist_;

        solver_.emplace(
            TrapezoidSolver{config.max_acc, config.max_spd, dist_}
        );
    }
public:
    ACTION_NAME(strict_shift)
    
    StrictShiftAction(Inst & inst, const Vector2 & dest):
        ChassisAction(UINT32_MAX, [this](){
            if(first()){
                init();
                inst_.entry_strict_shift();
            }

            auto time = this->time();
            auto fronting = solver_->forward(time);
            inst_.set_target_jny({fronting * norm_, 0});

            if(time > solver_->period() + safe_wait_time){
                inst_.freeze();
                this->kill();
            }

        }, inst),
        dest_(dest){};
};

class FreezeAction:public ChassisAction{
public:
    ACTION_NAME(freeze)
    FreezeAction(Inst & inst):
        ChassisAction(1, [this](){
            inst_.freeze();
        }, inst)
        {};
};


class TrimAction:public ChassisAction{
protected:
    Ray2_t<real_t> trim_;
public:
    ACTION_NAME(trim)
    TrimAction(Inst & inst, const Ray2_t<real_t> & trim):
        ChassisAction(UINT32_MAX, [this](){
            // if(inst_.arrived()) kill();
            // TODO("csd");
        }, inst),
        trim_(trim){};
};

}
};