#pragma once

#include "core/utils/Option.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/bytes/bytes_caster.hpp"

namespace ymd::xdrive::can_protocol{

enum class [[nodiscard]] Mode:uint32_t{
    Stop = 0,
    CommandPosition,
    CommandVelocity,
    CommandCurrent,
    CommandTrajectory,
    PwmPosition,
    PwmVelocity,
    PwmCurrent,
    StepDir,
};

enum class [[nodiscard]] State:uint32_t{
    Stop = 0,
    Finish,
    Running,
    Overload,
    Stall,
    NoCalib
};

enum class [[nodiscard]] CommandKind: uint8_t{
    EnableMotor = 0x01,
    DoCalibration = 0x02,
    SetCurrentSetPoint = 0x03,
    SetVelocitySetPoint = 0x04,
    
    SetPositionSetpoint = 0x05,
    SetPositionWithTime = 0x06,
    SetPositionWithVelocityLimit = 0x07,
    SetNodeIdAndStore = 0x11,
    SetCurrentLimitAndStore = 0x12,
    SetVelocityLimitAndStore = 0x13,
    SetAccelerationLimitAndStore = 0x14,
    ApplyHomePositionAndStore = 0x15,
    SetAutoEnableAndStore = 0x16,
    SetDceKp = 0x17,
    SetDceKv = 0x18,
    SetDceKi = 0x19,
    EnableStallProtect = 0x1b,

    GetCurrent = 0x21,
    GetVelocity = 0x22,
    GetPosition = 0x23,
    GetOffset = 0x23,
    EraseConfigs = 0x7e,
    Reboot = 0x7f,
};

enum class [[nodiscard]] DeserialzeError:uint8_t{
    DlcTooShort,
    DlcTooLong
};

using CanFrame = hal::BxCanFrame;
using CanPayload = hal::BxCanPayload;


struct [[nodiscard]] BytesReader{
    explicit constexpr BytesReader(std::span<const uint8_t> bytes) : 
        bytes_(bytes) {}

    template<typename T>
    [[nodiscard]] constexpr Option<T> try_fetch(){
        if(remaining().size() < sizeof(T))
            return None;
        return Some(le_bytes_to_int<T>(fetch_bytes<sizeof(T)>()));
    }

    template<typename T>
    [[nodiscard]] constexpr T fetch(){
        if(remaining().size() < sizeof(T))
            __builtin_trap();
        return le_bytes_to_int<T>(fetch_bytes<sizeof(T)>());
    }

private:
    std::span<const uint8_t> bytes_;

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_bytes(){
        const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
        bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
        return ret;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return bytes_;
    }
};

namespace req_msgs{
struct EnableMotor{
    using Self = EnableMotor;
    static constexpr CommandKind COMMAND = CommandKind::EnableMotor;
    static constexpr size_t FIXED_LENGTH = 4;
    Mode request_mode;

    static constexpr Result<Self, DeserialzeError> 
    from_bytes(std::span<const uint8_t, FIXED_LENGTH> bytes){ 
        const bool en = bytes[0] == 1;
        return Ok(Self{
            .request_mode = en ? Mode::CommandVelocity : Mode::Stop
        });
    }

    constexpr CanPayload to_can_payload() const{ 
        uint8_t buf[4] = {request_mode != Mode::Stop, 0, 0, 0};
        return CanPayload::from_bytes(buf);
    }
};


struct DoCalibration{
    static constexpr CommandKind COMMAND = CommandKind::DoCalibration;
};

struct SetCurrentSetPoint{
    using Self = SetCurrentSetPoint;
    static constexpr CommandKind COMMAND = CommandKind::SetCurrentSetPoint;
    static constexpr size_t FIXED_LENGTH = 4;
    math::fp32 current;

    static constexpr Result<Self, DeserialzeError> 
    from_bytes(std::span<const uint8_t, FIXED_LENGTH> bytes){ 
            return Err(DeserialzeError::DlcTooLong);
        BytesReader reader(bytes);
        return Ok(Self{
            .current = math::fp32::from_bits(reader.fetch<uint32_t>())
        });
    }

};

struct SetVelocitySetPoint{
    using Self = SetVelocitySetPoint;
    static constexpr CommandKind COMMAND = CommandKind::SetCurrentSetPoint;
    static constexpr size_t FIXED_LENGTH = 4;
    math::fp32 velocity;

    static constexpr Result<Self, DeserialzeError> 
    from_bytes(const std::span<const uint8_t, FIXED_LENGTH> bytes){ 
        BytesReader reader(bytes);
        return Ok(Self{
            .velocity = math::fp32::from_bits(reader.fetch<uint32_t>())
        });
    }

};


// struct SetPositionSetPoint{
//     using Self = SetPositionSetPoint;
//     static constexpr CommandKind COMMAND = CommandKind::SetPositionSetPoint;
//     fp32 position;
//     bool need_ack;

//     static constexpr Result<Self, DeserialzeError> 
//     from_bytes(std::span<const uint8_t, FIXED_LENGTH> bytes){ 
//         const auto bytes = frame.bytes();
//         if(bytes.size() > 5)
//             return Err(DeserialzeError::DlcTooLong);
//         if(bytes.size() < 5)
//             return Err(DeserialzeError::DlcTooLong);
//         BytesReader reader(bytes);
//         return Ok(Self{
//             .position = reader.fetch<fp32>(),
//             .need_ack = reader.fetch<bool>()
//         });
//     }

//     constexpr CanPayload to_can_payload() const{ 
//         const auto buf = velocity.to_le_bytes();
//         return CanPayload::from_bytes(buf);
//     }
// };
}
}