#pragma once

#include <cstdint>


namespace ymd::iqmath::details{
[[nodiscard]] __always_inline constexpr int16_t __mpy_w(int16_t arg1, int16_t arg2){
    return (arg1 * arg2);
}

[[nodiscard]] __always_inline constexpr uint16_t __mpy_uw(uint16_t arg1, uint16_t arg2){
    return (arg1 * arg2);
}

[[nodiscard]] __always_inline constexpr int32_t __mpyx_w(int16_t arg1, int16_t arg2){
    return (int32_t(arg1) * int32_t(arg2));
}

[[nodiscard]] __always_inline constexpr uint32_t __mpyx_uw(uint16_t arg1, uint16_t arg2){
    return (uint32_t(arg1) * uint32_t(arg2));
}

[[nodiscard]] __always_inline constexpr int16_t __mpyf_w(int16_t arg1, int16_t arg2){
    return int16_t((int32_t(arg1) * int32_t(arg2)) >> 15);
}

[[nodiscard]] __always_inline constexpr int16_t __mpyf_w_reuse_arg1(int16_t arg1, int16_t arg2){
    /* This is identical to __mpyf_w */
    return int16_t((int32_t(arg1) * int32_t(arg2)) >> 15);
}

[[nodiscard]] __always_inline constexpr uint16_t __mpyf_uw(uint16_t arg1, uint16_t arg2){
    return uint16_t((uint32_t(arg1) * uint32_t(arg2)) >> 15);
}

[[nodiscard]] __always_inline constexpr uint16_t __mpyf_uw_reuse_arg1(uint16_t arg1, uint16_t arg2){
    /* This is identical to __mpyf_uw */
    return uint16_t((uint32_t(arg1) * uint32_t(arg2)) >> 15);
}

[[nodiscard]] __always_inline constexpr int32_t __mpyfx_w(int16_t arg1, int16_t arg2){
    return ((int32_t(arg1) * int32_t(arg2)) << 1);
}

[[nodiscard]] __always_inline constexpr int32_t __mpyfx_uw(uint16_t arg1, uint16_t arg2){
    return ((uint32_t(arg1) * uint32_t(arg2)) << 1);
}


[[nodiscard]] __always_inline constexpr int32_t __mpy_l(int32_t arg1, int32_t arg2){
    return (arg1 * arg2);
}

[[nodiscard]] __always_inline constexpr uint32_t __mpy_ul(uint32_t arg1, uint32_t arg2){
    return (arg1 * arg2);
}

[[nodiscard]] __always_inline constexpr int64_t __mpyx(int32_t arg1, int32_t arg2){
    return (int64_t(arg1) * int64_t(arg2));
}

[[nodiscard]] __always_inline constexpr uint64_t __mpyx_u(uint32_t arg1, uint32_t arg2){
    return (uint64_t(arg1) * uint64_t(arg2));
}

[[nodiscard]] __always_inline constexpr int32_t __mpyf_l(int32_t arg1, int32_t arg2){
    return int32_t((int64_t(arg1) * int64_t(arg2)) >> 31);
}


[[nodiscard]] __always_inline constexpr int32_t __mpyf_l_reuse_arg1(int32_t arg1, int32_t arg2){
    /* This is identical to __mpyf_l */
    return int32_t((int64_t(arg1) * int64_t(arg2)) >> 31);
}

[[nodiscard]] __always_inline constexpr uint32_t __mpyf_ul(uint32_t arg1, uint32_t arg2){
    return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 31);
}

[[nodiscard]] __always_inline constexpr uint32_t __mpyf_ul32(uint32_t arg1, uint32_t arg2){
    return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 32);
}

[[nodiscard]] __always_inline constexpr int32_t __mpyf_ul_reuse_arg1(uint32_t arg1, uint32_t arg2){
    return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 31);
}

[[nodiscard]] __always_inline constexpr int64_t __mpyfx(int32_t arg1, int32_t arg2){
    return ((int64_t(arg1) * int64_t(arg2)) << 1);
}

[[nodiscard]] __always_inline constexpr uint64_t __mpyfx_u(uint32_t arg1, uint32_t arg2){
    return ((uint64_t(arg1) * uint64_t(arg2)) << 1);
}

}