#pragma once

#include "sys/core/platform.h"

namespace gxm{
    
template<arithmetic T>
class TrapezoidSolver_t{
protected:
    T a_;
    T v_;
    T s_;

    T t1;
    T t2;
    T t_all;
    T s1;
    bool peaked;
    
public:
    DELETE_COPY_AND_MOVE(TrapezoidSolver_t);

    TrapezoidSolver_t(const T & a, const T & v, const T & s):
        a_(a), v_(v), s_(s) {
            // v = at
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
        peaked = s > reach_peak_threshold;
        if(peaked == false){

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

    T forward(const T t) const{
        if(peaked){
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

    T period() const {
        return t_all;
    }

    T peak(){
        if(peaked){
            return v_;
        }else{
            return a_ * t1;
        }
    }
};


}