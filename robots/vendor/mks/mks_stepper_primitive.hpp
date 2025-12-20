#pragma once

#include "core/math/realmath.hpp"
#include "core/utils/Result.hpp"
#include "core/container/heapless_vector.hpp"
#include "core/tmp/reflect/enum.hpp"


#include "hal/bus/uart/uarthw.hpp"

#include "algebra/regions/range2.hpp"

namespace ymd::robots::mksmotor{

struct [[nodiscard]] NodeId{
    using Self = NodeId;
    uint8_t count;

    static constexpr NodeId from_u8(uint8_t bits) {
        return NodeId{bits};
    }
    [[nodiscard]] constexpr uint8_t to_u8() const {
        return count;
    }
};



enum class [[nodiscard]] Error:uint8_t{
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


enum class [[nodiscard]] HommingMode:uint8_t{ 
    Disabled = 0,
    Dir = 1,
    Nearest = 2,
};

enum class [[nodiscard]] SetHommingParaStatus:uint8_t{ 
    Failed = 0,
    Success = 1
};

enum class [[nodiscard]] HommingSpeed:uint8_t{
    _0 = 0, 
    _1, _2, _3, _4
};

enum class [[nodiscard]] HommingDirection:uint8_t{
    Clockwise = 0,
    CounterClockwise = 1
};


enum class [[nodiscard]] PositionCtrlStatus:uint8_t{
    Failed = 0,
    Started,
    Completed
};

enum class [[nodiscard]] EndstopHomingStatus:uint8_t{ 
    Failed = 0,
    Started = 1,
    Completed = 2
};

enum class [[nodiscard]] MotivationState:uint8_t{
    Failed = 0,
    Stopped,
    Acc,
    Deacc,
    FullSpeed,
    Zero
};

enum class [[nodiscard]] WorkMode:uint8_t{
    PulseOpenloop = 0,
    PulseCloseloop = 1,
    PulseFoc = 2,
    SerialOpenloop = 3,
    SerialCloseloop = 4,
    SerialFoc = 5,
};

enum class [[nodiscard]] CanBitrate:uint8_t{
    _125K = 0,
    _250K = 1,
    _500K = 2
};


enum class [[nodiscard]] FuncCode:uint8_t{
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

[[nodiscard]] static constexpr uint8_t get_verify_code(
    const NodeId nodeid,
    const FuncCode func_code,
    std::span<const uint8_t> bytes 
){
    uint32_t sum = nodeid.to_u8();
    sum += std::bit_cast<uint8_t>(func_code);
    for(const auto byte: bytes){
        sum += byte;
    }

    return static_cast<uint8_t>(sum);
}


struct [[nodiscard]] Rpm final{
    static constexpr Rpm from_tps(const iq16 tps){
        return {int16_t(tps * 60)};
    }
    constexpr int16_t to_bits() const {
        return bits;
    }

    constexpr iq16 to_tps() const {
        return iq16(bits) / 60;
    }

    int16_t bits;
};

static_assert(sizeof(Rpm) == 2);

struct [[nodiscard]] iRpm final{
    static constexpr iRpm from_tps(const iq16 tps){
        return {int16_t(int16_t(tps * 60) & int16_t(0x8fff))};
    }
    constexpr int16_t to_bits() const {
        return bits;
    }

    constexpr iq16 to_tps() const {
        return iq16(bits) / 60;
    }

    int16_t bits;
};

static_assert(sizeof(iRpm) == 2);

struct [[nodiscard]] AcclerationLevel{
    static constexpr AcclerationLevel from_tpss(const iq16 tpss){
        // TODO
        return AcclerationLevel{uint8_t(tpss)};
    }

    uint8_t bits;
};

static_assert(sizeof(AcclerationLevel) == 1);

#pragma pack(push, 1)
struct [[nodiscard]] PulseCnt final{
    static constexpr uint32_t PULSES_PER_TURN = 3200 * (256/16);
    static constexpr PulseCnt from_turns(const iq16 turns){
        return from_pulses(uint32_t(turns * PULSES_PER_TURN));
    }

    static constexpr PulseCnt from_pulses(const uint32_t pulses){
        return PulseCnt{
            .bytes = {uint8_t(pulses >> 16), uint8_t(pulses >> 8), uint8_t(pulses)}
        };
    }

    constexpr uint32_t as_u24() const {
        return (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
    }

    uint8_t bytes[3];
};
#pragma pack(pop)
static_assert(sizeof(PulseCnt) == 3);


}
