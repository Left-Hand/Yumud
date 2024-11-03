#pragma once
#include "sys/math/real.hpp"


namespace yumud::drivers{
class Encoder{
public:
    virtual real_t getLapPosition() = 0;
    virtual void update() = 0;
    virtual void init() = 0;
    virtual bool stable() = 0;
};

class IncrementalEncoder: public Encoder{

};

class AbsoluteEncoder: public Encoder{
protected:
    // virtual void forward() = 0;
    // virtual void backward() = 0;
};

}