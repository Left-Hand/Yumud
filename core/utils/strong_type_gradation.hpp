#pragma once

#include "Option.hpp"


#define DEF_U8_STRONG_TYPE_GRADATION(name, fn_name, T, start, stop, step)\
struct [[nodiscard]] name {\
public:\
    static constexpr T MAX_VALUE = static_cast<T>(stop);\
    static constexpr T MIN_VALUE = static_cast<T>(start);\
    static constexpr T STEP_VALUE = static_cast<T>(step);\
    static_assert(MIN_VALUE < MAX_VALUE);\
    [[nodiscard]] static constexpr Option<name> fn_name(const T in) {\
        if (in < MIN_VALUE || in > MAX_VALUE) return None;\
        auto raw = static_cast<uint8_t>((in - MIN_VALUE) / STEP_VALUE);\
        return Some(name(raw));\
    }\
    [[nodiscard]] constexpr uint8_t as_u8() const { return raw_code_; }\
    [[nodiscard]] constexpr T to_original() const {\
        return MIN_VALUE + static_cast<T>(raw_code_) * STEP_VALUE;\
    }\
private:\
    constexpr explicit name(uint8_t raw_code) : raw_code_(raw_code) {}\
    uint8_t raw_code_;\
};\

