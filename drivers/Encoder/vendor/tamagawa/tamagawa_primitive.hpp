#pragma once

#include "core/utils/Result.hpp"
#include "core/container/heapless_vector.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "tamagawa_utils.hpp"

//多摩川编码器

// https://blog.csdn.net/qq_28149763/article/details/132718177



namespace ymd::drivers::tamagawa{

static constexpr size_t MAX_EEPROM_PAGE = 0x3c;

namespace primitive{

static constexpr size_t MAX_CONTEXT_SIZE = 8;


enum class [[nodiscard]] CfCode:uint8_t{
    GetAbs = 0x02,
    GetAbm = 0x8a,
    GetVersion = 0x92,
    GetAllInfo = 0x1a,
    WriteEEprom = 0x32,
    ReadEEprom = 0xea,
    ClearAbs = 0xc2,
    ClearAbmAndFault = 0x62
};

struct [[nodiscard]] StatusField final{
    using Self = StatusField;
    //delimitier error in request frame
    uint8_t speed_err:1;

    //paraity error in request frame
    uint8_t comm_err:1;

    //overheat / multiturn err / battery err / batter alarm
    uint8_t battery_under_voltage:1;

    //counting err
    uint8_t supply_under_voltage:1;

    uint8_t install_err:1;
    uint8_t __resv__:1;
    uint8_t multi_turns_err:1;
    uint8_t temp_err:1;

    static constexpr Self from_u8(const uint8_t b){
        return std::bit_cast<Self>(b);
    }


    [[nodiscard]] constexpr bool is_none() const noexcept {
        return std::bit_cast<uint8_t>(*this) == 0;
    }

    [[nodiscard]] constexpr uint8_t to_u8() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }
};

struct [[nodiscard]] Almc final{
    uint8_t over_speed:1;
    uint8_t full_absolute_status:1;
    uint8_t counting_error:1;
    uint8_t counter_overflow:1;
    uint8_t over_heat:1;
    uint8_t multiturn_error:1;
    uint8_t battery_error:1;
    uint8_t battery_alarm:1;
};

struct [[nodiscard]] Abs24 final{
    using Self = Abs24;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t b24() const noexcept {
        uint32_t bits = 0;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;
        return bits;
    }


    constexpr Angular<uq32> to_angle(size_t enc_resolution) const noexcept {
        const size_t shift_cnt = static_cast<size_t>(32u - enc_resolution);
        const uint32_t bits = static_cast<uint32_t>(static_cast<uint32_t>(b24()) << shift_cnt);
        return Angular<uq32>::from_turns(uq32::from_bits(bits));
    }

    constexpr void fill_bytes(std::span<uint8_t, 3> other_bytes) const noexcept {
        other_bytes[0] = bytes[0];
        other_bytes[1] = bytes[1];
        other_bytes[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t, 3> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

struct [[nodiscard]] Abm24 final{
    using Self = Abm24;
    std::array<uint8_t, 3> bytes;

    constexpr uint32_t to_turns() const noexcept {
        uint32_t bits;
        bits |= static_cast<uint32_t>(bytes[0]);
        bits |= static_cast<uint32_t>(bytes[1]) << 8;
        bits |= static_cast<uint32_t>(bytes[2]) << 16;

        return bits;
    }

    constexpr void fill_bytes(std::span<uint8_t> other_bytes){
        other_bytes[0] = bytes[0];
        other_bytes[1] = bytes[1];
        other_bytes[2] = bytes[2];
    }

    static constexpr Self from_bytes(std::span<const uint8_t> bytes){
        return Self{
            .bytes = {bytes[0], bytes[1], bytes[2]}
        };
    }
};

}



}