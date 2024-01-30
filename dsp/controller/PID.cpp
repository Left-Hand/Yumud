#include "PID.hpp"

template<typename T>
template<typename U>
T PID<T>::update(const U & _setpoint, const U & _pv){
    T setpoint = static_cast<T>(_setpoint);
    T pv = static_cast<T>(_pv);

	T error = setpoint - pv;
	T Pout = kp * error;
	
	_integral += error;
	T Iout = T(std::clamp(ki * _integral, clp_min, clp_max));
	
	T derivative = (error - err_last);
	T Dout = kd * derivative;
	
	T output = T(std::clamp(Pout + Iout + Dout, clp_min, clp_max));
	err_last = error;

	return T(output);
}