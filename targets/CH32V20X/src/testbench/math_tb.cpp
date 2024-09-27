#include "tb.h"

#include "sys/debug/debug_inc.h"

#include "types/matrix/matrix.hpp"
#include "types/matrix/ceres/ceres.hpp"
#include "types/plane/plane_t.hpp"


void math_tb(UartHw & logger){

    logger.init(576000, CommMethod::Blocking);
    logger.setEps(4);


    Plane plane = {Vector3{1, 1, 1}, 1};
    
    while(true){
        
        DEBUG_PRINTLN(plane, 1 / sqrt(real_t(3)));

        Sys::Clock::reCalculateTime();
    }
}