#pragma once

#include <cstdint>

namespace ymd::str{


static constexpr  uint32_t pow10_table[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
    1000000UL, 
    10000000UL, 
    
    // 100000000UL,
    // 1000000000UL
};

constexpr size_t num_int2str_chars(uint64_t int_val, const uint8_t radix){
    if(int_val == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
    while(int_val >= sum){
        sum = sum * static_cast<uint64_t>(radix);
        i++;
    }
    return i > 0 ? i : 1;
}

}