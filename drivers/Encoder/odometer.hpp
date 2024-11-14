#pragma once

#include "sys/math/real.hpp"
#include "Encoder.hpp"
#include <array>
#include "thirdparty/sstl/include/sstl/vector.h"

namespace ymd::drivers{
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

}
