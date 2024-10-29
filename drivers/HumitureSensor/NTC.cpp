#include "NTC.hpp"

void NTC::update(){
    scexpr real_t T0= real_t(273.15+25);
    scexpr real_t Ka= real_t(273.15);
    real_t VR = get_uniV();
    real_t Rt_kOhms = (VR)/(1-VR) * R_kOhms;
    auto this_temp = real_t(real_t(B_)/(real_t(B_)/T0+log(Rt_kOhms/R0_kOhms))) - Ka + real_t(0.5);
    last_temp = (last_temp * 15 + this_temp) >> 4;
}