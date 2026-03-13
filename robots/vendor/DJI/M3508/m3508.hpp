#pragma once

#include <bitset>

#include "primitive/can/bxcan_frame.hpp"
#include "core/math/fixed/fixed.hpp"
#include "primitive/arithmetic/angular.hpp"


namespace ymd::robots::dji::m3508{
static constexpr uint16_t NUM_HIGHER_QUAD_CAN_ID = 0x200;
static constexpr uint16_t NUM_LOWER_QUAD_CAN_ID = 0x1ff;
static constexpr size_t NUM_MAX_MOTORS = 8;

struct [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;

    uint16_t bits;
    static constexpr Self from_amps(const iq16 amps){
        int16_t temp = int16_t((amps / 20) << 14);
        return Self{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(temp))
        };
    }
    constexpr iq16 to_amps() const {
        return (iq16(std::bit_cast<int16_t>(bits)) >> 14) * 20;
    }
};

struct [[nodiscard]] AngleCode final{
    uint16_t bits;

    constexpr Angular<uq32> to_angle() const {
        const auto angle_u13 = uint16_t(__builtin_bswap16(bits));
        const auto turns = uq32::from_bits(angle_u13 << (32u - 13u));
        return Angular<uq32>::from_turns(turns);
    }
};

struct [[nodiscard]] SpeedCode final{
    uint16_t bits;

    constexpr iq16 to_tps() const {
        return uq32(1.0 / 60) * std::bit_cast<int16_t>(__builtin_bswap16(bits));
    }
};

struct [[nodiscard]] TemperatureCode final{
    uint8_t bits;

    constexpr iq16 to_celsius() const {
        return iq16(bits) - 40;
    }
};


struct alignas(8) [[nodiscard]] TxContext final{
    using Self = TxContext;

    std::array<CurrentCode, 4> current_codes; 

    constexpr hal::BxCanPayload to_can_payload() const {
        return hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }

    constexpr Self from_can_payload(const hal::BxCanPayload& payload){
        return std::bit_cast<Self>(payload.to_u64());
    }
};

static_assert(sizeof(TxContext) == 8);

struct alignas(8) [[nodiscard]] RxContext final{
    using Self = RxContext;
    AngleCode angle_code;
    CurrentCode current_code;
    SpeedCode speed_code;
    TemperatureCode temperature_code;
    constexpr Self from_bytes(std::span<const uint8_t, 8> bytes) const {
        return Self{
            .angle_code = AngleCode{.bits = static_cast<uint16_t>(bytes[0] | (bytes[1] << 8))},
            .current_code = CurrentCode{.bits = static_cast<uint16_t>(bytes[2] | (bytes[3] << 8))},
            .speed_code = SpeedCode{static_cast<uint16_t>(bytes[4] | (bytes[5] << 8))},
            .temperature_code = TemperatureCode{.bits = bytes[6]}
        };
    }
};

static_assert(sizeof(RxContext) == 8);


};