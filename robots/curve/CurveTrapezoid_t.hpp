#pragma once

#include "CurveConcept_t.hpp"
#include "core/math/realmath.hpp"

namespace ymd::curve{

    
template<typename T>
class CurveTrapezoid:public CurveIntf<T>{
protected:

    struct Config{
        T from; 
        T to; 
        T v;
        T a;
    };

    const T _norm;
    real_t abs_acc_;
    real_t abs_spd_;
    real_t s_;

    real_t t1;
    real_t t2;
    real_t t_all;
    real_t s1;
    bool peaked;
    bool inversed;

    real_t s_forward(const real_t t) const{
        if(peaked){
            if(t < t1){
                return (abs_acc_ * square(t)) >> 1;
            }else if(t < t2){
                return s1 + abs_spd_ * (t - t1);
            }else if(t < t_all){
                return s_ - ((abs_acc_ * square(t - t_all)) >> 1);
            }
        }else{
            if(t < t1){
                return abs_acc_ * square(t) >> 1;
            }else if(t < t_all){
                return s_ - ((abs_acc_ * square(t - t_all)) >> 1);
            }
        }
        return s_;
    }
public:
    CurveTrapezoid(const CurveTrapezoid<T> & other) = default;
    CurveTrapezoid(CurveTrapezoid<T> && other) = default;

    CurveTrapezoid(const Config & cfg):
        CurveIntf<T>(cfg.from, cfg.to),
        _norm(normal(cfg.from, cfg.to)),
        abs_acc_(ABS(cfg.a)), abs_spd_(ABS(cfg.v)), s_(distance(cfg.from, cfg.to)){

        const auto a = abs_acc_;
        const auto v = abs_spd_;
        auto s = s_;

        const auto accleration_time = v / a;


        /*   /\       ____
            /  \  =>      |
           /    \         | */
           
        const auto reach_peak_threshold = v * accleration_time; 


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
            const auto real_max_spd = sqrt(a * s);
            
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

    T forward(const real_t t) const override{
        return this->_from + s_forward(t) * this->_norm;
    }

    real_t period() const override{
        return t_all;
    }

    T peak(){
        if(peaked){
            return abs_spd_;
        }else{
            return abs_acc_ * t1;
        }
    }
};


}