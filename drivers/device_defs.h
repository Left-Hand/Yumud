#pragma once

#include <bit>
#include <array>
#include <functional>
#include <memory>
#include <type_traits>
#include <optional>

#include "sys/core/system.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/bus/uart/uart.hpp"

#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/adc/adc_channel.hpp"


template<typename T>
struct Reg_t{
    Reg_t<T> & operator = (const Reg_t<T> & other) = delete;
    Reg_t<T> & operator = (Reg_t<T> && other) = delete;

    constexpr Reg_t<T> & operator =(const T data){*reinterpret_cast<T *>(this) = data;return *this;}
    constexpr operator T() const {return (*reinterpret_cast<const T *>(this));}
    constexpr operator T & () {return (*reinterpret_cast<T *>(this));}
};

template<typename T>
concept is_reg = std::same_as<T, Reg_t<std::decay_t<T>>>;


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


struct Fraction {
public:
    int numerator;
    int denominator;
    // Fraction(const int _numerator,const int _denominator) : numerator(_numerator), denominator(_denominator) {}

    int operator * (const int & value){
        return numerator * value / denominator;
    }

    Fraction operator * (const Fraction & value){
        return Fraction(numerator * value.numerator, denominator * value.denominator);
    }
};


#define TODO(str) do{PANIC("todo", str)}while(false);