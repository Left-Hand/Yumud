#include "odometer.hpp"

using namespace ymd;
using namespace ymd::drivers;


void Odometer::update(){
    encoder.update();
    real_t undiredRawLapPostion = encoder.getLapPosition();
    if (rsv){
        rawLapPosition = real_t(1) - undiredRawLapPostion;
    }else{
        rawLapPosition = undiredRawLapPostion;
    }

    lapPosition = correctPosition(rawLapPosition);
    deltaLapPosition = lapPosition - lapPositionLast;

    scexpr auto half_one = real_t(1)/2;

    if(deltaLapPosition >= half_one){
        deltaLapPosition -= 1;
    }else if (deltaLapPosition <= -half_one){
        deltaLapPosition += 1;
    }

    lapPositionLast = lapPosition;
    accPosition += deltaLapPosition;
}

