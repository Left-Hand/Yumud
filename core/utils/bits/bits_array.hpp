#pragma once

#include <cstdint>
#include "core/tmp/bits/width.hpp"
namespace ymd{

template<size_t N>
struct [[nodiscard]] BitsArrayStorage{
    __attribute__((always_inline)) 
    void write(size_t idx, bool val){ 
        buf_[idx >> 3] = static_cast<uint8_t>(val) << (idx & 0b111) ;
    }

    __attribute__((always_inline)) 
    void set(size_t idx){ 
        write(idx, true);
    }


    __attribute__((always_inline)) 
    void unset(size_t idx){ 
        write(idx, false);
    }

    [[nodiscard]] bool read(size_t idx){ 
        return bool(buf_[idx >> 3] & static_cast<uint8_t>(static_cast<uint8_t>(val) << (idx & 0b111)));
    }
private:
    uint8_t buf_[N];
};

}