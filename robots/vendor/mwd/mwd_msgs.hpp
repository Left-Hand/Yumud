#pragma once

#include "mwd_primitive.hpp"
#include "core/utils/bytes/bytes_caster.hpp"
#include "mwd_utils.hpp"
#include <utility>

namespace ymd::robots::mwd{
using namespace mwd::primitive;

namespace req_msgs{

struct [[nodiscard]] Nop{
    static constexpr CommandKind COMMAND_KIND = CommandKind::Nop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetZeroToRom{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetZeroToRom;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorStop{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorStop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorEStop{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorEStop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] BrakeControl{
    static constexpr CommandKind COMMAND_KIND = CommandKind::BrakeControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorRun{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorRun;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] OpenLoopControl{
    static constexpr CommandKind COMMAND_KIND = CommandKind::OpenLoopControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] TorqueControl{
    static constexpr CommandKind COMMAND_KIND = CommandKind::TorqueControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SpeedControl{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SpeedControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionMultiLoop1{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionMultiLoop1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionMultiLoop2{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionMultiLoop2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionSingleLoop1{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionSingleLoop1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionSingleLoop2{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionSingleLoop2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionIncrement1{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionIncrement1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionIncrement2{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionIncrement2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};


// REQ[0x9a] 读取电机状态1和错误标志命令（1帧）
struct [[nodiscard]] GetStatus1{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

// REQ[0x9b] 清除电机错误标志命令
struct [[nodiscard]] ClearError{
    static constexpr CommandKind COMMAND_KIND = CommandKind::ClearError;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetStatus2{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetStatus3{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus3;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetEncoder{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetEncoder;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetAngleMulti{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetAngleMulti;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] ClearTurnCount{
    static constexpr CommandKind COMMAND_KIND = CommandKind::ClearTurnCount;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetAngleSingle{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetAngleSingle;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetAngleManual{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetAngleManual;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetParam{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetParam;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetParamRam{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetParamRam;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};


}


namespace resp{

}

template<typename Receiver, typename Msg>
Result<void, typename Receiver::Error> 
serialize_rs485_msg_header(
    Receiver && receiver, 
    uint8_t motor_id,
    const Msg & msg
){
    static constexpr uint8_t HEADER = 0x3e;
    const CommandKind COMMAND_KIND = Msg::COMMAND_KIND;
    const size_t PAYLOAD_LENGTH = Msg::PAYLOAD_LENGTH;
    if(PAYLOAD_LENGTH >= 256) __builtin_unreachable();

    std::array<uint8_t, 5> buf = {
        HEADER,
        static_cast<uint8_t>(COMMAND_KIND),
        static_cast<uint8_t>(motor_id),
        static_cast<uint8_t>(PAYLOAD_LENGTH),
        0 //crc preserved
    };

    const uint8_t crc_val = WrapAddAccumulator()
        .push_bytes(std::span(buf.begin(), std::prev(buf.end())))
        .finalize();

    buf.back() = crc_val;

    if(const auto res = receiver.push_bytes(std::span(buf));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
};

}