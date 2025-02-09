#pragma once

#include "sys/math/uint24_t.h"
#include <span>

namespace ymd{

struct __RegBase{};

template<typename T>
struct RegC_t:public __RegBase{
protected:
    RegC_t() = default;
public:
    RegC_t(const std::span<const uint8_t> pdata){
        *(reinterpret_cast<T *>(this)) = *(reinterpret_cast<const T *>(pdata.data()));
    };
    using value_type = T;
    RegC_t<T> copy() const{return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    constexpr const T * operator &() const {return (reinterpret_cast<const T *>(this));}
    constexpr const uint8_t & operator [](const size_t idx) const {return (*(reinterpret_cast<const uint8_t *>(this) + idx));} 

    constexpr operator std::span<const uint8_t>() const {return {reinterpret_cast<const uint8_t *>(this), sizeof(T)};}
    auto span() const {return std::span<const uint8_t>(*this);}
};


template<typename T>
struct Reg_t:public RegC_t<T>{
protected:
    Reg_t<T> & operator = (const Reg_t<T> & other) = default;
    Reg_t<T> & operator = (Reg_t<T> && other) = default;
    Reg_t(const T & data){*this = data;};
    Reg_t(T && data){*this = data;};

public:
    using value_type = T;
    
    using RegC_t<T>::RegC_t;

    Reg_t<T> copy() const{return *this;}
    constexpr Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
    constexpr T * operator &() {return (reinterpret_cast<T *>(this));}
    constexpr uint8_t & operator [](const size_t idx){return (*(reinterpret_cast<uint8_t *>(this) + idx));} 
    constexpr operator std::span<uint8_t>() {return {reinterpret_cast<uint8_t *>(this), sizeof(T)};}
    auto span() {return std::span<uint8_t>(*this);}
};


#define REG_TEMPLATE(name, T)\
struct name:public Reg_t<T>{\
    using Reg_t<T>::Reg_t;\
    using Reg_t<T>::operator T;\
    using Reg_t<T>::operator T &;\
    using Reg_t<T>::operator =;\
};\

#define REGC_TEMPLATE(name, T)\
struct name:public RegC_t<T>{\
    using RegC_t<T>::RegC_t;\
    using RegC_t<T>::operator T;\
    using RegC_t<T>::operator =;\
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

REGC_TEMPLATE(RegC8, uint8_t)
REGC_TEMPLATE(RegC16, uint16_t)
REGC_TEMPLATE(RegC24, uint24_t)
REGC_TEMPLATE(RegC32, uint32_t)
REGC_TEMPLATE(RegC64, uint64_t)

REGC_TEMPLATE(RegC8i, int8_t)
REGC_TEMPLATE(RegC16i, int16_t)
REGC_TEMPLATE(RegC32i, int32_t)
REGC_TEMPLATE(RegC64i, int64_t)

#undef REG_TEMPLATE
#undef REGC_TEMPLATE
}
