#pragma once

#include <type_traits>
#include <cstdint>

#include "core/sys_defs.hpp"
// #include "core/stream/ostream.hpp"
namespace ymd{

class OutputStream;

#define DEF_MAKE_BINA_ENUM(name, u, l)\
struct name {\
private:\
    bool value;\
    constexpr name(){;}\
public:\
    constexpr name(const name & other){value = other.value;};\
    constexpr name(const bool & _value) = delete;\
    constexpr name(bool && _value) = delete;\
    constexpr name operator = (const bool _value) = delete;\
    static constexpr name from(bool _value) { name ret = name(); ret.value = _value; return ret; }\
    explicit operator bool() const { return value; }\
    bool operator == (const name & other) const { return value == other.value; }\
    bool operator != (const name & other) const { return value == other.value; }\
};\
OutputStream & operator <<(OutputStream & os, const name self);\
static constexpr name u = name::from(true);\
static constexpr name l = name::from(false);\


DEF_MAKE_BINA_ENUM(Endian, MSB, LSB)
DEF_MAKE_BINA_ENUM(Continuous, CONT, DISC)
DEF_MAKE_BINA_ENUM(Ack, ACK, NACK)
DEF_MAKE_BINA_ENUM(ClockDirection, CLOCKWISE,COUNTERCLOCKWISE)
DEF_MAKE_BINA_ENUM(BoolLevel, HIGH, LOW)
DEF_MAKE_BINA_ENUM(Enable, EN, DISEN)
// DEF_MAKE_BINA_ENUM(TB, TOP, BOTTOM)
// DEF_MAKE_BINA_ENUM(LR, LEFT, RIGHT)

#undef DEF_MAKE_BINA_ENUM
enum class TB:uint8_t{TOP, BOTTOM};
enum class LR:uint8_t{LEFT, RIGHT};



OutputStream & operator<<(OutputStream & os, const TB tb);

OutputStream & operator<<(OutputStream & os, const LR lr);
};