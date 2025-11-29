#pragma once

#include "core/math/realmath.hpp"

namespace ymd{

template<typename T>
struct [[nodiscard]] dB{
    static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");

    constexpr explicit dB(T val):
        val_(val){}

    [[nodiscard]] static constexpr dB from_linear(T val){
        return dB(10*log10(val));
    }

    [[nodiscard]] static consteval dB one(){
        return dB(0);
    }

    [[nodiscard]] constexpr T to_linear() const{
        return pow(10, val_/10);
    }

    [[nodiscard]] constexpr dB operator -() const {
        return dB(-val_);
    }

    [[nodiscard]] constexpr dB operator +() const {
        return dB(val_);
    }

    [[nodiscard]] constexpr dB operator +(const dB & rhs) const {
        return dB(val_ + rhs.val_);
    }

    [[nodiscard]] constexpr dB operator -(const dB & rhs) const {
        return dB(val_ + rhs.val_);
    }

    [[nodiscard]] constexpr dB scale(const T rhs) const {
        return dB(val_ * rhs);
    }

    [[nodiscard]] constexpr T operator *(const T rhs) const {
        return to_linear() * rhs;
    }
private:
    T val_;
};
}