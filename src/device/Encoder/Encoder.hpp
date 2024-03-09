#ifndef __ENCODER_HPP__

#define __ENCODER_HPP__

#include "types/real.hpp"


class Encoder{
public:
    virtual real_t getPosition() = 0
    virtual real_t setHomePosition() = 0;
};

class IncrementalEncoder: public Encoder{

};

class AbsoluteEncoder: public Encoder{

};


#endif