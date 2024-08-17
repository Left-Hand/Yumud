#ifndef __ODOMETER_HPP__

#define __ODOMETER_HPP__

#include "types/real.hpp"
#include "Encoder.hpp"
#include <array>

class Odometer{
public:
    Encoder & encoder;
protected:
    real_t rawLapPosition = real_t(0);

    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);
    real_t deltaLapPosition = real_t(0);

    real_t accPosition = real_t(0);
    real_t accPositionLast = real_t(0);

    bool rsv = false;

    void locate(const real_t pos){
        update();
        lapPosition = getLapPosition();
        lapPositionLast = lapPosition;
        accPosition = pos;
    }

    virtual real_t correctPosition(const real_t rawPosition){
        return rawPosition;
    }

public:
    Odometer(Encoder & _encoder):encoder(_encoder){;}

    real_t getRawLapPosition(){
        return rawLapPosition;
    }

    real_t getLapPosition(){
        return lapPosition;
    }

    void init(){
        encoder.init();
        locate(0);
    }

    void locateRelatively(const real_t offset = 0){
        locate(offset);
    }

    void locateAbsolutely(const real_t offset = 0){
        locate(getLapPosition() + offset);
    }

    virtual void reset(){
        rawLapPosition = real_t(0);
        lapPosition = real_t(0);
        lapPositionLast = real_t(0);

        accPosition = real_t(0);
        accPositionLast = real_t(0);
    }

    void inverse(const bool en = true){
        rsv = en;
    }

    void update();

    virtual real_t getPosition(){
        return accPosition;
    }
};


class OdometerScaled:public Odometer{
protected:
    real_t scale;
public:
    OdometerScaled(Encoder & _encoder, const real_t _scale):
        Odometer(_encoder),scale(_scale){;}
};

class OdometerPoles:public Odometer{
protected:
    static constexpr int poles = 50;
    real_t elecrad_cache = real_t(0);

    std::array<real_t, poles>cali_map;

    real_t correctPosition(const real_t rawPosition) override;
public:
    OdometerPoles(Encoder & _encoder):Odometer(_encoder){;}

    void reset() override{
        Odometer::reset();
        elecrad_cache = real_t(0);
        cali_map.fill(real_t(0));
    }

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
        real_t frac1 = poles * frac(position);
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

#endif