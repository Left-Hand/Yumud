#pragma once
#include "core/math/real.hpp"


namespace ymd::drivers{

class EncoderIntf{
public:
    virtual real_t get_lap_position() = 0;
    virtual void update() = 0;
    virtual void init() = 0;
    virtual bool stable() = 0;

    virtual ~EncoderIntf() = default;
};

class IncrementalEncoderIntf: public EncoderIntf{

};

class AbsoluteEncoderIntf: public EncoderIntf{
protected:
    // virtual void forward() = 0;
    // virtual void backward() = 0;
};

}