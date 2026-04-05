#pragma once

#include <cstdint>

namespace ymd::intrinsics{


template<typename T>
static uint32_t load_volatile_to_u32(volatile T * p_reg){
    static_assert(sizeof(T) == 4);
    return *reinterpret_cast<volatile uint32_t *>(p_reg);
}

template<typename T>
static void store_volatile_with_u32(volatile T * p_reg, const uint32_t x){
    static_assert(sizeof(T) == 4);
    *reinterpret_cast<volatile uint32_t *>(p_reg) = x;
}

template<typename T>
static T load_volatile32(volatile T * p_reg){
    static_assert(sizeof(T) == 4);
    const uint32_t bits = *reinterpret_cast<volatile uint32_t *>(p_reg);
    return std::bit_cast<T>(bits);
}

template<typename T, typename Fn>
static inline void modify_reg32(volatile T* p_reg, Fn&& fn) {
    static_assert(sizeof(T) == 4);
    static_assert(std::is_trivially_copyable_v<T>, 
        "T must be trivially copyable for register operations");
    
    // 读取并转换
    T tempreg = std::bit_cast<T>((*reinterpret_cast<const volatile uint32_t*>(p_reg)));
    
    // 应用修改
    tempreg = fn(tempreg);
    
    // 写回
    *reinterpret_cast<volatile uint32_t *>(p_reg) = std::bit_cast<uint32_t>(tempreg);
}


}