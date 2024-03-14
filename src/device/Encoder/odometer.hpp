#ifndef __ODOMETER_HPP__

#define __ODOMETER_HPP__

#include "types/real.hpp"
#include "Encoder.hpp"

namespace MotorUntils{

};


class Odometer{
protected:
    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);

    real_t accPosition = real_t(0);
    real_t accPositionLast = real_t(0);
    real_t elecRad = real_t(0);
    real_t elecRadOffset = real_t(0);

    Encoder & encoder;
    uint8_t poles;
    bool rsv = true;

    real_t position2rad(const real_t & position){
        real_t frac1 = real_t(poles) * frac(position);
        return real_t(TAU) * (frac(frac1));
    }

    int position2pole(const iq_t & position){
        real_t pole = frac(position) * real_t(poles);
        return int(pole);
    }

    void locate(const real_t & pos){
        lapPosition = getLapPosition();
        lapPositionLast = lapPosition;
        accPosition = pos;
    }

    real_t getLapPosition(){
        real_t undiredLapPostion = encoder.getPosition();
        if (rsv) return real_t(real_t(1) - undiredLapPostion);
        else return real_t(undiredLapPostion);
    }
public:
    Odometer(Encoder & _encoder, const uint8_t _poles):encoder(_encoder), poles(_poles){;}
    void locateRelatively(const real_t & offset = real_t(0)){
        locate(offset);
    }

    void locateAbsolutely(const real_t & offset = real_t(0)){
        locate(getLapPosition() + offset);
    }


    void locateElecrad(){
        elecRadOffset = position2rad(getLapPosition());
    }

    real_t update(){
        lapPosition = getLapPosition();

        real_t deltaLapPosition = lapPosition - lapPositionLast;

        if(deltaLapPosition > real_t(0.5f)){
            deltaLapPosition -= real_t(1);
        }else if (deltaLapPosition < real_t(-0.5f)){
            deltaLapPosition += real_t(1);
        }

        // elecRad += position2rad(deltaLapPosition);
        // elecRad = fmod(elecRad, real_t(TAU));
        elecRad = position2rad(lapPosition);

        lapPositionLast = lapPosition;
        accPosition += deltaLapPosition;

        return accPosition;
    }

    real_t getElecRad(){
        return elecRad - elecRadOffset;
    }

    real_t getPosition(){
        return accPosition;
    }
};

#endif