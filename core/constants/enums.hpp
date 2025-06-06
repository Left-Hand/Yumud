#pragma once

#include <type_traits>
#include <cstdint>

#include "core/sys_defs.hpp"
// #include "core/stream/ostream.hpp"
namespace ymd{

class OutputStream;

#define DEF_MAKE_BINA_ENUM(name, u, l)\
struct name {\
public:\
    enum class Kind:uint8_t{l, u};\
    using enum Kind;\
    [[nodiscard]] constexpr name(const name & other){kind_ = other.kind_;};\
    [[nodiscard]] constexpr name(name && other){kind_ = other.kind_;};\
    constexpr name & operator = (const name & other)\
        {kind_ = other.kind_; return *this;};\
    constexpr name & operator = (name && other)\
        {kind_ = other.kind_; return *this;};\
    [[nodiscard]] constexpr name(const Kind kind){kind_ = kind;};\
    [[nodiscard]] constexpr name(const bool & kind) = delete;\
    [[nodiscard]] constexpr name(bool && kind) = delete;\
    [[nodiscard]] static constexpr name from(const bool b){return b ? name(u) : name(l);}\
    [[nodiscard]] constexpr name operator ~() const{return name(((kind_ == u ? l : u)));}\
    [[nodiscard]] constexpr name operator !() const{return name(((kind_ == u ? l : u)));}\
    [[nodiscard]] constexpr name operator = (const bool kind) = delete;\
    [[nodiscard]] constexpr bool operator == (const name & other) const { return kind_ == other.kind_; }\
    [[nodiscard]] constexpr bool operator != (const name & other) const { return kind_ == other.kind_; }\
    [[nodiscard]] constexpr bool to_bool() const { return kind_ == u; }\
private:\
    Kind kind_;\
    constexpr name(){;}\
};\
using enum name::Kind;\
OutputStream & operator <<(OutputStream & os, const name self);\

DEF_MAKE_BINA_ENUM(Endian, MSB, LSB)
DEF_MAKE_BINA_ENUM(Continuous, CONT, DISC)
DEF_MAKE_BINA_ENUM(Ack, ACK, NACK)
DEF_MAKE_BINA_ENUM(ClockDirection, CW, CCW)
DEF_MAKE_BINA_ENUM(BoolLevel, HIGH, LOW)
DEF_MAKE_BINA_ENUM(Enable, EN, DISEN)
DEF_MAKE_BINA_ENUM(TB, TOP, BOTTOM)
DEF_MAKE_BINA_ENUM(LR, LEFT, RIGHT)

#undef DEF_MAKE_BINA_ENUM
;
};