#pragma once

#include "odometer.hpp"

class OdometerPoles:public Odometer{
protected:
    static constexpr int max_poles = 50;
    using Map = sstl::vector<real_t, max_poles>;
    using CompressedMap = sstl::vector<int16_t, max_poles>;
    
    const size_t poles;
    real_t elecrad_cache = real_t(0);

    Map cali_map;

    real_t correctPosition(const real_t rawPosition) override;
public:
    OdometerPoles(Encoder & _encoder, const size_t _poles):
            Odometer(_encoder),
            poles(_poles)
        {
            cali_map.clear();
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

    template<size_t N>
    void decompress(const std::array<int16_t, N> & data){
        if(N != poles) CREATE_FAULT
        for(size_t i = 0; i < N;i++){
            cali_map[i] = real_t(data[i]) / 16384;
        }
    }

    auto compress(const real_t radfix) const{
        CompressedMap ret;
        for(const auto & item : cali_map){
            scexpr auto ratio = real_t(1 / TAU);
            ret.push_back(int16_t((item - (radfix / poles * ratio)) * 16384));
        }
        return ret;
    }

    real_t position2rad(const real_t position){
        real_t frac1 = int(poles) * frac(position);
        return real_t(TAU) * (frac(frac1));
    }

    int position2pole(const iq_t position){
        int pole = int(frac(position) * int(poles));
        return MIN(pole, poles - 1);
    }

    real_t pole2position(const int pole){
        return real_t(pole) / poles;
    }
};
