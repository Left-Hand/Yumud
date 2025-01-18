#pragma once
#include "sys/math/real.hpp"


namespace ymd::drivers{

class EncoderIntf{
public:
    virtual real_t getLapPosition() = 0;
    virtual void update() = 0;
    virtual void init() = 0;
    virtual bool stable() = 0;
};

class IncrementalEncoderIntf: public EncoderIntf{

};

class AbsoluteEncoderIntf: public EncoderIntf{
protected:
    // virtual void forward() = 0;
    // virtual void backward() = 0;
};

}