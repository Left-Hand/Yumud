#include "odometer.hpp"

void Odometer::update(){

    {
        encoder.update();
        real_t undiredRawLapPostion = encoder.getLapPosition();
        if (rsv) rawLapPosition = real_t(1) - undiredRawLapPostion;
        else rawLapPosition = undiredRawLapPostion;
    }

    lapPosition = correctPosition(rawLapPosition);
    deltaLapPosition = lapPosition - lapPositionLast;

    scexpr auto one = real_t(1);
    scexpr auto half_one = real_t(1)/2;

    if(deltaLapPosition > half_one){
        deltaLapPosition -= one;
    }else if (deltaLapPosition < -half_one){
        deltaLapPosition += one;
    }

    lapPositionLast = lapPosition;
    accPosition += deltaLapPosition;
}

