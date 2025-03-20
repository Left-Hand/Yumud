#pragma once

#include "core/platform.hpp"

namespace ymd::drivers{
class RzEncoder{
protected:
    using Timming = std::pair<uint32_t, uint32_t>;
    //ns : ns

    Timming _h_timming;
    Timming _l_timming;

    virtual void sendBit(const bool bit) = 0;
public:
    RzEncoder(
        const Timming & h_timming,
        const Timming & l_timming
    ):
        _h_timming(h_timming),
        _l_timming(l_timming){;}
};

}