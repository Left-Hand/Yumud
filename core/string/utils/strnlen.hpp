#pragma once

#include <cstdint>


namespace ymd{

template<typename T>
[[nodiscard]] static constexpr size_t strnlen_from_right(const T * str, size_t right) {
    if (right == 0) [[unlikely]] return 0;
    
    size_t idx = right;
    
    // 从右向左查找第一个非空字符
    do {
        if (str[--idx] != '\0') [[likely]] {
            // 找到了非空字符，继续向前查找字符串开始
            size_t length = 1;
            #pragma GCC unroll 8
            while (idx > 0) {
                if (str[--idx] == '\0') [[unlikely]] {
                    return length;
                }
                ++length;
            }
            return length;
        }
    } while (idx > 0);
    
    return 0;
}

template<typename T>
[[nodiscard]] static constexpr size_t strnlen_from_left(const T * str, size_t left){
    if(left == 0) [[unlikely]] return 0;
    
    size_t length = 0;
    #pragma GCC unroll 8
    for(size_t i = 0; i < left && str[i] != '\0'; i++){
        length++;
    }
    return length;
}

}