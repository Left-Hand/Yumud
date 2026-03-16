#pragma once

#include <cstdint>

namespace ymd::intrinsics{


#if defined(__riscv)


__attribute__((always_inline))
static constexpr uint32_t mul32(uint32_t a, uint32_t b) {
    if (std::is_constant_evaluated()){
        return static_cast<uint32_t>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));
    }else{
        uint32_t result;
        __asm__ (
            "mul %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;
    }
}


__attribute__((always_inline))
static constexpr int32_t mul32hss(int32_t a, int32_t b) {
    if (std::is_constant_evaluated()){
        return static_cast<int32_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b) >> 32);
    }else{
        int32_t result;
        __asm__ (
            "mulh %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;
    }
}

__attribute__((always_inline))
static constexpr uint32_t mul32hu(uint32_t a, uint32_t b) {
    if (std::is_constant_evaluated()){
        return static_cast<uint32_t>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b) >> 32);
    }else{
        uint32_t result;
        __asm__ (
            "mulhu %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;
    }
}

__attribute__((always_inline))
static constexpr int32_t mul32hsu(int32_t a, uint32_t b) {
    if (std::is_constant_evaluated()){
        return static_cast<int32_t>(static_cast<int64_t>(a) * static_cast<uint64_t>(b) >> 32);
    }else{
        int32_t result;
        __asm__ (
            "mulhsu %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;
    }
}

__attribute__((always_inline))
static constexpr int32_t mul32hus(uint32_t a, int32_t b) {
    if (std::is_constant_evaluated()){
        return static_cast<int32_t>(static_cast<uint64_t>(a) * static_cast<int64_t>(b) >> 32);
    }else{
        int32_t result;
        __asm__ (
            "mulhsu %0, %1, %2"
            : "=r"(result)
            : "r"(b), "r"(a)
        );
        return result;
    }
}

[[nodiscard]] __attribute__((__always_inline__)) constexpr 
uint32_t __mpyf_ul(uint32_t arg1, uint32_t arg2){
    return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 31);
}




[[nodiscard]] __attribute__((__always_inline__)) constexpr 
int32_t __mpyf_ul_reuse_arg1(uint32_t arg1, uint32_t arg2){
    return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 31);
}


#elif defined(__arm__)
#error "arm intrinsics not implemented"
#elif defined(__aarch64__)
#error "aarch64 intrinsics not implemented"
#endif
}