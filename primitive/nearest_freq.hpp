#pragma once

#include "core/stream/ostream.hpp"

namespace ymd::hal{
struct [[nodiscard]] NearestFreq{
    using Self = NearestFreq;
    uint32_t count;

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};

struct [[nodiscard]] LeastFreq{
    using Self = LeastFreq;
    uint32_t count;

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};
}
