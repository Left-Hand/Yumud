#pragma once

#include "Odometer.hpp"

namespace ymd::drivers{

class OdometerLines:public OdometerScaled{
public:
    OdometerLines(Encoder & _encoder, const int & _lines):
            // OdometerScaled(_encoder, real_t((uint32_t)(16384  << 16) / _lines) >> 16){;}
                OdometerScaled(_encoder, real_t(real_t(16384) / real_t(_lines))){;}
    real_t getPosition() override{
        return Odometer::getPosition() * scale;
    }
};


};