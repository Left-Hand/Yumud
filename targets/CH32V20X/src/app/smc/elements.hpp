#ifndef __ELEMENTS_HPP__

#define __ELEMENTS_HPP__

#include "../src/enum.hpp"
#include "finder.hpp"
// #include "enum.hpp"

namespace SMC{
BETTER_ENUM(ElementType, uint8_t,
    NONE,
    RING,
    CROSS,
    STOP
    );

BETTER_ENUM(State, uint8_t,
    MEET,
    STRAIGHT,
    ENTRY,
    ONWAY,
    EXIT,
    RECOVER
    );
struct RingConfig{


    State state;
};

class CrossConfig{

    
    State state;
};


class Element{};
class Ring : public RingConfig, Element{

};


class Cross :public CrossConfig, Element{

};
};


#endif