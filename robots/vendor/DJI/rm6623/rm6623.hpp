#pragma once

#include "rm6623_utils.hpp"
#include "../rmmotor_primitive.hpp"

namespace ymd::robots::dji{

static constexpr float MAX_CURRENT_AMPS = 5.0;


static constexpr hal::CanStdId RM6623_YAW_CANID = hal::CanStdId::from_u11(0x205);
static constexpr hal::CanStdId RM6623_PITCH_CANID = hal::CanStdId::from_u11(0x206);
static constexpr hal::CanStdId RM6623_ROLL_CANID = hal::CanStdId::from_u11(0x207);
static constexpr hal::CanStdId RM6623_EX1_CANID = hal::CanStdId::from_u11(0x209);
static constexpr hal::CanStdId RM6623_EX2_CANID = hal::CanStdId::from_u11(0x20a);
static constexpr hal::CanStdId RM6623_EX3_CANID = hal::CanStdId::from_u11(0x20b);
static constexpr hal::CanStdId RM6623_EX4_CANID = hal::CanStdId::from_u11(0x20c);

static constexpr hal::CanStdId RM6623_CALIBRATE_CANID = hal::CanStdId::from_u11(0x3f0);
static constexpr hal::ClassicCanFrame RM6623_CALIBRATE_CANFRAME = hal::ClassicCanFrame::from_parts(
    RM6623_CALIBRATE_CANID, hal::ClassicCanPayload::from_u64(static_cast<uint64_t>('c'))
);


struct RM6623CurrentCodeInterpreter{
    template<typename T>
    static constexpr CurrentCode from_amps_bounded(const T amps){
        const auto bits = rm6623::utils::scale_1000(amps);
        return CurrentCode{
            .bits = std::bit_cast<uint16_t>(__builtin_bswap16(bits))
        };
    }


    template<typename T>
    static constexpr T to_amps(const CurrentCode code) noexcept {
        return rm6623::utils::scale_1_by_1000<T>::calc(std::bit_cast<int16_t>(__builtin_bswap16(code.bits)));
    }
};


}