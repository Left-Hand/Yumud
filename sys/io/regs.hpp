#pragma once

#include "sys/math/uint24_t.h"
#include "sys/math/real.hpp"

template<typename T>
struct Reg_t{
protected:
    Reg_t<T> & operator = (const Reg_t<T> & other) = default;
    Reg_t<T> & operator = (Reg_t<T> && other) = default;
public:
    Reg_t<T> copy() const{return *this;}
    constexpr Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
    constexpr const T * operator &() const {return (reinterpret_cast<const T *>(this));}
    constexpr T * operator &() {return (reinterpret_cast<T *>(this));}

    template<typename U = T>
    requires std::signed_integral<U>
    constexpr real_t uni() const { return s16_to_uni(*this); }

    template<typename U = T>
    requires std::unsigned_integral<U>
    constexpr real_t uni() const { return u16_to_uni(*this); }


    constexpr uint8_t & operator [](const size_t idx){return (*(reinterpret_cast<uint8_t *>(this) + idx));} 
    constexpr const uint8_t & operator [](const size_t idx) const {return (*(reinterpret_cast<const uint8_t *>(this) + idx));} 
};

#define REG_TEMPLATE(name, T)\
struct name:public Reg_t<T>{\
    using Reg_t<T>::operator T;\
    using Reg_t<T>::operator T &;\
    using Reg_t<T>::operator =;\
};\

REG_TEMPLATE(Reg8, uint8_t)
REG_TEMPLATE(Reg16, uint16_t)
REG_TEMPLATE(Reg24, uint24_t)
REG_TEMPLATE(Reg32, uint32_t)
REG_TEMPLATE(Reg64, uint64_t)

REG_TEMPLATE(Reg8i, int8_t)
REG_TEMPLATE(Reg16i, int16_t)
REG_TEMPLATE(Reg32i, int32_t)
REG_TEMPLATE(Reg64i, int64_t)

#undef REG_TEMPLATE