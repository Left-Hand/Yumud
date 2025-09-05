#pragma once

#include "core/math/realmath.hpp"
#include "core/utils/Result.hpp"
#include "core/container/inline_vector.hpp"
#include "core/utils/nodeid.hpp"
#include "core/magic/enum_traits.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "types/regions/range2.hpp"

namespace ymd::robots{


namespace mksmotor{

namespace prelude{


enum class Error:uint8_t{
    SubDivideOverflow,
    RxNoMsgToDump,
    RxMsgIdTypeNotMatch,
    RxMsgNodeIdNotTheSame,
    RxMsgFuncCodeNotTheSame,
    RxMsgPieceIsNotSteady,
    RxMsgVerifyed,
    RxMsgNoPayload
};

DEF_DERIVE_DEBUG(Error)

using Buf = HeaplessVector<uint8_t, 8>;

template<typename T = void>
using IResult = Result<T, Error>;


enum class HommingMode:uint8_t{ 
    Disabled = 0,
    Dir = 1,
    Nearest = 2,
};

enum class SetHommingParaStatus:uint8_t{ 
    Failed = 0,
    Success = 1
};

enum class HommingSpeed:uint8_t{
    _0, _1, _2, _3, _4
};

enum class HommingDirection:uint8_t{
    Clockwise = 0,
    CounterClockwise = 1
};


enum class PositionCtrlStatus:uint8_t{
    Failed,
    Started,
    Completed
};

enum class EndstopHomingStatus:uint8_t{ 
    Failed = 0,
    Started = 1,
    Completed = 2
};

enum class MotivationState:uint8_t{
    Failed,
    Stopped,
    Acc,
    Deacc,
    FullSpeed,
    Zero
};

enum class WorkMode:uint8_t{
    PulseOpenloop,
    PulseCloseloop,
    PulseFoc,
    SerialOpenloop,
    SerialCloseloop,
    SerialFoc,
};

enum class CanBitrate:uint8_t{
    _125K,
    _250K,
    _500K
};


enum class FuncCode:uint8_t{
    GetMultiLapEncoderValue = 0x30,
    // GetMultiLEncoderValue = 0x31,
    GetRealtimeSpeed = 0x32,
    GetAccumulatedPulses = 0x33,
    GetIoPortStatus = 0x34,
    GetPositionErr = 0x39,
    GetEnableStatus = 0x3a,
    GetPowerOnHommingStatus = 0x3b,
    EscapeStallState = 0x3d,
    GetStallFlag = 0x3e,
    TrigCalibrateEncoder = 0x80, 
    SetSubdivides = 0x84,
    SetGroupAddr = 0x8d,
    SetKeyLocked = 0x8f,
    SetEndstopParaments = 0x90,
    EndstopHomming = 0x91,
    SetEnableStatus = 0xf3,
    PositionCtrl3 = 0xf5,
    SpeedCtrl = 0xf6
};


struct VerifyUtils final{
    static constexpr uint8_t get_verify_code(
        const NodeId nodeid,
        const FuncCode funccode,
        std::span<const uint8_t> bytes 
    ){
        uint32_t sum = nodeid.as_u8();
        sum += std::bit_cast<uint8_t>(funccode);
        for(const auto byte: bytes){
            sum += byte;
        }

        return static_cast<uint8_t>(sum);
    }

};

struct Rpm final{
    static constexpr Rpm from_speed(const real_t speed){
        return {int16_t(speed * 60)};
    }
    constexpr int16_t as_i16() const {
        return raw_;
    }

    constexpr real_t to_speed() const {
        return real_t(raw_) / 60;
    }

    int16_t raw_;
}__packed;

struct iRpm final{
    static constexpr iRpm from_speed(const real_t speed){
        return {int16_t(int16_t(speed * 60) & int16_t(0x8fff))};
    }
    constexpr int16_t as_i16() const {
        return raw_;
    }

    constexpr real_t to_speed() const {
        return real_t(raw_) / 60;
    }

    int16_t raw_;
}__packed;

static_assert(sizeof(Rpm) == 2);

struct AcclerationLevel{
    static constexpr AcclerationLevel from(const real_t acc_per_second){
        // TODO
        return AcclerationLevel{uint8_t(acc_per_second)};
    }

    uint8_t raw_;
}__packed;

static_assert(sizeof(AcclerationLevel) == 1);


struct PulseCnt final{
    static constexpr uint32_t SCALE = 3200 * (256/16);
    static constexpr PulseCnt from_position(const real_t position){
        return from_pulses(uint32_t(position * SCALE));
    }

    static constexpr PulseCnt from_pulses(const uint32_t pulses){
        return PulseCnt{
            .buf_ = {uint8_t(pulses >> 16), uint8_t(pulses >> 8), uint8_t(pulses)}
        };
    }

    constexpr uint32_t as_u24() const {
        return (buf_[0] << 16) | (buf_[1] << 8) | buf_[2];
    }

    uint8_t buf_[3];
}__packed;



}

namespace payloads{
    using namespace prelude;


    struct SetPositionMode3 final{
        static constexpr FuncCode FUNC_CODE = FuncCode::PositionCtrl3;
        Rpm rpm;
        AcclerationLevel acc_level;
        PulseCnt abs_pulse_cnt;
    }__packed;

    struct StopPositionMode3 final{
        static constexpr FuncCode FUNC_CODE = FuncCode::PositionCtrl3;

        const Rpm rpm = Rpm::from_speed(0);
        AcclerationLevel acc_level;
        const PulseCnt abs_pulse_cnt = PulseCnt::from_pulses(0);
    }__packed;

    struct SetSpeed final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SpeedCtrl;
        iRpm rpm;
        AcclerationLevel acc_level;
    }__packed;

    struct SetEnableStatus final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetEnableStatus;
        bool enable;
    }__packed;


    struct SetSubdivides final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetSubdivides;
        uint8_t subdivides;
    }__packed;

    struct SetEndstopParaments final{
        static constexpr FuncCode FUNC_CODE = FuncCode::SetEndstopParaments;
        bool is_high;
        bool is_ccw;
        Rpm rpm;
    };

    struct EndstopHomming final{
        static constexpr FuncCode FUNC_CODE = FuncCode::EndstopHomming;
    }__packed;

    template<typename Raw, typename T = std::decay_t<Raw>>
    static std::span<const uint8_t> serialize(
        Raw && obj
    ){
        return std::span(
            reinterpret_cast<const uint8_t *>(&obj),
            magic::pure_sizeof_v<T>
        );
    }
}
}

}