#pragma once

#include <type_traits>
#include <cstdint>
#include "core/sys_defs.hpp"

#define MAKE_BINA_ENUM(name, u, l)\
struct name {\
private:\
    bool value;\
    constexpr name(){;}\
public:\
    constexpr name(const name & other){value = other.value;};\
    constexpr name(const bool & _value) = delete;\
    constexpr name(bool && _value) = delete;\
    constexpr name operator = (const bool _value) = delete;\
    scexpr name from(bool _value) { name ret = name(); ret.value = _value; return ret; }\
    explicit operator bool() const { return value; }\
    bool operator == (const name & other) const { return value == other.value; }\
    bool operator != (const name & other) const { return value == other.value; }\
};\
scexpr name u = name::from(true);\
scexpr name l = name::from(false);\

MAKE_BINA_ENUM(Endian, MSB, LSB)
MAKE_BINA_ENUM(Continuous, CONT, DISC)
MAKE_BINA_ENUM(Ack, ACK, NACK)
MAKE_BINA_ENUM(ClockDirection, CLOCKWISE,COUNTERCLOCKWISE)
MAKE_BINA_ENUM(BoolLevel, HIGH, LOW)
MAKE_BINA_ENUM(Enable, EN, DISEN)
// MAKE_BINA_ENUM(TB, TOP, BOTTOM)
// MAKE_BINA_ENUM(LR, LEFT, RIGHT)

enum class TB:uint8_t{TOP, BOTTOM};
enum class LR:uint8_t{LEFT, RIGHT};


namespace ymd{
class OutputStream;

OutputStream & operator<<(OutputStream & os, const TB tb);

OutputStream & operator<<(OutputStream & os, const LR lr);
};