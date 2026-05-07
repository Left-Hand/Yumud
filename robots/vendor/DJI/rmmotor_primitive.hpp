#pragma once


#include <bitset>
#include <type_traits>

#include "core/math/fixed/fixed.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "rmmotor_utils.hpp"

namespace ymd::robots::dji{



struct alignas(2) [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;

    uint16_t bits;

    static constexpr Self zero(){
        return Self{0};
    }


};

//8192 max
struct alignas(2) [[nodiscard]] AngleCode final{
    using Self = AngleCode;

    uint16_t bits;

    static constexpr Self zero(){
        return Self{0};
    }


    static constexpr AngleCode from_angle(const Angular<uq32> angle) {
        const auto turns_bits = angle.to_turns().to_bits();
        const uint16_t angle_u13 = static_cast<uint16_t>(turns_bits >> (32u - 13u)) & 0x1FFF;
        return AngleCode{
            .bits = __builtin_bswap16(angle_u13)
        };
    }

    constexpr Angular<uq32> to_angle() const noexcept {
        const auto angle_u13 = uint16_t(__builtin_bswap16(bits));
        const auto turns = uq32::from_bits(angle_u13 << (32u - 13u));
        return Angular<uq32>::from_turns(turns);
    }
};

struct alignas(2) [[nodiscard]] SpeedCode final{
    using Self = SpeedCode;

    uint16_t bits;

    static constexpr Self zero(){
        return Self{0};
    }

    static constexpr SpeedCode from_tps(const float x) noexcept {
        const int16_t bits = int16_t(x * 60);
        return {__builtin_bswap16(bits)};
    }

    template<size_t Q>
    static constexpr SpeedCode from_tps(const math::fixed<Q, int32_t> x) noexcept {
        const int16_t bits = int16_t(x * 60);
        return {__builtin_bswap16(bits)};
    }

    template<typename T>
    constexpr T to_rpm() const noexcept {
        return static_cast<T>(std::bit_cast<int16_t>(__builtin_bswap16(bits)));
    }

    template<typename T>
    constexpr T to_tps() const noexcept {
        const auto temp_bits = std::bit_cast<int16_t>(__builtin_bswap16(bits));
        return utils::scale_1_by_60<T>::calc(temp_bits);
    }
};

struct alignas(1) [[nodiscard]] TemperatureCode final{
    using Self = TemperatureCode;

    uint8_t bits;

    static constexpr Self zero(){
        return Self{0};
    }


    constexpr int32_t to_celsius() const noexcept {
        return static_cast<int32_t>(bits) - 40;
    }
};


struct alignas(4) [[nodiscard]] TxPacket final{
    using Self = TxPacket;

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

static_assert(sizeof(TxPacket) == 8);


struct alignas(4) [[nodiscard]] RxPacket final{
    using Self = RxPacket;


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

static_assert(sizeof(RxPacket) == 8);

}