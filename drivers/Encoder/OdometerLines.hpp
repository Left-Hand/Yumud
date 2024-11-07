#pragma once

#include "Odometer.hpp"


namespace yumud::drivers{
class OdometerScaled:public Odometer{
protected:
    real_t scale;
public:
    OdometerScaled(Encoder & _encoder, const real_t _scale):
        Odometer(_encoder),scale(_scale){;}
};

class OdometerLines:public OdometerScaled{
public:
    OdometerLines(Encoder & _encoder, const int _lines):
        OdometerScaled(_encoder, real_t(real_t(16384) / real_t(_lines))){;}
    real_t getPosition() override{
        return Odometer::getPosition() * scale;
    }
};
}