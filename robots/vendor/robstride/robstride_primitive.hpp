#pragma once

#include "core/stream/ostream.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/bits/bits_set.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd::robots::robstride{

namespace primitive{
using literals::iq16;

enum class [[nodiscard]] DeMsgError:uint8_t{
    StdFrame,
    InvalidCommand,
    Arg2Not0xfe,
    PayloadNot8Bytes,
};

enum class Command:uint8_t{
    GetDeviceId = 0,// 4.1.1@37
    CommunicationType1 = 1,//4.1.2@37
    CommunicationType2 = 2,
    EnableRunning = 3,
    DisableRunning = 4,
    SetMechanicalZero = 6,
    SetMotorId = 7,
    ReadSingleParament = 17
};

enum class DictKey:uint16_t{
    RunMode = 0x7005,
    IqRef = 0x7006,
    IdRef = 0x7007,
    SpeedRef = 0x700a,
    TorqueLimit = 0x700b,
    NowKp = 0x7010,
    NowKi = 0x7011,
    CurrentFilterGain = 0x7014,
    LocRef = 0x7016,
    SpeedLimit = 0x7017,
    CurrentLimit = 0x7018,
    MechicalPosition = 0x7019,
    IqFilter = 0x701a,
    LoadSpeed = 0x701b,
    BusbarVoltage = 0x701c,
    LocKp = 0x701e,
    SpeedKp = 0x701f,
    SpeedKi = 0x7020,
    SpeedFilterGain = 0x7021,
    Accereleration = 0x7022,
    SpeedLimitPP = 0x7024,
    AccelerationPP = 0x7025,
    EpscanPeriod = 0x7026,
    CanTimeout = 0x7028,
    // indicates the zero flagbit
    // 0:0~2π
    // 1:-π~π
    ZeroSta = 0x7029,
    ZeroOffset = 0x702b
};


struct [[nodiscard]] DictVal{
    uint32_t bits;
    
    template<typename T>
    constexpr explicit DictVal(const T val){
        static_assert(sizeof(T) <= 4);
        if constexpr(sizeof(T) == 1){
            this->bits = static_cast<uint32_t>(std::bit_cast<uint8_t>(val));
        }else if constexpr(sizeof(T) == 2){
            this->bits = static_cast<uint32_t>(std::bit_cast<uint16_t>(val));
        }else if constexpr(sizeof(T) == 4){
            this->bits = static_cast<uint32_t>(std::bit_cast<uint32_t>(val));
        }else{
            __builtin_trap();
        }
    }

    template<typename T>
    constexpr T to() const {
        static_assert(sizeof(T) <= 4);
        if constexpr(sizeof(T) == 1){
            return std::bit_cast<T>(static_cast<uint8_t>(bits));
        }else if constexpr(sizeof(T) == 2){
            return std::bit_cast<T>(static_cast<uint16_t>(bits));
        }else if constexpr(sizeof(T) == 4){
            return std::bit_cast<T>(static_cast<uint32_t>(bits));
        }else{
            __builtin_trap();
        }
    }

    constexpr std::array<uint8_t, 4> to_bytes() const{
        return std::bit_cast<std::array<uint8_t, 4>>(bits);
    }
};

struct [[nodiscard]] NodeId final{
    uint8_t count;
};


static constexpr Option<Command> try_into_command(const uint8_t b){ 
    //TODO
    return Some(static_cast<Command>(b));
}

struct [[nodiscard]] CanIdFields final{
    using Self = CanIdFields;
    Command command;
    uint16_t arg1;
    uint8_t arg2;

    static constexpr Result<Self, DeMsgError> try_from(const hal::CanExtId & id){
        Self self{
            .command = ({
                const auto may_command = try_into_command(id.to_bits() >> 24);
                if(may_command.is_none()) return Err(DeMsgError::InvalidCommand);
                may_command.unwrap();
            }),
            .arg1 = static_cast<uint16_t>((id.to_bits() >> 8) & 0xffff),
            .arg2 = static_cast<uint8_t>(id.to_bits() & 0xff),
        };
        return Ok(self);
    }

    [[nodiscard]] constexpr hal::CanExtId pack() const{
        const auto bits = (static_cast<uint8_t>(command) << 24) | (arg1 << 8) | arg2;
        return hal::CanExtId::from_bits(bits);
    }
};

struct [[nodiscard]] TorqueCode final{
    using Self = TorqueCode;
    // (0~65535) corresponds to(-6Nm~6Nm)
    uint16_t bits;

    static constexpr TorqueCode from_bits(const uint16_t bits){
        return TorqueCode{bits};
    }

    static constexpr Result<TorqueCode, std::partial_ordering> try_from_nm(const iq16 nm){ 
        if(nm > 6) return Err(std::partial_ordering::greater);
        if(nm < -6) return Err(std::partial_ordering::less);
        const uint16_t bits = static_cast<uint16_t>(
            ((nm + 6) * iq16(1.0/12)).to_bits()
        );
        return Ok(TorqueCode::from_bits(bits));
    }

    constexpr iq16 to_nm() const {
        return iq16::from_bits(static_cast<uint32_t>(bits)) * 12 - 6;
    }
};

struct [[nodiscard]] AngleCode final{
    using Self = AngleCode;
    // (0~65535) corresponds to(-4pi~4pi)
    uint16_t bits;

    static constexpr AngleCode from_bits(const uint16_t bits){
        return AngleCode{bits};
    }

    static constexpr Result<AngleCode, std::partial_ordering> try_from_angle(const Angular<iq16> angle){ 
        if(angle.to_turns() > 2) return Err(std::partial_ordering::greater);
        if(angle.to_turns() < -2) return Err(std::partial_ordering::less);
        const uint16_t bits = static_cast<uint16_t>(
            ((angle.to_turns() + 2) >> 2).to_bits()
        );
        return Ok(AngleCode::from_bits(bits));
    }

    constexpr Angular<iq16> to_angle() const {
        return Angular<iq16>::from_turns(iq16::from_bits(static_cast<uint32_t>(bits)) * 2 - 4);
    }
};

struct [[nodiscard]] AngularVelocityCode final{
    using Self = AngularVelocityCode;
    // (0~65535) corresponds to(-50rad/s~50rad/s)
    uint16_t bits;

    static constexpr AngularVelocityCode from_bits(const uint16_t bits){
        return AngularVelocityCode{bits};
    }

    static constexpr Result<AngularVelocityCode, std::partial_ordering> try_from(const Angular<iq16> speed){ 
        const auto rads = speed.to_radians();
        if(rads > 50) return Err(std::partial_ordering::greater);
        if(rads < -50) return Err(std::partial_ordering::less);
        const uint16_t bits = static_cast<uint16_t>((rads * uq16(1.0/50)).to_bits());
        return Ok(AngularVelocityCode::from_bits(bits));
    }

    constexpr Angular<iq16> to_speed() const {
        return Angular<iq16>::from_radians(iq16::from_bits(static_cast<uint32_t>(bits)) * 50);
    }
};

struct [[nodiscard]] KpCode final{
    using Self = KpCode;
    // (0~65535) corresponds to(0.0~500.00)
    uint16_t bits;

    static constexpr KpCode from_bits(const uint16_t bits){
        return KpCode{bits};
    }

    static constexpr Result<KpCode, std::partial_ordering> try_from_val(const uq16 val){ 
        if(val > 500) return Err(std::partial_ordering::greater);
        const uint16_t bits = static_cast<uint16_t>(
            (val * uq16(65535/500))
        );

        return Ok(KpCode::from_bits(bits));
    }

    constexpr uq16 to_val() const {
        return uq16::from_bits(static_cast<uint32_t>(bits)) * 500 / 65535;
    }
};

struct [[nodiscard]] KdCode final{
    using Self = KdCode;
    // (0~65535) corresponds to(0.0~5.00)
    uint16_t bits;

    static constexpr KdCode from_bits(const uint16_t bits){
        return KdCode{bits};
    }

    static constexpr Result<KdCode, std::partial_ordering> try_from_val(const uq16 val){ 
        if(val > 5) return Err(std::partial_ordering::greater);
        const uint16_t bits = static_cast<uint16_t>(
            (val * uq16(65535/5))
        );

        return Ok(KdCode::from_bits(bits));
    }

    constexpr uq16 to_val() const {
        return uq16::from_bits(static_cast<uint32_t>(bits)) * 5 / 65535;
    }
};

struct TemperatureCode final{
    using Self = TemperatureCode;
    // Currenttemperature:
    // Temp(Celsius)*10Ifthevalueis
    // higherthan10, thehighbyteis
    // firstandthelowbyteislast
    uint16_t bits;

    static constexpr TemperatureCode from_bits(const uint16_t bits){
        return TemperatureCode{bits};
    }
};

enum class Mode:uint8_t{
    Reset = 0,
    Cali  = 1,
    Run = 2
};

struct FaultFlags{
    uint8_t under_voltage:1;
    uint8_t phase_current:1;
    uint8_t over_temperature:1;
    uint8_t encoder_coding:1;

    uint8_t gridlock_overload:1;
    uint8_t uncalibrated:1;
    Mode mode:2;
};

}


}