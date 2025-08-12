#pragma once

#include <cstdint>

namespace ymd{


template<typename T, size_t N>
struct SVector{
    std::array<T, N> data; 
};

}