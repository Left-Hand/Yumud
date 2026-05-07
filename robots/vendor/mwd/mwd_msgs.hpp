#pragma once

#include "mwd_primitive.hpp"
#include "core/utils/bytes/bytes_caster.hpp"
#include "primitive/can/can_id.hpp"
#include "primitive/can/bxcan_payload.hpp"
#include "mwd_utils.hpp"
#include <utility>

namespace ymd::robots::mwd{
using namespace mwd::primitive;

namespace req_msgs{

struct [[nodiscard]] Nop final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::Nop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetZeroToRom final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetZeroToRom;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorStop final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorStop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorEStop final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorEStop;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] BrakeControl final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::BrakeControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] MotorRun final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::MotorRun;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] OpenLoopControl final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::OpenLoopControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] TorqueControl final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::TorqueControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SpeedControl final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SpeedControl;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionMultiLoop1 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionMultiLoop1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionMultiLoop2 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionMultiLoop2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionSingleLoop1 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionSingleLoop1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionSingleLoop2 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionSingleLoop2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionIncrement1 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionIncrement1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] PositionIncrement2 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::PositionIncrement2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};


// REQ[0x9a] 读取电机状态1和错误标志命令（1帧）
struct [[nodiscard]] GetStatus1 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus1;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

// REQ[0x9b] 清除电机错误标志命令
struct [[nodiscard]] ClearError final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::ClearError;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetStatus2 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus2;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetStatus3 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus3;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetEncoder final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetEncoder;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetAngleMulti final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetAngleMulti;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] ClearTurnCount final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::ClearTurnCount;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetAngleSingle final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetAngleSingle;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetAngleManual final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetAngleManual;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] GetParam final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetParam;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};

struct [[nodiscard]] SetParamRam final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::SetParamRam;
    static constexpr size_t PAYLOAD_LENGTH = 0;
};


}


namespace resp{

// RESP[0x9a] 读取电机状态1和错误标志命令（1帧）
struct [[nodiscard]] GetStatus1 final{
    static constexpr CommandKind COMMAND_KIND = CommandKind::GetStatus1;
    static constexpr size_t PAYLOAD_LENGTH = 7;
    PackedStatus1 status;
};


}

template<typename Receiver, typename Msg>
static constexpr Result<void, typename Receiver::Error> 
serialize_rs485_frame_header(
    Receiver & receiver, 
    uint8_t motor_id,
    const Msg & msg
){
    constexpr uint8_t RS485_FRAME_HEADER_TOKEN = 0x3e;
    constexpr CommandKind COMMAND_KIND = Msg::COMMAND_KIND;
    constexpr size_t PAYLOAD_LENGTH = Msg::PAYLOAD_LENGTH;
    if(PAYLOAD_LENGTH >= 256) __builtin_unreachable();

    std::array<uint8_t, 5> buf = {
        RS485_FRAME_HEADER_TOKEN,
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



template<typename Receiver, typename Msg>
static constexpr Result<void, typename Receiver::Error> 
serialize_rs485_frame_payload(
    Receiver & receiver, 
    const Msg & msg
){
    if(const auto res = msg.serialize(receiver);
        res.is_err()) return Err(res.unwrap_err());

    const uint8_t crc_val = WrapAddAccumulator()
        .push_bytes(receiver.collected_bytes())
        .finalize();

    if(const auto res = msg.push_byte(crc_val);
        res.is_err()) return Err(res.unwrap_err());
    
    return Ok();
}

template<typename Receiver, typename Msg>
static constexpr  Result<void, typename Receiver::Error>
serialize_rs485_frame(
    Receiver & receiver, 
    uint8_t motor_id,
    const Msg & msg
){
    if(const auto res = serialize_rs485_frame_header(receiver, motor_id, msg);
        res.is_err()) return Err(res.unwrap_err());

    const auto remaining_receiver = ({
        const auto res = receiver.split_remaining();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    if(const auto res = serialize_rs485_frame_payload(remaining_receiver, msg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

static constexpr hal::CanStdId
serialize_can_frame_id(
    uint8_t motor_id
){
    return hal::CanStdId::from_u11(0x140 + motor_id);
}

template<typename Msg>
static constexpr hal::ClassicCanPayload
serialize_can_frame_payload(
    const Msg & msg
){
    std::array<uint8_t, 8> u8x8;

    return hal::ClassicCanPayload::from_u8x8(u8x8);
}

}