#pragma once


#include "c620_primitive.hpp"

namespace ymd::robots::dji::c620{


struct alignas(4) [[nodiscard]] TxContext final{
    using Self = TxContext;

    std::array<CurrentCode, 4> current_codes; 

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        return hal::ClassicCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }

    constexpr hal::ClassicCanFrame to_can_frame(const hal::CanStdId can_id) const noexcept{
        return hal::ClassicCanFrame::from_parts(
            can_id,
            to_can_payload()
        );
    }

    constexpr Self from_can_payload(const hal::ClassicCanPayload& payload){
        return std::bit_cast<Self>(payload.to_u64());
    }
};

static_assert(sizeof(TxContext) == 8);

struct alignas(4) [[nodiscard]] RxContext final{
    using Self = RxContext;


    AngleCode angle_code;
    SpeedCode speed_code;
    CurrentCode current_code;
    TemperatureCode temperature_code;


    constexpr Self from_bytes(std::span<const uint8_t, 8> bytes) const noexcept {
        return Self{
            .angle_code = AngleCode{.bits = static_cast<uint16_t>(bytes[0] | (bytes[1] << 8))},
            .speed_code = SpeedCode{static_cast<uint16_t>(bytes[2] | (bytes[3] << 8))},
            .current_code = CurrentCode{.bits = static_cast<uint16_t>(bytes[4] | (bytes[5] << 8))},
            .temperature_code = TemperatureCode{.bits = bytes[6]}
        };
    }

    constexpr Self from_bytes_c610(std::span<const uint8_t, 8> bytes) const noexcept {
        return Self{
            .angle_code = AngleCode{.bits = static_cast<uint16_t>(bytes[0] | (bytes[1] << 8))},
            .speed_code = SpeedCode{static_cast<uint16_t>(bytes[2] | (bytes[3] << 8))},
            .current_code = CurrentCode{.bits = static_cast<uint16_t>(bytes[4] | (bytes[5] << 8))},
            .temperature_code = TemperatureCode::zero()
        };
    }

    constexpr Self from_can_payload(const hal::ClassicCanPayload& payload){
        return std::bit_cast<Self>(payload.to_u64());
    }

    constexpr hal::ClassicCanPayload to_can_payload() const noexcept {
        return hal::ClassicCanPayload::from_u64(std::bit_cast<uint64_t>(*this));
    }
};

static_assert(sizeof(RxContext) == 8);

}