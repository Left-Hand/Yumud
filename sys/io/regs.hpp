#pragma once

#include "sys/math/uint24_t.h"
#include <span>

namespace ymd{


struct __RegBase{};

template<typename T, typename D = T>
struct alignas(sizeof(T)) __RegC_t:public __RegBase{
protected:
    static constexpr bool is_crtp = std::is_same_v<T, D>;
    static constexpr bool is_non_crtp = !is_crtp;

    static_assert(is_non_crtp or std::has_unique_object_representations_v<D>, "data must have unique object representation");
    static_assert(is_non_crtp or (sizeof(T) == sizeof(D)), "data must keep the same size");
    using TReg = T;
    __RegC_t() = default;
public:
    __RegC_t(const std::span<const uint8_t> pdata){
        *(reinterpret_cast<T *>(this)) = *(reinterpret_cast<const T *>(pdata.data()));
    };
    using value_type = T;
    __RegC_t<T> copy() const{return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    
    T operator &(const T data) const {return T(*this) & data;}
    T operator |(const T data) const {return T(*this) | data;}
    constexpr const T * operator &() const {return (reinterpret_cast<const T *>(this));}
    [[deprecated("use to_bytes instead")]] constexpr operator std::span<const uint8_t>() const {return {reinterpret_cast<const uint8_t *>(this), sizeof(T)};}
    [[deprecated("use to_bytes instead")]] constexpr uint8_t operator [](const size_t idx) const {return (*(reinterpret_cast<const uint8_t *>(this) + idx));} 
    
    constexpr std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(this), sizeof(TReg));}
    
    constexpr const T & as_ref() const {return (reinterpret_cast<const T &>(this));}
    constexpr T as_val() const {return (reinterpret_cast<const T &>(*this));}
};


template<typename T, typename D = T>
struct __Reg_t:public __RegC_t<T, D>{
protected:
    __Reg_t<T> & operator = (const __Reg_t<T, D> & other) = default;
    __Reg_t<T> & operator = (__Reg_t<T, D> && other) = default;
    __Reg_t(const T & data){*this = data;};
    __Reg_t(T && data){*this = data;};
    
public:
    using value_type = T;
    
    using __RegC_t<T>::__RegC_t;
    using __RegC_t<T>::operator [];
    using __RegC_t<T>::as_bytes;
    using TReg = __RegC_t<T>::TReg;
    
    __Reg_t<T> copy() const{return *this;}
    constexpr __Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
    constexpr T * operator &() {return (reinterpret_cast<T *>(this));}
    [[deprecated("use to_bytes instead")]] constexpr uint8_t & operator [](const size_t idx){return (*(reinterpret_cast<uint8_t *>(this) + idx));} 
    [[deprecated("use clr_bits instead")]] constexpr __Reg_t<T> & operator &=(const T data) {static_cast<T &>(*this) = static_cast<T>(*this) & static_cast<T>(data); return *this;}
    [[deprecated("use set_bits instead")]] constexpr __Reg_t<T> & operator |=(const T data) {static_cast<T &>(*this) = static_cast<T>(*this) | static_cast<T>(data); return *this;}
    
    constexpr std::span<uint8_t> as_bytes() {
        return std::span<uint8_t>(reinterpret_cast<uint8_t *>(this), sizeof(TReg));}
    constexpr std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(this), sizeof(TReg));}
    
    constexpr __Reg_t<T> & set_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(*this) | static_cast<T>(data); return *this;}
    constexpr __Reg_t<T> & clr_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(*this) & ~static_cast<T>(data); return *this;}
    constexpr __Reg_t<T> & reconf_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(data); return *this;}
    
    constexpr T & as_ref() {return (reinterpret_cast<T &>(*this));}
    constexpr const T & as_ref() const {return (reinterpret_cast<const T &>(this));}
    constexpr T as_val() const {return (reinterpret_cast<const T &>(*this));}
};

#define REG_TEMPLATE(name, T)\
template<typename D = T>\
struct name:public __Reg_t<T, D>{\
using __Reg_t<T, D>::__Reg_t;\
using __Reg_t<T, D>::operator T;\
using __Reg_t<T, D>::operator T &;\
using __Reg_t<T, D>::operator =;\
};\

#define REGC_TEMPLATE(name, T)\
template<typename D = T>\
struct name:public __RegC_t<T, D>{\
using __RegC_t<T, D>::__RegC_t;\
using __RegC_t<T, D>::operator T;\
using __RegC_t<T, D>::operator =;\
};\


REG_TEMPLATE(Reg8, uint8_t)
REG_TEMPLATE(Reg16, uint16_t)
// REG_TEMPLATE(Reg24, uint24_t)
REG_TEMPLATE(Reg32, uint32_t)
REG_TEMPLATE(Reg64, uint64_t)

REG_TEMPLATE(Reg8i, int8_t)
REG_TEMPLATE(Reg16i, int16_t)
REG_TEMPLATE(Reg32i, int32_t)
REG_TEMPLATE(Reg64i, int64_t)

REGC_TEMPLATE(RegC8, uint8_t)
REGC_TEMPLATE(RegC16, uint16_t)
// REGC_TEMPLATE(RegC24, uint24_t)
REGC_TEMPLATE(RegC32, uint32_t)
REGC_TEMPLATE(RegC64, uint64_t)

REGC_TEMPLATE(RegC8i, int8_t)
REGC_TEMPLATE(RegC16i, int16_t)
REGC_TEMPLATE(RegC32i, int32_t)
REGC_TEMPLATE(RegC64i, int64_t)

#undef REG_TEMPLATE
#undef REGC_TEMPLATE
}
