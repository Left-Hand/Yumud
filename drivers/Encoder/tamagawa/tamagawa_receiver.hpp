#pragma once

#include "tamagawa_msgs.hpp"

namespace ymd::drivers::tamagawa{


#if 0

struct [[nodiscard]] Response final{
    using Self = Response;

    CfCode cf;

    union{
        GetAbs _0;
        GetAbm _1;
        GetVersion _2;
        GetAllInfo _3;
        ClearFault _7;
        ClearAbs _8;
        ClearAbmAndFault _c;
        std::array<uint8_t, 10> bytes;
    }context;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept {
        return std::span{reinterpret_cast<const uint8_t *>(this), 11};
    }

};

#endif

}