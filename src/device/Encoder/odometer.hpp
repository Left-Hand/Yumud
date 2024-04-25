#ifndef __ODOMETER_HPP__

#define __ODOMETER_HPP__

#include "types/real.hpp"
#include "Encoder.hpp"

// #include "OdometerLines.hpp"

namespace MotorUntils{

};

// class Odometer;
// class OdometerLines;
// class OdometerPoles;


class Odometer{
protected:
    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);

    real_t accPosition = real_t(0);
    real_t accPositionLast = real_t(0);

    Encoder & encoder;
    bool rsv = false;

    void locate(const real_t & pos){
        lapPosition = getLapPosition();
        lapPositionLast = lapPosition;
        accPosition = pos;
    }


public:
    Odometer(Encoder & _encoder):encoder(_encoder){;}

    real_t getLapPosition(){
        return lapPosition;
    }

    void init(){
        encoder.init();
    }

    void locateRelatively(const real_t & offset = real_t(0)){
        locate(offset);
    }

    void locateAbsolutely(const real_t & offset = real_t(0)){
        locate(getLapPosition() + offset);
    }

    virtual void reset(){
        lapPosition = real_t(0);
        lapPositionLast = real_t(0);

        accPosition = real_t(0);
        accPositionLast = real_t(0);
    }

    void inverse(const bool en = true){
        rsv = en;
    }

    void update(){
        {
            real_t undiredLapPostion = encoder.getLapPosition();
            if (rsv) lapPosition = real_t(1) - undiredLapPostion;
            else lapPosition = undiredLapPostion;
        }
        real_t deltaLapPosition = lapPosition - lapPositionLast;

        if(deltaLapPosition > real_t(0.5f)){
            deltaLapPosition -= real_t(1);
        }else if (deltaLapPosition < real_t(-0.5f)){
            deltaLapPosition += real_t(1);
        }

        lapPositionLast = lapPosition;
        accPosition += deltaLapPosition;
    }

    virtual real_t getPosition(){
        return accPosition;
    }
};


class OdometerScaled:public Odometer{
protected:
    real_t scale;
public:
    OdometerScaled(Encoder & _encoder, const real_t & _scale):
        Odometer(_encoder),scale(_scale){;}
};


class OdometerPoles:public Odometer{
protected:
    real_t poles;
    real_t elecRad = real_t(0);
    real_t elecRadOffset = real_t(0);




public:
    OdometerPoles(Encoder & _encoder, const int & _poles):
            Odometer(_encoder), poles(_poles){;}

    void reset() override{
        Odometer::reset();
        elecRad = real_t(0);
        elecRadOffset = real_t(0);
    }

    real_t getElecRad(){
        return position2rad(lapPosition) - elecRadOffset;
    }

    real_t getElecRadOffset(){
        return elecRadOffset;
    }

    void locateElecrad(const real_t & percentage = real_t(1)){
        elecRadOffset += position2rad(getLapPosition()) * percentage;
    }

    real_t position2rad(const real_t & position){
        real_t frac1 = real_t(poles) * frac(position);
        return real_t(TAU) * (frac(frac1));
    }

    int position2pole(const iq_t & position){
        real_t pole = frac(position) * real_t(poles);
        return MIN(int(pole), poles - 1);
    }
};

#endif