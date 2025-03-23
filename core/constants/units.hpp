#pragma once

#include <cstdint>

__inline constexpr uint64_t operator"" _KHz(uint64_t x){
    return x * 1000;
}

__inline constexpr uint64_t operator"" _MHz(uint64_t x){
    return x * 1000000;
}

__inline constexpr uint64_t operator"" _GHz(uint64_t x){
    return x * 1000000000;
}

__inline constexpr uint64_t operator"" _KB(uint64_t x){
    return x << 10;
}

__inline constexpr uint64_t operator"" _MB(uint64_t x){
    return x << 20;
}

__inline constexpr uint64_t operator"" _GB(uint64_t x){
    return x << 30;
}