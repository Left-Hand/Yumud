#pragma once

#include "core/math/uint24_t.hpp"
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
    __RegC_t(const std::span<const std::byte> pdata){
        *(reinterpret_cast<T *>(this)) = *(reinterpret_cast<const T *>(pdata.data()));
    };
    using value_type = T;
    __RegC_t<T> copy() const{return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    
    T operator &(const T data) const {return T(*this) & data;}
    T operator |(const T data) const {return T(*this) | data;}
    constexpr const T * operator &() const {return (reinterpret_cast<const T *>(this));}
    
    constexpr std::span<const std::byte> as_bytes() const {
        return std::span<const std::byte>(reinterpret_cast<const uint8_t *>(this), sizeof(TReg));}
    
    constexpr const T & as_ref() const {return (reinterpret_cast<const T &>(this));}
    constexpr T as_val() const {return (reinterpret_cast<const T &>(*this));}
    constexpr std::span<const T, 1> as_span() const {return std::span(reinterpret_cast<const T *>(*this), 1);}
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
    using __RegC_t<T>::as_bytes;
    using TReg = __RegC_t<T>::TReg;
    
    __Reg_t<T> copy() const{return *this;}
    constexpr __Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
    constexpr T * operator &() {return (reinterpret_cast<T *>(this));}
    constexpr std::span<std::byte> as_bytes() {
        return std::span<std::byte>(reinterpret_cast<std::byte *>(this), sizeof(TReg));}
    constexpr std::span<const std::byte> as_bytes() const {
        return std::span<const std::byte>(reinterpret_cast<const std::byte *>(this), sizeof(TReg));}
    
    constexpr __Reg_t<T> & set_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(*this) | static_cast<T>(data); return *this;}
    constexpr __Reg_t<T> & clr_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(*this) & ~static_cast<T>(data); return *this;}
    constexpr __Reg_t<T> & reconf_bits(const T data) {
        static_cast<T &>(*this) = static_cast<T>(data); return *this;}
    
    constexpr T & as_ref() {return (reinterpret_cast<T &>(*this));}
    constexpr const T & as_ref() const {return (reinterpret_cast<const T &>(this));}
    constexpr T as_val() const {return (reinterpret_cast<const T &>(*this));}
    constexpr std::span<T, 1> as_span() {return std::span(reinterpret_cast<T *>(*this), 1);}
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

#define DEF_R16(name)\
name{};\
static_assert(sizeof(std::decay_t<decltype(name)>) == 2, "x must be 16bit register");\
static_assert(std::has_unique_object_representations_v<std::decay_t<decltype(name)>>, "x must has unique bitfield");\

#define DEF_R8(name)\
name{};\
static_assert(sizeof(std::decay_t<decltype(name)>) == 1, "x must be 8bit register");\
static_assert(std::has_unique_object_representations_v<std::decay_t<decltype(name)>>, "x must has unique bitfield");\

#define REG16I_QUICK_DEF(addr, type, name)\
struct type :public Reg16i<>{scexpr uint8_t address = addr; int16_t data;} DEF_R16(name)

#define REG16_QUICK_DEF(addr, type, name)\
struct type :public Reg16<>{scexpr RegAddress address = addr; uint16_t data;} DEF_R16(name)

#define REG8_QUICK_DEF(addr, type, name)\
struct type :public Reg8<>{scexpr RegAddress address = addr; uint8_t data;} DEF_R8(name)

}
