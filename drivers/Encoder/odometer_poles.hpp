#pragma once

#include "odometer.hpp"

class OdometerPoles:public Odometer{
protected:
    static constexpr int max_poles = 50;
    real_t elecrad_cache = real_t(0);

    sstl::vector<real_t, max_poles>cali_map;
    const size_t poles;

    real_t correctPosition(const real_t rawPosition) override;
public:
    OdometerPoles(Encoder & _encoder, const size_t _poles = 50):
            Odometer(_encoder),
            poles(_poles)
        {
            for(size_t i = 0; i < poles; i++) cali_map.push_back(0);    
        }

    void reset() override;

    real_t getElecRad(){
        return position2rad(getLapPosition());
    }

    real_t getElecRadFixed(){
        return position2rad(getLapPosition() + deltaLapPosition);
    }

    int getRawPole(){
        return position2pole(getRawLapPosition());
    }

    auto & map(){
        return cali_map;
    }

    real_t position2rad(const real_t position){
        real_t frac1 = int(poles) * frac(position);
        return real_t(TAU) * (frac(frac1));
    }

    int position2pole(const iq_t position){
        int pole = int(frac(position) * poles);
        return MIN(pole, poles - 1);
    }

    real_t pole2position(const int pole){
        return real_t(pole) / 50;
    }
};
