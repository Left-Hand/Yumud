#pragma once 

#include "m1502e_primitive.hpp"

namespace ymd::robots::bmkj::m1502e{
using namespace primitive;
    
struct [[noiscard]] BytesFiller{
    constexpr explicit BytesFiller(std::span<uint8_t> bytes): bytes_(bytes){}

    void push_be_u16(uint16_t value){ 
        if(offset_ + 2 > bytes_.size())
            __builtin_trap();
        bytes_[offset_++] = value >> 8;
        bytes_[offset_++] = value;
    }

    void push_u8(const uint8_t value){
        if(offset_ + 1 > bytes_.size())
            __builtin_trap();
        bytes_[offset_++] = value;
    }

    void push_repeat_u8(const uint8_t value, size_t count){ 
        if(offset_ + count > bytes_.size())
            __builtin_trap();
        for(size_t i = 0; i < count; i++)
            bytes_[offset_++] = value;

    }
private:
    std::span<uint8_t> bytes_;
    size_t offset_ = 0;
};

namespace req_msg{

struct [[nodiscard]] SetLowQuadMotorSetpoint{
    static constexpr uint16_t NUM_CANID = 0x32;
    std::array<SetPoint, 4> setpoints;
    constexpr std::array<uint8_t, 8> to_bytes() const {
        std::array<uint8_t, 8> bytes;
        BytesFiller filler(bytes);
        for(const auto & setpoint: setpoints){
            filler.push_be_u16(setpoint.bits);
        }
        return bytes;
    }
};

struct [[nodiscard]] SetLoopMode{
    using Self = SetLoopMode;

    static constexpr uint16_t NUM_CANID = 0x105;
    std::array<LoopMode, 8> modes;

    constexpr std::array<uint8_t, 8> to_bytes() const{
        std::array<uint8_t, 8> bytes;
        BytesFiller filler(bytes);
        for(const auto & mode: modes){
            filler.push_u8(static_cast<uint8_t>(mode));
        }
        return bytes;
    }
};

struct [[nodiscard]] QueryItems{
    static constexpr uint16_t NUM_CANID = 0x107;

    MotorId motor_id;
    std::array<QueryKind, 3> query_kinds;
    uint8_t custom_key = CUSTOM_MAGIC_KEY;

    constexpr std::array<uint8_t, 8> to_bytes() const{
        std::array<uint8_t, 8> bytes;
        BytesFiller filler(bytes);
        filler.push_u8((motor_id.bits()));
        for(const auto & kind: query_kinds){
            filler.push_u8(static_cast<uint8_t>(kind));
        }
        filler.push_u8(custom_key);
        filler.push_repeat_u8(0xff, 3);
        return bytes;
    }
};
}

// static constexpr CanId generic_feedback

namespace resp_msg{
struct [[nodiscard]] StateFeedback{
    using Self = StateFeedback;
    static constexpr size_t NUM_CANID_BASE = NUM_GENERIC_FEEDBACK_CANID_BASE;

    MotorId motor_id;
    SpeedCode speed;
    CurrentCode current;
    PositionCode position;
    Exception exception;
    LoopMode loop_mode;

    constexpr CanFrame to_can_frame() const{
        const auto canid = CanId::from_u11(
            NUM_GENERIC_FEEDBACK_CANID_BASE + motor_id.nth().count()
        );
        std::array<uint8_t, 8> buf;
        BytesFiller filler(buf);
        filler.push_be_u16(speed.bits);
        filler.push_be_u16(current.bits);
        filler.push_be_u16(position.bits);
        filler.push_u8(static_cast<uint8_t>(exception));
        filler.push_u8(static_cast<uint8_t>(loop_mode));
        return CanFrame::from_parts(
            canid,
            CanPayload::from_u8x8(buf)
        );
    };

    static constexpr Result<Self, DeMsgError> from_can_frame(const CanFrame & frame){
        if(frame.is_extended()) 
            return Err(DeMsgError::ExtendedFrame);
        if(frame.is_remote()) 
            return Err(DeMsgError::RemoteFrame);
        if(frame.payload().size() != 8)
            return Err(DeMsgError::DlcTooShort);

        const auto id_u32 = frame.id_u32();

        if(id_u32 <= 0x96 || id_u32 > (0x96 + 8))
            __builtin_trap();

        const auto bytes = frame.payload_bytes();
        const auto motor_id = MotorId::from_bits(id_u32 - 0x96);

        const auto speed = SpeedCode::from_be_bytes(bytes[0], bytes[1]);
        const auto current = CurrentCode::from_be_bytes(bytes[2], bytes[3]);
        const auto position = PositionCode::from_be_bytes(bytes[4], bytes[5]);

        //TODO use try_from instead
        const auto exception = std::bit_cast<Exception>(bytes[6]);
        const auto loop_mode = std::bit_cast<LoopMode>(bytes[7]);

        return Ok(StateFeedback {
            .motor_id = motor_id,
            .speed = speed,
            .current = current,
            .position = position,
            .exception = exception,
            .loop_mode = loop_mode,
        });
    }
};

struct [[nodiscard]] SetLoopMode{
    using Self = SetLoopMode;
    static constexpr uint16_t NUM_CANID_BASE = 0x200;
    MotorId motor_id;
    LoopMode loop_mode;


    constexpr CanFrame to_can_frame() const{
        const auto canid = CanId::from_u11(
            NUM_CANID_BASE + motor_id.nth().count()
        );
        std::array<uint8_t, 8> buf;
        BytesFiller filler(buf);
        filler.push_u8(static_cast<uint8_t>(loop_mode));
        filler.push_repeat_u8(0xff, 7);

        return CanFrame::from_parts(
            canid,
            CanPayload::from_u8x8(buf)
        );
    };

    static constexpr Result<Self, DeMsgError> from_can_frame(const CanFrame & frame){
        if(frame.is_extended()) 
            return Err(DeMsgError::ExtendedFrame);
        if(frame.is_remote()) 
            return Err(DeMsgError::RemoteFrame);
        if(frame.payload().size() != 8)
            return Err(DeMsgError::DlcTooShort);

        const auto id_u32 = frame.id_u32();

        if(id_u32 <= NUM_CANID_BASE || id_u32 > (NUM_CANID_BASE + 8))
            __builtin_trap();

        const auto bytes = frame.payload().bytes(); 
        const bool is_0xffx7 = [&]{
            for(size_t i = 1; i < 8; i++)
                if(bytes[i] != 0xff) return false;
            return true;
        }();

        if(not is_0xffx7)
            return Err(DeMsgError::SetLoopModeNot_0xffx7);

        const auto motor_id = MotorId::from_bits(id_u32 - 0x96);
        //TODO use try_from instead
        const auto loop_mode = std::bit_cast<LoopMode>(bytes[0]);
        return Ok(SetLoopMode{
            .motor_id = motor_id,
            .loop_mode = loop_mode
        });
    }
};


// CANID和状态反馈帧一样 何意味？让我自己用载荷字段的custom magic key做区分？？
// 我请问，如果被污染的状态反馈帧误撞上查询帧的magic key我怎么知道是哪个帧的响应
// struct [[nodiscard]] QueryItems{
//     using Self = QueryItems ;
//     static constexpr uint16_t NUM_CANID_BASE = 0x96;

//     MotorId motor_id;
//     std::array<uint16_t, 3> values;
//     uint8_t custom_key = 0;

//     static constexpr Result<Self, DeMsgError> from_can_frame(const CanFrame & frame){
//         if(frame.is_extended()) 
//             return Err(DeMsgError::ExtendedFrame);
//         if(frame.is_remote()) 
//             return Err(DeMsgError::RemoteFrame);
//         if(frame.payload().size() != 8)
//             return Err(DeMsgError::DlcTooShort);

//         const auto id_u32 = frame.id_u32();

//         if(id_u32 <= NUM_CANID_BASE || id_u32 > (NUM_CANID_BASE + 8))
//             __builtin_trap();
//     }
// };



}

}