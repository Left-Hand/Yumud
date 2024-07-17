#ifndef __ELEMENTS_HPP__

#define __ELEMENTS_HPP__

#include "../thirdparty/enums/enum.h"
#include "finder.hpp"

namespace SMC{

enum class ElementType:uint8_t{
    NONE, 

    ZEBRA = 4,
    CROSS,
    RING,
    BARRIER
};


class Element{};

#define CONFIG_CONSTRUCT(type)\
Config * config;\
type (Config * _config):config(_config){;}\

class Zebra : public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{

    };

    CONFIG_CONSTRUCT(Zebra)

};
class Ring : public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{

    };

    CONFIG_CONSTRUCT(Ring)
};


class Cross :public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{

    };

    CONFIG_CONSTRUCT(Cross)
};

class Barrier :public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{

    };

    CONFIG_CONSTRUCT(Barrier)\

};


};


#endif