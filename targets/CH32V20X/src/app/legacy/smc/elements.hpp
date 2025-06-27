#pragma once
#include "../thirdparty/enums/enum.h"
#include "finder.hpp"

namespace SMC{

enum class ElementType:uint8_t{
    NONE, 
    STRAIGHT,
    WAVE,
    CURVE,
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
        BEG,
        END
    };

    CONFIG_CONSTRUCT(Zebra)

};
class Ring : public Element{
public:
    struct Config{
    };

    enum class Status:uint8_t{
        BEG,
        IN,
        RUNNING,
        OUT,
        END
    };

    CONFIG_CONSTRUCT(Ring)
};


class Cross :public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{
        BEG,
        END
    };

    CONFIG_CONSTRUCT(Cross)
};

class Barrier :public Element{
public:
    struct Config{

    };

    enum class Status:uint8_t{
        BEG,
        END
    };

    CONFIG_CONSTRUCT(Barrier)

};


};

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::ElementType type){
    using namespace SMC;

    #define ELEMENT_TYPE_OUT(x) case ElementType::x: return os << #x;

    switch(type){
        ELEMENT_TYPE_OUT(NONE)
        ELEMENT_TYPE_OUT(STRAIGHT)
        ELEMENT_TYPE_OUT(CURVE)
        ELEMENT_TYPE_OUT(ZEBRA)
        ELEMENT_TYPE_OUT(CROSS)
        ELEMENT_TYPE_OUT(RING)
        ELEMENT_TYPE_OUT(BARRIER)
        default: return os << '?';

    };

    #undef ELEMENT_TYPE_OUT
}

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::Zebra::Status type){
    using namespace SMC;

    #define ELEMENT_STAT_OUT(x) case Zebra::Status::x: return os << #x;

    switch(type){
        ELEMENT_STAT_OUT(BEG)
        ELEMENT_STAT_OUT(END)

        default: return os << '?';

    };

    #undef ELEMENT_STAT_OUT
}

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::Ring::Status type){
    using namespace SMC;

    #define ELEMENT_STAT_OUT(x) case Ring::Status::x: return os << #x;

    switch(type){
        ELEMENT_STAT_OUT(BEG)
        ELEMENT_STAT_OUT(IN)
        ELEMENT_STAT_OUT(OUT)
        ELEMENT_STAT_OUT(RUNNING)
        ELEMENT_STAT_OUT(END)

        default: return os << '?';

    };

    #undef ELEMENT_STAT_OUT
}


__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::Cross::Status type){
    using namespace SMC;

    #define ELEMENT_STAT_OUT(x) case Cross::Status::x: return os << #x;

    switch(type){
        ELEMENT_STAT_OUT(BEG)
        ELEMENT_STAT_OUT(END)

        default: return os << '?';

    };

    #undef ELEMENT_STAT_OUT
}

__fast_inline OutputStream & operator<<(OutputStream & os, const SMC::Barrier::Status type){
    using namespace SMC;

    #define ELEMENT_STAT_OUT(x) case Barrier::Status::x: return os << #x;

    switch(type){
        ELEMENT_STAT_OUT(BEG)
        ELEMENT_STAT_OUT(END)

        default: return os << '?';

    };

    #undef ELEMENT_STAT_OUT
}