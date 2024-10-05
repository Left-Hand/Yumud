#pragma once

#include <type_traits>

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