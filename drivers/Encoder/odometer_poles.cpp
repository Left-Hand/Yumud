#include "odometer_poles.hpp"

void OdometerPoles::reset(){
    Odometer::reset();
    elecrad_cache = real_t(0);

    for(size_t i = 0; i < cali_map.size(); i++){
        cali_map[i] = 0;
    }
}


real_t OdometerPoles::correctPosition(const real_t rawPosition){
    auto pole_a = position2pole(rawPosition);
    auto err_a = cali_map[pole_a];

    // auto poles = cali_map.size();
    // auto pole_b = (pole_a + 1) % poles;
    // auto err_b = cali_map[pole_b];


    // auto ratio = frac(poles * frac(rawPosition));
    // auto err = LERP(err_a, err_b, ratio);
    auto err = err_a;
    
    return (rawPosition - err);
}