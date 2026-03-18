#pragma once

#include <cstdint>
namespace ymd::intrinsics{


constexpr uint32_t bmask32(const bool x){
    return static_cast<uint32_t>(-int32_t(x));
}
}