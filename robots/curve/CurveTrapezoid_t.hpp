#pragma once

#include "CurveConcept_t.hpp"

namespace ymd::curve{

    
template<typename T>
class CurveTrapezoid_t:public CurveConcept_t<T>{
protected:

    // struct Config{}
    const T _norm;
    real_t a_;
    real_t v_;
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
public:
    CurveTrapezoid_t(const CurveTrapezoid_t<T> & other) = default;
    CurveTrapezoid_t(CurveTrapezoid_t<T> && other) = default;

    CurveTrapezoid_t(const T & from, const T & to, real_t v, real_t a):
        CurveConcept_t<T>(from, to),
        _norm(normal(from, to)),
        a_(ABS(a)), v_(ABS(v)), s_(distance(from, to)){

        a = a_;
        v = v_;
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
            return v_;
        }else{
            return a_ * t1;
        }
    }
};


}