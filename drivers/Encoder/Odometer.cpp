#include "odometer.hpp"

void Odometer::update(){
    {
        real_t undiredRawLapPostion = encoder.getLapPosition();
        if (rsv) rawLapPosition = real_t(1) - undiredRawLapPostion;
        else rawLapPosition = undiredRawLapPostion;
    }

    lapPosition = correctPosition(rawLapPosition);
    deltaLapPosition = lapPosition - lapPositionLast;

    static constexpr auto one = real_t(1);
    static constexpr auto half_one = real_t(1)/2;

    if(deltaLapPosition > half_one){
        deltaLapPosition -= one;
    }else if (deltaLapPosition < -half_one){
        deltaLapPosition += one;
    }

    lapPositionLast = lapPosition;
    accPosition += deltaLapPosition;
}

real_t OdometerPoles::correctPosition(const real_t rawPosition){
    auto pole_a = position2pole(rawPosition);
    auto err_a = cali_map[pole_a];

    auto pole_b = (pole_a + 1) % poles;
    auto err_b = cali_map[pole_b];

    // auto err
    auto ratio = frac(poles * frac(rawPosition));
    // auto ratio = 0;
    // auto err = LERP(err_a, err_b, ratio);
    auto err = LERP(err_a, err_b, ratio);

    // return (rawPosition - err_a);
    return (rawPosition - err);
}