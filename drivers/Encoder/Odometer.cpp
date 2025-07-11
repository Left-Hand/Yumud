#include "odometer.hpp"

using namespace ymd;
using namespace ymd::drivers;


void Odometer::reset(){
    rawLapPosition = real_t(0);
    lapPosition = real_t(0);
    lapPositionLast = real_t(0);
    deltaLapPosition = real_t(0);
    accPosition = real_t(0);
    accPositionLast = real_t(0);
}

void Odometer::update(){
    encoder.update().unwrap();
    real_t undiredRawLapPostion = encoder.read_lap_position().unwrap();
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

    const real_t new_speed = deltaLapPosition * 25000;
    // speed = (speed * 127 + new_speed) >> 7;
    speed = (speed * 31 + new_speed) >> 5;
    // speed = (speed * 15 + new_speed) >> 4;
}


void Odometer::locate(const real_t pos){
    update();
    lapPosition = getLapPosition();
    lapPositionLast = lapPosition;
    accPosition = pos;
}