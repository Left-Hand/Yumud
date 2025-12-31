#pragma once

#include "robstride_primitive.hpp"

namespace ymd::robots::robstride{


namespace req_msgs{
using namespace primitive;
struct [[nodiscard]] GetDeviceId final{
    static constexpr Command COMMAND = Command::GetDeviceId;
    NodeId host_id;
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(host_id.count), motor_id.count).pack();
    }
};

struct [[nodiscard]] Conmmunication1 final{
    static constexpr Command COMMAND = Command::CommunicationType1;
    TorqueCode torque_code;
    NodeId motor_id;

    AngleCode x1_code;
    AngularVelocityCode x2_code;
    KpCode kp_code;
    KdCode kd_code;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, torque_code.bits, motor_id.count).pack();
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes){
        //TODO
    }
};

struct [[nodiscard]] EnableRunning final{
    static constexpr Command COMMAND = Command::EnableRunning;
    NodeId main_id;
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(main_id.count), motor_id.count).pack();
    }
};

struct [[nodiscard]] DisableRunning final{
    static constexpr Command COMMAND = Command::DisableRunning;
    NodeId main_id;
    NodeId motor_id;
    bool clear_fault;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(main_id.count), motor_id.count).pack();
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(clear_fault);
        std::fill_n(bytes.begin() + 1, bytes.size() - 1, 0);
    }
};


struct [[nodiscard]] SetMechanicalZero final{
    static constexpr Command COMMAND = Command::SetMechanicalZero;
    NodeId main_id;
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(main_id.count), motor_id.count).pack();
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = 1;
        std::fill_n(bytes.begin() + 1, bytes.size() - 1, 0);
    }
};


struct [[nodiscard]] SetMotorId final{
    static constexpr Command COMMAND = Command::SetMotorId;
    NodeId main_id;
    NodeId motor_id;
    NodeId preset_id;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        const uint16_t arg1 = static_cast<uint16_t>(
            static_cast<uint16_t>(static_cast<uint16_t>(preset_id.count) << 8) 
            | static_cast<uint16_t>(motor_id.count));
        return CanIdFields(COMMAND, arg1, motor_id.count).pack();
    }
};

struct [[nodiscard]] ReadSingleParament final{
    static constexpr Command COMMAND = Command::ReadSingleParament;
    NodeId main_id;
    NodeId motor_id;
    DictKey dict_key;
    DictVal dict_val;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(main_id.count), motor_id.count).pack();
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(static_cast<uint16_t>(dict_key));
        bytes[1] = static_cast<uint8_t>(static_cast<uint16_t>(dict_key) >> 8);
        bytes[2] = 0;
        bytes[3] = 0;
        const auto val_bytes = dict_val.to_bytes();
        std::copy(val_bytes.begin(), val_bytes.end(), bytes.begin() + 4);
    }
};
}

namespace resp_msgs{
using namespace primitive;
struct [[nodiscard]] GetDeviceId final{
    using Self = GetDeviceId;
    static constexpr Command COMMAND = Command::GetDeviceId;
    NodeId motor_id;
    uint64_t serial_number;

    constexpr Result<Self, DeMsgError> try_from_can_frame(const hal::BxCanFrame & frame){
        if(frame.length() != 8) return Err(DeMsgError::PayloadNot8Bytes);
        if(frame.is_standard()) return Err(DeMsgError::StdFrame);
        const auto depack = ({
            const auto either_depack = CanIdFields::try_from(frame.identifier().to_extid());
            if(either_depack.is_err()) return Err(either_depack.unwrap_err());
            either_depack.unwrap();
        });
        if(depack.arg2 != 0xfe) return Err(DeMsgError::Arg2Not0xfe);
        return Ok(Self{
            .motor_id = NodeId{
                .count = static_cast<uint8_t>(depack.arg1)
            },
            .serial_number = frame.payload_u64(),
        });
    }
};

struct [[nodiscard]] Conmmunication2 final{
    static constexpr Command COMMAND = Command::CommunicationType2;
    FaultFlags fault_flags;
    NodeId motor_id;
    NodeId host_id;

    AngleCode x1_code;
    AngularVelocityCode x2_code;
    TorqueCode torque_code;
    TemperatureCode temperature_code;
};
}

}