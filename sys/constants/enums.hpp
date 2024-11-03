#pragma once

#include <type_traits>
#include <cstdint>
#include "sys/core/sys_defs.h"

#define MAKE_BINA_ENUM(name, u, l)\
struct name {\
private:\
    bool value;\
public:\
    constexpr explicit name(bool _value) : value(_value) {}\
    constexpr operator bool() const { return value; }\
};\
scexpr name u {true};\
scexpr name l{false};\

MAKE_BINA_ENUM(Endian, MSB, LSB)
MAKE_BINA_ENUM(Continuous, CONT, DISC)
MAKE_BINA_ENUM(Ack, ACK, NACK)
MAKE_BINA_ENUM(ClockDirection, CLOCKWISE,COUNTERCLOCKWISE)
// MAKE_BINA_ENUM(TB, TOP, BOTTOM)
// MAKE_BINA_ENUM(LR, LEFT, RIGHT)

enum class TB:uint8_t{TOP, BOTTOM};
enum class LR:uint8_t{LEFT, RIGHT};


namespace yumud{
class OutputStream;

OutputStream & operator<<(OutputStream & os, const TB tb);

OutputStream & operator<<(OutputStream & os, const LR lr);
};