#pragma once

#include "chassis_module.hpp"
#include "../autodrive/sequence/TrapezoidSolver_t.hpp"


namespace gxm{

namespace ChassisActions{

class RapidMoveAction:public ChassisAction{
protected:
    Vector2 dest_;
public:
    RapidMoveAction(Inst & inst, const Vector2 & pos):
        ChassisAction(UINT_MAX, [this](){
            inst_.rapid_move(dest_);
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
    RapidSpinAction(Inst & inst, const real_t rad):
        ChassisAction(UINT_MAX, [this](){
            inst_.rapid_spin(dest_);
            if(inst_.arrived()){
                this->kill();
            }
        }, inst),
        dest_(rad){};
};


class MoveAction:public ChassisAction{
protected:
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
    MoveAction(Inst & inst, const Vector2 & pos):
        ChassisAction(UINT_MAX, [this](){
            if(first()){
                init();
            }
            if(inst_.arrived()){
                this->kill();
            }

            auto time = this->time();
            inst_.rapid_move(from_ + norm_ * solver_->forward(time));

        }, inst),
        dest_(pos){};
};

class SpinAction:public ChassisAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;

    real_t current_;
    real_t dest_;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;

    void init(){
        current_ = inst_.feedback().rad;
        const auto & config = inst_.config(); 

        solver_.emplace(
            TrapezoidSolver{config.max_agr, config.max_spr, dest_ - current_}
        );
    }
public:
    SpinAction(Inst & inst, const real_t rad):
        ChassisAction(UINT_MAX, [this](){
            if(first()){
                init();
            }

            auto time = this->time();
            inst_.rapid_spin(current_ + solver_->forward(time));
            if(inst_.arrived()){
                this->kill();
            }
        }, inst),
        dest_(rad){};
};


class WaitArriveAction:public ChassisAction{
public:
    WaitArriveAction(Inst & inst):
        ChassisAction(UINT_MAX, [this](){
            if(inst_.arrived()) kill();
        }, inst)
        {};
};



class TrimAction:public ChassisAction{
public:
    TrimAction(Inst & inst):
        ChassisAction(UINT_MAX, [this](){
            // if(inst_.arrived()) kill();
            TODO("csd");
        }, inst)
        {};
};

}
};