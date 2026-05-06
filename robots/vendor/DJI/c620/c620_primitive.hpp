#pragma once


#include "c620_utils.hpp"
#include "primitive/can/bxcan_frame.hpp"



namespace ymd::robots::dji::c620{


static constexpr hal::CanStdId HIGHER_QUAD_CANID = hal::CanStdId::from_u11(0x200);
static constexpr hal::CanStdId LOWER_QUAD_CANID = hal::CanStdId::from_u11(0x1ff);
static constexpr size_t NUM_MAX_MOTORS = 8;

struct alignas(2) [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;

    uint16_t bits;

    static constexpr Self zero(){
        return Self{0};
    }

    template<typename T>
    static constexpr Self from_amps_bounded(const T amps){
        const auto bits = utils::scale_16384_by_20(amps);
        return Self{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }


    template<typename T>
    static constexpr Self from_amps_bounded_c610(const T amps){
        const auto bits = utils::scale_16384_by_10(amps);
        return Self{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }

    template<typename T>
    constexpr T to_amps() const noexcept {
        return utils::scale_20_by_16384<T>::calc(std::bit_cast<int16_t>(__builtin_bswap16(bits)));
    }

    template<typename T>
    constexpr T to_amps_c610() const noexcept {
        return to_amps<T>() * static_cast<T>(0.5);
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



};