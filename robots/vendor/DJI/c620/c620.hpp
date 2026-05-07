#pragma once


#include "c620_utils.hpp"
#include "../rmmotor_primitive.hpp"

namespace ymd::robots::dji{


static constexpr hal::CanStdId C6x0_HIGHER_QUAD_CANID = hal::CanStdId::from_u11(0x200);
static constexpr hal::CanStdId C6x0_LOWER_QUAD_CANID = hal::CanStdId::from_u11(0x1ff);
static constexpr size_t C6x0_NUM_MAX_MOTORS = 8;

struct C620CurrentCodeInterpreter{
    template<typename T>
    static constexpr CurrentCode from_amps_bounded(const T amps){
        const auto bits = c620::utils::scale_16384_by_20(amps);
        return CurrentCode{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }

    template<typename T>
    static constexpr T to_amps(const CurrentCode code) noexcept {
        return c620::utils::scale_20_by_16384<T>::calc(std::bit_cast<int16_t>(__builtin_bswap16(code.bits)));
    }
};

struct C610CurrentCodeInterpreter{
    template<typename T>
    static constexpr CurrentCode from_amps_bounded(const T amps){
        const auto bits = c620::utils::scale_16384_by_10(amps);
        return CurrentCode{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }


    template<typename T>
    static constexpr T to_amps(const CurrentCode code) noexcept {
        return C620CurrentCodeInterpreter::to_amps<T>(code) * static_cast<T>(0.5);
    }
};

}