#pragma once

#include "Option.hpp"


#define DEF_STRONG_TYPE_GRADATION(name, fn_name, D, T, start, stop, step)\
struct [[nodiscard]] name {\
public:\
    static constexpr T MAX_VALUE = static_cast<T>(stop);\
    static constexpr T MIN_VALUE = static_cast<T>(start);\
    static constexpr T STEP_VALUE = static_cast<T>(step);\
    static_assert(MIN_VALUE < MAX_VALUE);\
    [[nodiscard]] static constexpr Option<name> fn_name(const T in) {\
        if (in < MIN_VALUE || in > MAX_VALUE) return None;\
        auto raw = static_cast<D>((in - MIN_VALUE) / STEP_VALUE);\
        return Some(name(raw));\
    }\
    [[nodiscard]] constexpr D as_bits() const { return bits; }\
    [[nodiscard]] constexpr T to_original() const {\
        return MIN_VALUE + static_cast<T>(bits) * STEP_VALUE;\
    }\
    D bits;\
    static constexpr name from_bits(D bits) {\
        name ret;\
        ret.bits = bits;\
        return ret;\
    }\
};\

#define DEF_U8_STRONG_TYPE_GRADATION(name, fn_name, T, start, stop, step) DEF_STRONG_TYPE_GRADATION(name, fn_name, uint8_t, T, start, stop, step)
#define DEF_U16_STRONG_TYPE_GRADATION(name, fn_name, T, start, stop, step) DEF_STRONG_TYPE_GRADATION(name, fn_name, uint16_t, T, start, stop, step)
