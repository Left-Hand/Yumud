#pragma once

#include "core/platform.hpp"
#include "core/math/realmath.hpp"

namespace ymd::robots{
    
template<arithmetic T>
class TrapezoidSolver final{
public:
    struct Config{
        T max_acc;
        T max_speed;
    };

    constexpr TrapezoidSolver(const Config & cfg, T s):
        a_(ABS(cfg.max_acc)), 
        v_(ABS(cfg.max_speed)), 
        s_(ABS(s))
    {

        const T a = a_;
        const T v = v_;

        is_inversed_ = (s < 0);
        s = ABS(s);

        const T accleration_time = v / a;


        /*   /\       ____
            /  \  =>      |
           /    \         | */

        const T reach_peak_threshold = v * accleration_time; 


        /*      s1
                 ____   
                /    \  
               /      \ 
              /        \  
             /          \ */
        //  0  t1   t2  ta 
        reached_max_speed_ = s > reach_peak_threshold;
        if(reached_max_speed_ == false){

            //v ^ 2 = a * x
            T real_max_spd = sqrt(a * s);
            
            t1 = real_max_spd / a;
            t2 = t1;
            
            s1 = t1 * real_max_spd >> 1;

            t_all = 2 * t1;
        }else{
            t1 = accleration_time;
            t2 = ((s - reach_peak_threshold) / v) + t1;

            s1 = a * t1 * t1 >> 1;
            t_all = t2 + t1;
        }   
    }

    constexpr T forward(const T t)const{
        return is_inversed_ ? -_forward(t) : _forward(t);
    }

    constexpr T elapsed() const {
        return t_all;
    }

    constexpr T peak_speed(){
        if(reached_max_speed_){
            return v_;
        }else{
            return a_ * t1;
        }
    }

private:
    T a_ = 0;
    T v_ = 0;
    T s_ = 0;

    T t1 = 0;
    T t2 = 0;
    T t_all = 0;
    T s1 = 0;
    bool reached_max_speed_ = false;
    bool is_inversed_ = false;

    T _forward(const T t) const{
        if(reached_max_speed_){
            if(t < t1){
                return (a_ * square(t)) >> 1;
            }else if(t < t2){
                return s1 + v_ * (t - t1);
            }else if(t < t_all){
                return s_ - ((a_ * square(t - t_all)) >> 1);
            }
        }else{
            if(t < t1){
                return a_ * square(t) >> 1;
            }else if(t < t_all){
                return s_ - ((a_ * square(t - t_all)) >> 1);
            }
        }
        return s_;
    }
};


}