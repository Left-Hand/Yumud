#include "enums.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

#define DEF_MAKE_BINA_ENUM(name, u, l)\
OutputStream & operator <<(OutputStream & os, const name self){\
    return os << (bool(self) ? #u : #l); } \

DEF_MAKE_BINA_ENUM(Endian, MSB, LSB)
DEF_MAKE_BINA_ENUM(Continuous, CONT, DISC)
DEF_MAKE_BINA_ENUM(Ack, ACK, NACK)
DEF_MAKE_BINA_ENUM(ClockDirection, CW, CCW)
DEF_MAKE_BINA_ENUM(BoolLevel, HIGH, LOW)
DEF_MAKE_BINA_ENUM(Enable, EN, DISEN)
DEF_MAKE_BINA_ENUM(TB, TOP, BOTTOM)
DEF_MAKE_BINA_ENUM(LR, LEFT, RIGHT)
}