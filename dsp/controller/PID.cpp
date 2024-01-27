#include "PID.hpp"

real_t PID::update(const real_t & setpoint, const real_t & pv){
	real_t error = setpoint - pv;
	real_t Pout = kp * error;
	
	_integral += error;
	real_t Iout = real_t(std::clamp(ki * _integral, clp_min, clp_max));
	
	real_t derivative = (error - err_last);
	real_t Dout = kd * derivative;
	
	real_t output = real_t(std::clamp(Pout + Iout + Dout, clp_min, clp_max));
	err_last = error;

	return real_t(output);
}