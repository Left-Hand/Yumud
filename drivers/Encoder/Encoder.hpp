#ifndef __ENCODER_HPP__

#define __ENCODER_HPP__

#include "types/real.hpp"


class Encoder{
public:
    virtual real_t getLapPosition() = 0;
    virtual void init() = 0;
    virtual bool stable() const = 0;
};

class IncrementalEncoder: public Encoder{

};

class AbsoluteEncoder: public Encoder{
protected:
    // virtual void forward() = 0;
    // virtual void backward() = 0;
};


#endif