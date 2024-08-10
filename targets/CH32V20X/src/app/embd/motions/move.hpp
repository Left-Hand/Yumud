#pragma once

#include "../../robots/actions/actions.hpp"

struct MoveAction:public Action{
protected:
    using Machine = XY_Machine;
private:
    Machine & machine;

    using Action::abort;
protected:
    const Vector2 end;
    Vector2 begin;

    static constexpr real_t max_spd = real_t(0.4);
    static constexpr real_t max_acc = real_t(1);

    void move_to(const Vector2 & pos){
        machine.xy_mm(pos * 1000);
    }

    void abort_until_done(){
        // if (machine.reached(end * 1000))
        abort();
    }

    virtual void do_move() = 0;
    virtual void initial_calculate(){;}

    void execute() override{
        if(first()){
            begin = machine.xy_mm() / 1000;
            initial_calculate();
        }
        do_move();

    }
public:
    MoveAction(Machine & _machine, const Vector2 & _end):
            Action([this](){this->execute();}, UINT_MAX, false), machine(_machine), end(_end / real_t(1000)){}
};


struct RapidMoveAction:public MoveAction{
protected:
    void do_move() override{
        move_to(end);
        abort_until_done();
    }
public:
    RapidMoveAction(Machine & _machine, const Vector2 & _end):
            MoveAction(_machine, _end){}
};


struct LineMoveAction:public MoveAction{
protected:
    real_t elapsed;

    void initial_calculate() override{
        real_t distance = (end - begin).length();
        elapsed = distance / max_spd;
    }

    void do_move() override{
        real_t ratio = since() / elapsed;
        if(ratio > 1) return abort_until_done();
        move_to(begin.lerp(end, ratio));
    }
public:
    LineMoveAction(Machine & _machine, const Vector2 & _end):
            MoveAction(_machine, _end){}
};


struct TrapezoidMoveAction:public MoveAction{
protected:
    real_t t1;
    real_t t2;
    real_t t_all;

    real_t s1;
    real_t s_all;

    Vector2 norm;

    void initial_calculate() override{
        /*   /\
            /  \
           /    \  */

        const real_t accleration_time_threshold = max_spd / max_acc;
        const real_t not_full_distance_threshold = max_spd * accleration_time_threshold; 
        const real_t distance = (end - begin).length();
        if(distance < not_full_distance_threshold){

            //v ^ 2 = a * x
            real_t real_max_spd = sqrt(max_acc * distance);
            t1 = real_max_spd / max_acc;
            t2 = t1;
            

            s1 = t1 * real_max_spd / 2;
            s_all = distance;

            t_all = 2 * t1;
        }else{
            t1 = accleration_time_threshold;
            t2 = ((distance - not_full_distance_threshold) / max_spd) + t1;

            s1 = max_acc * t1 * t1 / 2;
            s_all = distance;
            t_all = t2 + t1;
        }

        norm = (end - begin) / distance;
    }

    void do_move() override{
        real_t time = since();
        if(time > t_all) return abort_until_done();

        #define SD (max_acc * time * time / 2)
        #define TRANS(v) ((norm * (v)) + begin)
        if(time < t1) return move_to(TRANS(SD));
        if(time < t2){
            time -= t1;
            return move_to(TRANS(s1 + (time) * max_spd));
        } 
        else{
            time = t_all - time;
            return move_to(TRANS(s_all - SD));
        }

    }
public:
    TrapezoidMoveAction(Machine & _machine, const Vector2 & _end):
            MoveAction(_machine, _end){}
};
