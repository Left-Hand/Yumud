#pragma once

#include <cstdint>
#include "core/tmp/bits/width.hpp"
namespace ymd{

template<size_t NUM_BITS>
struct BitsRef{
    static constexpr size_t NUM_BYTES = tmp::bits::width<NUM_BITS>::value;
};

}