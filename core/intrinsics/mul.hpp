#pragma once

#include <cstdint>

namespace ymd::intrinsics{





__attribute__((always_inline, const, optimize( "-Ofast" )))
static constexpr uint32_t mul32(uint32_t a, uint32_t b) {
    #define DEF_GENERAL_CALC static_cast<uint32_t>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b));

    if (std::is_constant_evaluated()){
        return DEF_GENERAL_CALC
    }else{

        #if defined(__riscv) && defined(__riscv_mul)

        uint32_t result;
        __asm__ (
            "mul %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;

        #endif

        return DEF_GENERAL_CALC
    }
    #undef DEF_GENERAL_CALC
}


__attribute__((always_inline, const, optimize( "-Ofast" )))
static constexpr int32_t mul32hss(int32_t a, int32_t b) {
    #define DEF_GENERAL_CALC static_cast<int32_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b) >> 32);

    if (std::is_constant_evaluated()){
        return DEF_GENERAL_CALC
    }else{

        #if defined(__riscv) && defined(__riscv_mul)

        int32_t result;
        __asm__ (
            "mulh %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;

        #endif

        return DEF_GENERAL_CALC
    }
    #undef DEF_GENERAL_CALC
}

__attribute__((always_inline, const, optimize( "-Ofast" )))
static constexpr uint32_t mul32hu(uint32_t a, uint32_t b) {
    #define DEF_GENERAL_CALC static_cast<uint32_t>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b) >> 32);

    if (std::is_constant_evaluated()){
        return DEF_GENERAL_CALC
    }else{

        #if defined(__riscv) && defined(__riscv_mul)

        uint32_t result;
        __asm__ (
            "mulhu %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;

        #endif

        return DEF_GENERAL_CALC
    }
    #undef DEF_GENERAL_CALC
}

__attribute__((always_inline, const, optimize( "-Ofast" )))
static constexpr int32_t mul32hsu(int32_t a, uint32_t b) {
    #define DEF_GENERAL_CALC static_cast<int32_t>(static_cast<int64_t>(a) * static_cast<uint64_t>(b) >> 32);

    if (std::is_constant_evaluated()){
        return DEF_GENERAL_CALC
    }else{

        #if defined(__riscv) && defined(__riscv_mul)

        int32_t result;
        __asm__ (
            "mulhsu %0, %1, %2"
            : "=r"(result)
            : "r"(a), "r"(b)
        );
        return result;

        #endif

        return DEF_GENERAL_CALC
    }
    #undef DEF_GENERAL_CALC
}

__attribute__((always_inline, const, optimize( "-Ofast" )))
static constexpr int32_t mul32hus(uint32_t a, int32_t b) {
    #define DEF_GENERAL_CALC static_cast<int32_t>(static_cast<uint64_t>(a) * static_cast<int64_t>(b) >> 32);

    if (std::is_constant_evaluated()){
        return DEF_GENERAL_CALC
    }else{

        #if defined(__riscv) && defined(__riscv_mul)

        int32_t result;
        __asm__ (
            "mulhsu %0, %1, %2"
            : "=r"(result)
            : "r"(b), "r"(a)
        );
        return result;

        #endif

        return DEF_GENERAL_CALC
    }
    #undef DEF_GENERAL_CALC
}




}