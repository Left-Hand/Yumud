#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct PAJ7620_Prelude{
    struct Flags{
        uint8_t right:1;
        uint8_t left:1;
        uint8_t up:1;
        uint8_t down:1;
        uint8_t forward:1;
        uint8_t backward:1;
        uint8_t cw:1;
        uint8_t ccw:1;

        constexpr std::bitset<8> to_bitset() const{
            return std::bitset<8>(std::bit_cast<uint8_t>(*this));
        }
    };

    static_assert(sizeof(Flags) == 1);

    enum class Error_Kind:uint8_t{
        Data0ValidateFailed,
        Data1ValidateFailed
    };

    DEF_FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;


};


}
