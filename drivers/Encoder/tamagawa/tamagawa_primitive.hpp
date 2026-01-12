#pragma once

#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angular.hpp"

//多摩川编码器
namespace ymd::drivers::tamagawa{

namespace primitive{
enum class [[nodiscard]] Command:uint8_t{
    GetAbsoluteLap = 0,
    GetMultiTurns = 1,
    GetEncoderId = 2,
    GetFullInfo = 3,
    WriteEEprom = 6,
    ReadEEprom = 0x0d,
    ClearError = 0x07,
    ResetTurns = 0x08,
    ClearAll = 0x0c
};

struct [[nodiscard]] ControlField final{
    using Self = ControlField;
};

struct [[nodiscard]] StatusField final{
    //delimitier error in request frame
    uint8_t ca1:1;

    //paraity error in request frame
    uint8_t ca2:1;

    //overheat / multiturn err / battery err / batter alarm
    uint8_t ea1:1;

    //counting err
    uint8_t ea2:1;
    uint8_t :4;
};

struct [[nodiscard]] Amlc final{
    uint8_t over_speed:1;
    uint8_t full_absolute_status:1;
    uint8_t counting_error:1;
    uint8_t counter_overflow:1;
    uint8_t over_heat:1;
    uint8_t multiturn_error:1;
    uint8_t battery_error:1;
    uint8_t battery_alarm:1;
};

struct [[nodiscard]] AbsolutionData final{
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t b24() const {
        uint32_t bits;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;
        return bits;
    }


    constexpr Angular<uq32> to_angle(size_t q) const {
        const size_t shift_cnt = static_cast<size_t>(32u - q);
        const uint32_t bits = static_cast<uint32_t>(static_cast<uint32_t>(b24()) << shift_cnt);
        return Angular<uq32>::from_turns(uq32::from_bits(bits));
    }
};

struct [[nodiscard]] MultiTurnData final{
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t to_turns() const {
        uint32_t bits;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;

        return bits;
    }
};
}



namespace req_msgs{
    
};
}