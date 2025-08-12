#pragma once

#include "grab_module.hpp"
#include "../autodrive/sequence/TrapezoidSolver_t.hpp"
#include "types/vectors/vector3.hpp"


namespace ymd::robots::GrabActions{
class PressAction:public GrabAction{
public:
    ACTION_NAME(press)
    PressAction(Inst & inst):
        GrabAction(inst.config().nozzle_sustain, [this](){
            if(first()){
                inst_.meta_press();
                // DEBUG_PRINTLN("[presss]")
            }
        }, inst){};
};


class ReleaseAction:public GrabAction{
public:
    ACTION_NAME(release)
    ReleaseAction(Inst & inst):
        GrabAction(inst.config().nozzle_sustain, [this](){
            if(first()){
                inst_.meta_release();
                // DEBUG_PRINTLN("[reasss]")
            }
        }, inst){};
};

class TestAction:public GrabAction{
public:
    ACTION_NAME(test)
    TestAction(Inst & inst):
        GrabAction(200, [this](){
            // DEBUG_PRINTLN(millis())
        }, inst){};
};

class MoveAction:public GrabAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;

    Vector3<real_t> to_;
    Vector3<real_t> from_;
    real_t dist_;
    Vector3<real_t> norm_;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;
    real_t dur_;

    void init(const Vector3<real_t> & from){
        from_ = (from);
        dist_ = ((to_ - from).length());
        norm_ = ((to_ - from) / dist_);

        solver_.emplace(TrapezoidSolver{
            inst_.config().max_acc, 
            inst_.config().max_spd,
            dist_
        });

        dur_ = {solver_->period()};
    }
public:
    ACTION_NAME(move)
    MoveAction(Inst & inst, const Vector3<real_t> & to):
        GrabAction(UINT32_MAX, [this](){
            if(this->first()){//将计算延迟到实际开始运行而不是构造时 确保起始位置正确
                init(inst_.getPos());
            }

            auto time = this->time();
            if(time > dur_){
                // DEBUG_PRINTLN("killed")
                this->kill();
            }
            auto dest_pos = from_ + norm_ * solver_->forward(time);
            // DEBUG_PRINTLN(pos.x, pos.y, pos.z, time, dur_);
            inst_.rapid(dest_pos);
            // auto pos = inst_.getPos();
            // DEBUG_PRINTLN(pos.x, pos.y, pos.z, time);
        }, inst),
        to_(to)
        {};
};

class MoveZAction:public GrabAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;

    real_t to_;
    Vector3<real_t> from_;
    real_t dist_;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;
    real_t dur_;

    void init(const Vector3<real_t> & from){
        from_ = (from);
        dist_ = to_ - from.z;
        // dist_ = ABS(to - from);
        
        solver_.emplace(TrapezoidSolver{
            inst_.config().max_z_acc, 
            inst_.config().max_z_spd,
            // 0.01_r,
            // 0.01_r,
            dist_
        });

        dur_ = {solver_->period()};
    }
public:
    ACTION_NAME(movez)
    MoveZAction(Inst & inst, const real_t to):
        GrabAction(UINT32_MAX, [this](){
            if(this->first()){//将计算延迟到实际开始运行而不是构造时 确保起始位置正确
                init(inst_.getPos());
            }

            auto time = this->time();
            if(time > dur_){
                this->kill();
            }
            auto dest_pos = Vector3<real_t>(from_.xy(), from_.z + solver_->forward(time));
            // DEBUG_PRINTLN(dest_pos.x, dest_pos.y, dest_pos.z, time, dist_);
            inst_.rapid(dest_pos);
            // auto pos = inst_.getPos();
            // DEBUG_PRINTLN(pos.x, pos.y, pos.z, time);
        }, inst),
        to_(to)
        {};
};

class MoveXYAction:public GrabAction{
protected:
    using TrapezoidSolver = TrapezoidSolver_t<real_t>;

    Vector2<q16> to_;
    Vector3<real_t> from_;
    real_t dist_;
    Vector2<q16> norm_;
    std::optional<TrapezoidSolver> solver_ = std::nullopt;
    real_t dur_;

    void init(const Vector3<real_t> & from){
        from_ = (from);
        dist_ = (to_ - from.xy()).length();
        norm_ = ((to_ - from.xy()) / dist_);

        solver_.emplace(TrapezoidSolver{
            inst_.config().max_acc, 
            inst_.config().max_spd,
            dist_
        });

        dur_ = {solver_->period()};
    }
public:
    ACTION_NAME(movexy)
    MoveXYAction(Inst & inst, const Vector2<q16> to):
        GrabAction(UINT32_MAX, [this](){
            if(this->first()){//将计算延迟到实际开始运行而不是构造时 确保起始位置正确
                init(inst_.getPos());
            }

            auto time = this->time();
            if(time > dur_){
                this->kill();
            }
            auto dest_pos = Vector3<real_t>(from_.xy() + norm_ * solver_->forward(time), from_.z);
            // DEBUG_PRINTLN(dest_pos.x, dest_pos.y, dest_pos.z);
            inst_.rapid(dest_pos);
            // auto pos = inst_.getPos();
            // DEBUG_PRINTLN(pos.x, pos.y, pos.z, time);
            // DEBUG_PRINTLN(dest_pos.x, dest_pos.y);
        }, inst),
        to_(to)
        {};
};

class ReachedAction:public GrabAction{
public:
    ACTION_NAME(reached)
    ReachedAction(Inst & inst):
        GrabAction(UINT32_MAX, [this](){
            if(inst_.reached()) kill();
        }, inst)
        {};
};

class RapidAction:public GrabAction{
protected:
    Vector3<real_t> to_;
public:
    ACTION_NAME(rapid)
    RapidAction(Inst & inst, const Vector3<real_t> & to):
        GrabAction(UINT32_MAX, [this](){
            if(first()){
                inst_.rapid(to_);
            }

            // DEBUG_PRINTLN("rapid", to_);
            // DEBUG_PRINTLN("rapid, re",/ inst_.reached());
            if(inst_.reached()){
                kill();
                // DEBUG_PRINTLN("rapid killed", uint32_t(this));
            }
        }, inst),
        to_(to)
        {};
};

class IdleAction:public GrabAction{
protected:
public:
    ACTION_NAME(rapid)
    IdleAction(Inst & inst):
        GrabAction(1, [this](){
            inst_.meta_idle();
        }, inst)
        {};
};



}