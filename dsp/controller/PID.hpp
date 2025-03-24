#pragma once

#include "core/math/real.hpp"
#include <algorithm>

template<typename T>
class PID_t{
    protected:
        T err = T(0.0);
        T err_last = T(0.0);
        T m_integral = T(0.0);

        T clp_min = T(-1.0f);
        T clp_max = T(1.0f);


    public:
        T kp = T(0.0);
        T ki = T(0.0);
        T kd = T(0.0);

        PID_t() = delete;

        template<typename U>
        PID_t(const U & _kp,const U & _ki = U(0),const U & _kd = U(0), const U & _cl = U(1)):
                kp(static_cast<T>(_kp)), ki(static_cast<T> (_ki)), kd(static_cast<T>(_kd)){
                    setClamp(_cl);
                }

        // template<typename U>
        // T update(const auto & _error){

        // }
        auto update(const auto & _target, const auto & _value, const auto & _value_integral, const auto & _value_derivative){
            T target = static_cast<T>(_target);
            T value = static_cast<T>(_value);

            // T error = setpoint - value;
            // T Pout = kp * error;

            // _integral += error;
            // T Iout = ;

            // T derivative = (error - err_last);
            // T Dout = kd * derivative;

            T output = CLAMP(
                kp * (target - value) + ki * _value_integral + kd * _value_derivative,
                clp_min, clp_max);

            return T(output);
        }


        auto update(const auto & _target, const auto & _value, const auto & _value_derivative){
            // T error  = static_cast<T>(_target - _value);
            m_integral = CLAMP(m_integral + _target - _value, clp_min, clp_max);
            auto ret = update(_target, _value, m_integral, -static_cast<T>(_value_derivative));
            // err_last = err;
            return ret;
        }

        auto update(const auto & _target, const auto & _value){
            T error  = static_cast<T>(_target - _value);
            auto ret = update(_target, _value, m_integral, error - err_last);
            err_last = err;
            return ret;
        }

        template<typename U>
        void setClamp(const U & _clp){
            clp_min = static_cast<T>(-abs(_clp)),
            clp_max = static_cast<T>(abs(_clp));}

        template<typename U>
        void setClamp(const U & _clp_min, const U & _clp_max){
            clp_min = static_cast<T>(_clp_min),
            clp_max = static_cast<T>(_clp_max);}
};

#include "pid.tpp"

typedef PID_t<real_t> PID;


