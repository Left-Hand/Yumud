#pragma once

#include "sys/math/real.hpp"
#include "Encoder.hpp"
#include <array>
#include "thirdparty/sstl/include/sstl/vector.h"

namespace ymd::drivers{
class Odometer{
public:
    EncoderIntf & encoder;
protected:
    real_t rawLapPosition;

    real_t lapPosition;
    real_t lapPositionLast;
    real_t deltaLapPosition;

    real_t accPosition;
    real_t accPositionLast;
    real_t speed;

    bool rsv = false;

    void locate(const real_t pos);
    virtual real_t correctPosition(const real_t rawPosition){
        return rawPosition;
    }

public:
    Odometer(const Odometer & other) = delete;
    Odometer(Odometer && other) = default;
    Odometer(EncoderIntf & _encoder):encoder(_encoder){
        reset();
    }

    real_t getRawLapPosition(){
        return rawLapPosition;
    }

    real_t getSpeed(){
        return speed;
    }

    real_t getLapPosition(){
        return lapPosition;
    }

    void init(){
        reset();
        locate(0);
    }

    virtual void reset();
    //将当前的位置视为某偏移
    void locateRelatively(const real_t offset){
        locate(offset);
    }

    //圈内寻址 设定偏移
    void locateAbsolutely(const real_t offset){
        locate(getLapPosition() + offset);
    }

    void inverse(const bool en = true){
        rsv = en;
    }

    void update();

    bool stable(){
        return encoder.stable();
    }

    virtual real_t getPosition(){
        return accPosition;
    }
};

}
