#include "enums.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

#define DEF_DISPLAY_BINA_ENUM(name, u, l)\
OutputStream & operator <<(OutputStream & os, const name self){\
    return os << ((self == name::u) ? #u : #l);\
} 


DEF_DISPLAY_BINA_ENUM(BitOrder, MSB, LSB)
DEF_DISPLAY_BINA_ENUM(Continuous, CONT, DISC)
DEF_DISPLAY_BINA_ENUM(Ack, ACK, NACK)
DEF_DISPLAY_BINA_ENUM(RotateDirection, CW, CCW)
DEF_DISPLAY_BINA_ENUM(BoolLevel, HIGH, LOW)
DEF_DISPLAY_BINA_ENUM(Enable, EN, DISEN)
DEF_DISPLAY_BINA_ENUM(TB, TOP, BOTTOM)
DEF_DISPLAY_BINA_ENUM(LR, LEFT, RIGHT)

#undef DEF_DISPLAY_BINA_ENUM
}