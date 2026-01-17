#pragma once 

#include "m1502e_primitive.hpp"
#include "m1502e_utils.hpp"

namespace ymd::robots::bmkj::m1502e{
using namespace primitive;


namespace req_msgs{

struct [[nodiscard]] SetLowQuadMotorSetpoint final{
    static constexpr uint16_t NUM_CANID = 0x32;
    std::array<SetPoint, 4> setpoints;
    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        BytesFiller filler(bytes);
        for(const auto & setpoint: setpoints){
            filler.push_be_u16(setpoint.bits);
        }
    }
};

struct [[nodiscard]] SetLoopMode final{
    using Self = SetLoopMode;

    static constexpr uint16_t NUM_CANID = 0x105;
    std::array<LoopMode, 8> modes;

    static constexpr Self from_all(const LoopMode mode){
        std::array<LoopMode, 8> modes;
        modes.fill(mode);
        return Self{.modes = modes};
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        BytesFiller filler(bytes);
        for(const auto & mode: modes){
            filler.push_u8(static_cast<uint8_t>(mode));
        }
    }

};

struct [[nodiscard]] SetFeedbackStrategy final{
    using Self = SetFeedbackStrategy;

    static constexpr uint16_t NUM_CANID = 0x106;
    std::array<FeedbackStrategy, 8> strategies;

    static constexpr Self from_all(const FeedbackStrategy strategy){
        std::array<FeedbackStrategy, 8> strategies;
        strategies.fill(strategy);
        return Self{.strategies = strategies};
    }

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        BytesFiller filler(bytes);
        for(const auto & strategy: strategies){
            filler.push_u8(strategy.to_bits());
        }
    }


};

struct [[nodiscard]] QueryItems final{
    static constexpr uint16_t NUM_CANID = 0x107;

    MotorId motor_id;
    std::array<QueryKind, 3> query_kinds;
    uint8_t custom_key = CUSTOM_MAGIC_KEY;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        BytesFiller filler(bytes);
        filler.push_u8((motor_id.to_bits()));
        for(const auto & kind: query_kinds){
            filler.push_u8(static_cast<uint8_t>(kind));
        }
        filler.push_u8(custom_key);
        filler.push_repeat_u8(0xff, 3);
    }

};

struct [[nodiscard]] SetMotorId final{
    static constexpr uint16_t NUM_CANID = 0x108;
    static constexpr uint8_t RESVERED_BYTE_CONTEXT = 0xff;
    MotorId motor_id;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        bytes[0] = motor_id.to_bits();
        std::fill(bytes.begin() + 1, bytes.end(), RESVERED_BYTE_CONTEXT);
    }
};

struct [[nodiscard]] QueryFirmwareVersion final{
    static constexpr uint16_t NUM_CANID = 0x10B;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        std::fill(bytes.begin(), bytes.end(), 0);
    }
};

}


namespace resp_msgs{
struct [[nodiscard]] StateFeedback{
    using Self = StateFeedback;
    static constexpr size_t NUM_CANID_BASE = NUM_GENERIC_FEEDBACK_CANID_BASE;

    MotorId motor_id;
    SpeedCode speed;
    CurrentCode current;
    PositionCode lap_position;
    ExceptionCode exception_code;
    LoopMode loop_mode;

    constexpr CanFrame to_can_frame() const{
        const auto can_id = CanId::from_u11(
            NUM_GENERIC_FEEDBACK_CANID_BASE + motor_id.nth().count()
        );
        std::array<uint8_t, 8> buf;
        BytesFiller filler(buf);
        filler.push_be_u16(speed.bits);
        filler.push_be_u16(current.bits);
        filler.push_be_u16(lap_position.bits);
        filler.push_u8(std::bit_cast<uint8_t>(exception_code));
        filler.push_u8(std::bit_cast<uint8_t>(loop_mode));
        return CanFrame::from_parts(
            can_id,
            CanPayload::from_u8x8(buf)
        );
    };

    static constexpr Result<Self, DeMsgError> try_from_can_frame(const CanFrame & frame){
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
        const auto lap_position = PositionCode::from_be_bytes(bytes[4], bytes[5]);

        //TODO use try_from instead
        const auto exception_code = std::bit_cast<ExceptionCode>(bytes[6]);
        const auto loop_mode = std::bit_cast<LoopMode>(bytes[7]);

        return Ok(StateFeedback {
            .motor_id = motor_id,
            .speed = speed,
            .current = current,
            .lap_position = lap_position,
            .exception_code = exception_code,
            .loop_mode = loop_mode,
        });
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os 
            << os.field("motor_id")(self.motor_id) << os.splitter()
            << os.field("speed(rpm)")(self.speed.to_rpm()) << os.splitter()
            << os.field("current(A)")(self.current.to_amps()) << os.splitter()
            << os.field("lap_position(rad)")(self.lap_position.to_angle().cast_inner<uq16>().to_radians()) << os.splitter()
            << os.field("exception_code")(self.exception_code) << os.splitter()
            << os.field("loop_mode")(self.loop_mode)
        ;
    }
};

struct [[nodiscard]] SetLoopMode{
    using Self = SetLoopMode;
    static constexpr uint16_t NUM_CANID_BASE = 0x200;
    MotorId motor_id;
    LoopMode loop_mode;


    constexpr CanFrame to_can_frame() const{
        const auto can_id = CanId::from_u11(
            NUM_CANID_BASE + motor_id.nth().count()
        );
        std::array<uint8_t, 8> buf;
        BytesFiller filler(buf);
        filler.push_u8(static_cast<uint8_t>(loop_mode));
        filler.push_repeat_u8(0xff, 7);

        return CanFrame::from_parts(
            can_id,
            CanPayload::from_u8x8(buf)
        );
    };

    static constexpr Result<Self, DeMsgError> try_from_can_frame(const CanFrame & frame){
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

//     static constexpr Result<Self, DeMsgError> try_from_can_frame(const CanFrame & frame){
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

struct [[nodiscard]] QueryFirmwareVersion{
    using Self = QueryFirmwareVersion;
    static constexpr uint16_t NUM_CANID_BASE = 0x200;
    MotorId motor_id;
    uint8_t software_major;
    uint8_t software_minor;
    uint8_t hardware_major;
    uint8_t hardware_minor;
    uint8_t year;
    uint8_t month;
    uint8_t day;


    static constexpr Result<Self, DeMsgError> try_from_can_frame(const CanFrame & frame){
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

        const auto motor_id = MotorId::from_bits(id_u32 - NUM_CANID_BASE);
        
        return Ok(Self{
            .motor_id = motor_id,
            .software_major = bytes[1],
            .software_minor = bytes[2],
            .hardware_major = bytes[3],
            .hardware_minor = bytes[4],
            .year = bytes[5],
            .month = bytes[6],
            .day = bytes[7]
        });
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os 
            << os.field("software_major")(self.software_major) << os.splitter()
            << os.field("software_minor")(self.software_minor) << os.splitter()
            << os.field("hardware_major")(self.hardware_major) << os.splitter()
            << os.field("hardware_minor")(self.hardware_minor) << os.splitter()
            << os.field("year")(self.year) << os.splitter()
            << os.field("month")(self.month) << os.splitter()
            << os.field("day")(self.day)
        ;
    }
};

}

}