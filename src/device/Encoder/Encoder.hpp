#ifndef __ENCODER_HPP__

#define __ENCODER_HPP__

#include "types/real.hpp"


class Encoder{
public:
    virtual real_t getLapPosition() = 0;
};

class IncrementalEncoder: public Encoder{

};

class AbsoluteEncoder: public Encoder{

};


#endif