#pragma once

template<typename T>
struct Reg_t{
    Reg_t<T> & operator = (const Reg_t<T> & other) = delete;
    Reg_t<T> & operator = (Reg_t<T> && other) = delete;

    constexpr Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
};

#define REG_TEMPLATE(name, T)\
struct name:public Reg_t<T>{\
    using Reg_t<T>::operator T;\
    using Reg_t<T>::operator T &;\
    using Reg_t<T>::operator =;\
};\

REG_TEMPLATE(Reg8, uint8_t)
REG_TEMPLATE(Reg16, uint16_t)
REG_TEMPLATE(Reg32, uint32_t)
REG_TEMPLATE(Reg64, uint64_t)

REG_TEMPLATE(Reg8i, int8_t)
REG_TEMPLATE(Reg16i, int16_t)
REG_TEMPLATE(Reg32i, int32_t)
REG_TEMPLATE(Reg64i, int64_t)

#undef REG_TEMPLATE