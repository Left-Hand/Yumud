#pragma once

#include "core/utils/Option.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/bits/bits_caster.hpp"

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
    SetPositionSetPoint = 0x04
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
    Mode request_mode;

    static constexpr Result<Self, DeserialzeError> 
    from_can_frame(const CanFrame& frame){ 
        const auto payload_bytes = frame.payload_bytes();
        if(payload_bytes.size() > 4)
            return Err(DeserialzeError::DlcTooLong);
        if(payload_bytes.size() < 4)
            return Err(DeserialzeError::DlcTooLong);
        const bool en = payload_bytes[0] == 1;
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
    fp32 current;

    static constexpr Result<Self, DeserialzeError> 
    from_can_frame(const CanFrame& frame){ 
        const auto payload_bytes = frame.payload_bytes();
        if(payload_bytes.size() > 4)
            return Err(DeserialzeError::DlcTooLong);
        if(payload_bytes.size() < 4)
            return Err(DeserialzeError::DlcTooLong);
        BytesReader reader(payload_bytes);
        return Ok(Self{
            .current = reader.fetch<fp32>()
        });
    }

    constexpr CanPayload to_can_payload() const{ 
        const auto buf = current.to_le_bytes();
        return CanPayload::from_bytes(buf);
    }
};

struct SetVelocitySetPoint{
    using Self = SetVelocitySetPoint;
    static constexpr CommandKind COMMAND = CommandKind::SetCurrentSetPoint;
    fp32 velocity;

    static constexpr Result<Self, DeserialzeError> 
    from_can_frame(const CanFrame& frame){ 
        const auto payload_bytes = frame.payload_bytes();
        if(payload_bytes.size() > 4)
            return Err(DeserialzeError::DlcTooLong);
        if(payload_bytes.size() < 4)
            return Err(DeserialzeError::DlcTooLong);
        BytesReader reader(payload_bytes);
        return Ok(Self{
            .velocity = reader.fetch<fp32>()
        });
    }

    constexpr CanPayload to_can_payload() const{ 
        const auto buf = velocity.to_le_bytes();
        return CanPayload::from_bytes(buf);
    }
};


// struct SetPositionSetPoint{
//     using Self = SetPositionSetPoint;
//     static constexpr CommandKind COMMAND = CommandKind::SetPositionSetPoint;
//     fp32 position;
//     bool need_ack;

//     static constexpr Result<Self, DeserialzeError> 
//     from_can_frame(const CanFrame& frame){ 
//         const auto payload_bytes = frame.payload_bytes();
//         if(payload_bytes.size() > 5)
//             return Err(DeserialzeError::DlcTooLong);
//         if(payload_bytes.size() < 5)
//             return Err(DeserialzeError::DlcTooLong);
//         BytesReader reader(payload_bytes);
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