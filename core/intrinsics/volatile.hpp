#pragma once

#include <cstdint>

namespace ymd::intrinsics{


template<typename T>
static T load_volatile(volatile T * p_reg){
    return *const_cast<T *>(p_reg);
}

template<typename T>
static void store_volatile(volatile T * p_reg, const T x){
    *const_cast<T *>(p_reg) = x;
}

template<typename T, typename Fn>
static inline void modify_reg(volatile T* reg, Fn&& fn) {
    static_assert(std::is_trivially_copyable_v<T>, 
        "T must be trivially copyable for register operations");
    
    // 读取并转换
    T temp = (*const_cast<const T*>(reg));
    
    // 应用修改
    temp = fn(temp);
    
    // 写回
    *const_cast<T*>(reg) = temp;
}


}