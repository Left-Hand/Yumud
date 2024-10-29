#pragma once

#include "coil.hpp"

// class Coil3:public Coil3DriverConcept{
// protected:
//     PwmChannel & instanceU;
//     PwmChannel & instanceV;
//     PwmChannel & instanceW;
//     bool enabled = true;
// public:
//     Coil3(PwmChannel & _instanceU, PwmChannel & _instanceV, PwmChannel & _instanceW):
//             instanceU(_instanceU), instanceV(_instanceV), instanceW(_instanceW){;}

//     void init() override{
//         instanceU.init();
//         instanceV.init();
//         instanceW.init();
//     }

//     void setClamp(const real_t max_value){
//         real_t abs_max_value = abs(max_value);
//         instanceU.setClamp(abs_max_value);
//         instanceV.setClamp(abs_max_value);
//         instanceW.setClamp(abs_max_value);
//     }
// };