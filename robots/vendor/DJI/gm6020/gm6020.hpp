#pragma once


#include "gm6020_utils.hpp"
#include "../rmmotor_primitive.hpp"

namespace ymd::robots::dji{

static constexpr hal::CanStdId GM6020_VOLTAGE_CTL_LOWQUAD_CANID = hal::CanStdId::from_u11(0x1ff);
static constexpr hal::CanStdId GM6020_VOLTAGE_CTL_HIGHQUAD_CANID = hal::CanStdId::from_u11(0x2ff);

static constexpr hal::CanStdId GM6020_CURRENT_CTL_LOWQUAD_CANID = hal::CanStdId::from_u11(0x1fe);
static constexpr hal::CanStdId GM6020_CURRENT_CTL_HIGHQUAD_CANID = hal::CanStdId::from_u11(0x2fe);



struct GM6020CurrentCodeInterpreter{
    template<typename T>
    static constexpr CurrentCode from_amps_bounded(const T amps){
        const auto bits = utils::scale_16384_by_3(amps);
        return CurrentCode{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }


    template<typename T>
    static constexpr T to_amps(const CurrentCode code) noexcept {
        return utils::scale_3_by_16384<T>::calc(std::bit_cast<int16_t>(__builtin_bswap16(code.bits)));
    }
};
}