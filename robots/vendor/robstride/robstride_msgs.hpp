#pragma once

#include "robstride_primitive.hpp"
#include "robots/vendor/mit/mit_primitive.hpp"

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

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes){
        std::fill_n(bytes.begin(), bytes.size(), 0);
    }
};

struct [[nodiscard]] MotionControl final{
    using Self = MotionControl;
    static constexpr Command COMMAND = Command::MotionControl;
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
        const auto x1_bits = x1_code.bits;
        const auto x2_bits = x2_code.bits;
        const auto kp_bits = kp_code.bits;
        const auto kd_bits = kd_code.bits;

        bytes[0] = static_cast<uint8_t>(x1_bits >> 8);
        bytes[1] = static_cast<uint8_t>(x1_bits);
        bytes[2] = static_cast<uint8_t>(x2_bits >> 8);
        bytes[3] = static_cast<uint8_t>(x2_bits);
        bytes[4] = static_cast<uint8_t>(kp_bits >> 8);
        bytes[5] = static_cast<uint8_t>(kp_bits);
        bytes[6] = static_cast<uint8_t>(kd_bits >> 8);
        bytes[7] = static_cast<uint8_t>(kd_bits);
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

struct [[nodiscard]] GetSingleParament final{
    static constexpr Command COMMAND = Command::GetSingleParament;
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

struct [[nodiscard]] SetBaudrate final{
    static constexpr Command COMMAND = Command::SetBaudrate;
    NodeId main_id;
    NodeId motor_id;
    CanBaudrate baudrate;

    [[nodiscard]] constexpr hal::CanExtId can_id() const{
        return CanIdFields(COMMAND, static_cast<uint16_t>(main_id.count), motor_id.count).pack();
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = 0x01;
        bytes[1] = 0x02;
        bytes[2] = 0x03;
        bytes[3] = 0x04;
        bytes[4] = 0x05;
        bytes[5] = 0x06;
        bytes[6] = static_cast<uint8_t>(baudrate);
        bytes[7] = 0x08;
    }
};
}

namespace mit_msgs{
using namespace primitive;

struct [[nodiscard]] EnableMit final{
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | 0);
        return hal::CanStdId::from_u11(id_bits);
    }

    void fill_bytes(std::span<uint8_t, 8> bytes) const {
        std::fill_n(bytes.begin(), bytes.size(), 0xff);
        bytes.back() = 0xfc;
    }
};

struct [[nodiscard]] DisableMit final{
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | 0);
        return hal::CanStdId::from_u11(id_bits);
    }

    void fill_bytes(std::span<uint8_t, 8> bytes) const {
        std::fill_n(bytes.begin(), bytes.size(), 0xff);
        bytes.back() = 0xfd;
    }
};

struct [[nodiscard]] SetMotorType final{
    NodeId motor_id;
    uint8_t motor_type;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | 0);
        return hal::CanStdId::from_u11(id_bits);
    }

    void fill_bytes(std::span<uint8_t, 8> bytes) const {
        std::fill_n(bytes.begin(), bytes.size(), 0xff);
        bytes[6] = motor_type;
        bytes.back() = 0xfc;
    }
};

struct [[nodiscard]] SetMotorId final{
    NodeId motor_id;
    uint8_t f_cmd;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | 0);
        return hal::CanStdId::from_u11(id_bits);
    }

    void fill_bytes(std::span<uint8_t, 8> bytes) const {
        std::fill_n(bytes.begin(), bytes.size(), 0xff);
        bytes[6] = f_cmd;
        bytes.back() = 0x01;
    }
};

struct [[nodiscard]] MitControl final{
    NodeId motor_id;
    mit::MitPositionCode_u16 position;
    mit::MitSpeedCode_u12 speed;
    mit::MitKpCode_u12 kp;
    mit::MitKdCode_u12 kd;
    mit::MitTorqueCode_u12 torque;
    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | 0);
        return hal::CanStdId::from_u11(id_bits);
    }
    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
    };
};

struct [[nodiscard]] MitPositionControl final{
    NodeId motor_id;
    math::fp32 x1_radians;
    math::fp32 x2_radians;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | (1u < 8));
        return hal::CanStdId::from_u11(id_bits);
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        //little endian
        const auto x1_bytes = std::bit_cast<std::array<uint8_t, 4>>(x1_radians.to_bits());
        const auto x2_bytes = std::bit_cast<std::array<uint8_t, 4>>(x2_radians.to_bits());
        std::copy(x1_bytes.begin(), x1_bytes.end(), bytes.begin());
        std::copy(x2_bytes.begin(), x2_bytes.end(), bytes.begin() + 4);
    };
};

struct [[nodiscard]] MitSpeedControl final{
    NodeId motor_id;
    math::fp32 x2_radians;
    math::fp32 current_limit;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | (2u < 8));
        return hal::CanStdId::from_u11(id_bits);
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        //little endian
        const auto x2_bytes = std::bit_cast<std::array<uint8_t, 4>>(x2_radians.to_bits());
        const auto current_bytes = std::bit_cast<std::array<uint8_t, 4>>(current_limit.to_bits());
        std::copy(x2_bytes.begin(), x2_bytes.end(), bytes.begin());
        std::copy(current_bytes.begin(), current_bytes.end(), bytes.begin() + 4);
    };
};

struct [[nodiscard]] SetZeroPosition final{
    NodeId motor_id;

    [[nodiscard]] constexpr hal::CanStdId can_id() const{
        const auto id_bits = static_cast<uint16_t>(motor_id.count | (0));
        return hal::CanStdId::from_u11(id_bits);
    }

    void fill_bytes(std::span<uint8_t, 8> bytes) const {
        std::fill_n(bytes.begin(), bytes.size(), 0xff);
        bytes.back() = 0xfe;
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

struct [[nodiscard]] Feedback final{
    static constexpr Command COMMAND = Command::Feedback;
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