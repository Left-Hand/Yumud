#pragma once

#include <cstdint>
// #include "hal/bus/can/can.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "primitive/arithmetic/angle.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/nth.hpp"


namespace ymd::robots::bmkj::m1502e{
namespace primitive{

static constexpr hal::CanBaudrate DEFAULT_CAN_BAUD = hal::CanBaudrate::_500K;


using CanFrame = hal::CanClassicFrame;
using CanId = hal::CanStdId;
using CanPayload = hal::CanClassicPayload;

static constexpr size_t NUM_MAX_MOTORS = 8; 
static constexpr uint16_t NUM_GENERIC_FEEDBACK_CANID_BASE = 0x96;
static constexpr uint8_t CUSTOM_MAGIC_KEY = 0xaa;


//请求的CANID       响应的CANID
//  0x32             0x96   +MOTOR_ID           设置低四个电机的参数
//  0x33             0x96   +MOTOR_ID           设置高四个电机的参数
//  0x105            0x200  +MOTOR_ID           设置环路模式
//  0x106            0x264  +MOTOR_ID           设置反馈策略
//  0x107            0x96   +MOTOR_ID           查询三项内容
//  0x108            0x96   +MOTOR_ID           设置MOTOR_ID
//  0x109            0x390  +MOTOR_ID           设置CAN终端电阻
//  0x10A            0x2C8  +MOTOR_ID           设置通信超时读写操作
//  0x10A            0x32C  +MOTOR_ID           查询固件版本

//可用的ID为(0, 8]分布 
struct [[nodiscard]] MotorId{
    using Self = MotorId;

    static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Option<Self> try_from_nth(const Nth nth){
        if(nth.count() > NUM_MAX_MOTORS)
            return None;
        if(nth.count() <= 0)
            return None;
        return Some(from_bits(static_cast<uint8_t>(nth.count())));
    }

    constexpr uint8_t to_bits() const{
        return bits_;
    }

    constexpr bool operator==(const MotorId& rhs) const{
        return bits_ == rhs.bits_;
    }

    constexpr Nth nth() const {
        return Nth(bits_);
    }
private:
    uint8_t bits_;

    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        return os << self.nth().count();
    }
};

enum class [[nodiscard]] Exception:uint8_t{
    UnderVoltage2 = 0x01, // 电压小于17V
    UnderVoltage1 = 0x02, // 17V - 22V
    OverVoltage = 0x03, // 36V +
    OverCurrent = 0x0A, // 15A +
    OverSpeed = 0x14, //250RPM +
    OverHeat2 = 0x1F, // 120C +
    OverHeat1 = 0x20, // 80C +
    EncoderError = 0x2A,
    EncoderSingalCorrupted = 0x2B,
    CommunicationTimeout = 0x3c,
    Stall = 0x62,
};

struct [[nodiscard]] ExceptionCode{
    using Self = ExceptionCode;

    static constexpr uint8_t NONE = 0x00;

    uint8_t bits;

    constexpr ExceptionCode(const Exception exception) 
        : bits(std::bit_cast<uint8_t>(exception)){;}

    constexpr ExceptionCode(const _None_t) 
        : bits(NONE){;}

    [[nodiscard]] static constexpr Self 
    from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr bool is_none() const {
        return bits == NONE;
    }

    [[nodiscard]] constexpr Exception unwrap() const {
        if(is_none()) 
            __builtin_trap();
        return std::bit_cast<Exception>(bits);
    }


    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits;
    }
private:
    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        if(self.is_none()) [[likely]]
            return os << "None";
        return os << self.unwrap();
    }
};

enum class [[nodiscard]] LoopMode:uint8_t{
    OpenloopVoltage = 0,
    CloseloopCurrent =0x01,
    CloseloopSpeed = 0x02,
    CloseloopPosition = 0x03,
    Disable = 0x09,
    Enable = 0x0A
};

enum class [[nodiscard]] QueryKind:uint8_t{
    Speed = 0x01,
    TorqueCurrent = 0x02,
    Temperature = 0x03,
    Position = 0x04,
    Exception = 0x05,
    NowMode = 0x06
};


enum class [[nodiscard]] DeMsgError:uint8_t{
    ExtendedFrame,
    RemoteFrame,
    DlcTooShort,
    DlcTooLong,
    SetLoopModeNot_0xffx7,
};

struct [[nodiscard]] FeedbackStrategy{
    using Self = FeedbackStrategy;

    uint8_t duration_ms:7;
    uint8_t is_once:1;

    static constexpr Self from_bits(uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_once(){
        return Self{
            .duration_ms = 0,
            .is_once = 1
        };
    }

    static constexpr Option<Self> from_duration(std::chrono::duration<uint8_t, std::milli> duration){
        if(duration.count() > 0x7f) 
            return None;
        return Some(Self{
            .duration_ms = duration.count(), 
            .is_once = 0
        });
    }

    [[nodiscard]] constexpr uint8_t to_bits() const{
        return std::bit_cast<uint8_t>(*this);
    }
};


// 设定值范围-32767~32767，对应-33A~33A
struct [[nodiscard]] CurrentCode{
    using Self = CurrentCode;
    uint16_t bits;

    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    static constexpr Self from_be_bytes(const uint8_t b0, const uint8_t b1){
        const auto bits = (uint16_t(b0) << 8) | uint16_t(b1);
        return Self::from_bits(bits);
    }

    //从安培构造
    static constexpr Result<Self, std::weak_ordering> from_amps(const iq16 amps){
        if(amps > 33) return Err(std::weak_ordering::greater);
        else if (amps < -33) return Err(std::weak_ordering::less);
        const uint16_t bits = std::bit_cast<uint16_t>(int16_t((amps / 33).to_bits()));
        return Ok(from_bits(bits));
    }

    //转为安培
    constexpr iq16 to_amps() const {
        const auto i32_bits = std::bit_cast<int16_t>(bits);
        return iq16::from_bits(i32_bits * 33);
    }
};

//每LSB 0.01RPm
struct [[nodiscard]] SpeedCode{
    using Self = SpeedCode;
    uint16_t bits;

    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    static constexpr Self from_be_bytes(const uint8_t b0, const uint8_t b1){
        const auto bits = (uint16_t(b0) << 8) | uint16_t(b1);
        return Self::from_bits(bits);
    }

    static constexpr Result<Self, std::weak_ordering> from_rpm(const iq16 rpm){
        if(rpm > 210) return Err(std::weak_ordering::greater);
        else if (rpm < -210) return Err(std::weak_ordering::less);
        const uint16_t bits = std::bit_cast<uint16_t>(int16_t((rpm * 100)));
        return Ok(from_bits(bits));
    }

    constexpr iq16 to_rpm() const {
        return iq16(std::bit_cast<int16_t>(bits)) * uq16(0.01);
    }

    constexpr iq16 to_rps() const {
        return iq16(std::bit_cast<int16_t>(bits)) * uq32(0.01 / 60);
    }
};

// 0~32767 对应 0°~360°
struct [[nodiscard]] PositionCode{ 
    using Self = PositionCode;  
    uint16_t bits;

    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    static constexpr Self from_be_bytes(const uint8_t b0, const uint8_t b1){
        const auto bits = (uint16_t(b0) << 8) | uint16_t(b1);
        return Self::from_bits(bits);
    }

    // static constexpr Result<Self, std::weak_ordering> from_angle(const Angle<uq32> angle){
    //     const uint16_t bits = std::bit_cast<uint16_t>(int16_t(angle.to_turns().to_bits() >> (16u + 1u)));
    //     return Ok(from_bits(bits));
    // }

    // constexpr Angle<uq32> to_angle() const {
    //     const auto turns = uq32::from_bits(bits << 1);
    //     return Angle<uq32>::from_turns(turns);
    // }

    // Fixed conversion from angle to position code
    static constexpr Result<Self, std::weak_ordering> from_angle(const Angle<uq32> angle){
        // Convert angle to turns (0-1 for 0°-360°)
        const auto turns = angle.to_turns();
        // Scale 0-1 turns to 0-32767 range
        // Using uq32's full range to map to 15 bits (32768 values)
        const uint32_t scaled = (turns.to_bits() >> 17);
        // Clamp to valid range and cast to uint16_t
        const uint16_t bits = static_cast<uint16_t>(scaled & 0x7FFF);
        return Ok(from_bits(bits));
    }

    constexpr Angle<uq32> to_angle() const {
        // Convert 0-32767 back to 0-1 turns (0°-360°)
        // bits is 15-bit value (0-32767), we need to convert to uq32
        const uint32_t expanded = (static_cast<uint32_t>(bits) << 17);
        const auto turns = uq32::from_bits(expanded);
        return Angle<uq32>::from_turns(turns);
    }
};


struct [[nodiscard]] SetPoint{
    constexpr SetPoint(const CurrentCode code):bits(code.bits){;}
    constexpr SetPoint(const PositionCode code):bits(code.bits){;}
    constexpr SetPoint(const SpeedCode code):bits(code.bits){;}

    constexpr SetPoint(const uint16_t code):bits(code){;}


    uint16_t bits;
};

namespace details{
[[nodiscard]] static constexpr const char * exception_to_str(const Exception e){
    switch(e){
        case Exception::UnderVoltage2: return "UnderVoltage2";
        case Exception::UnderVoltage1: return "UnderVoltage1";
        case Exception::OverVoltage: return "OverVoltage";
        case Exception::OverCurrent: return "OverCurrent";
        case Exception::OverSpeed: return "OverSpeed";
        case Exception::OverHeat2: return "OverHeat2";
        case Exception::OverHeat1: return "OverHeat1";
        case Exception::EncoderError: return "EncoderError";
        case Exception::EncoderSingalCorrupted: return "EncoderSingalCorrupted";
        case Exception::CommunicationTimeout: return "CommunicationTimeout";
        case Exception::Stall: return "Stall";
    }
    return nullptr;
};

[[nodiscard]] static constexpr const char * loopmode_to_str(const LoopMode loop_mode){
    switch(loop_mode){
        case LoopMode::OpenloopVoltage: return "OpenloopVoltage";
        case LoopMode::CloseloopCurrent: return "CloseloopCurrent";
        case LoopMode::CloseloopSpeed: return "CloseloopSpeed";
        case LoopMode::CloseloopPosition: return "CloseloopPosition";
        case LoopMode::Disable: return "Disable";
        case LoopMode::Enable: return "Enable";
    }
    return nullptr;
};

[[nodiscard]] static constexpr const char * querykind_to_str(const QueryKind query_kind){
    switch(query_kind){
        case QueryKind::Exception: return "Exception";
        case QueryKind::NowMode: return "NowMode";
        case QueryKind::Position: return "Position";
        case QueryKind::Speed: return "Speed";
        case QueryKind::Temperature: return "Temperature";
        case QueryKind::TorqueCurrent: return "TorqueCurrent";
    }
    return nullptr;
}
}

}


using namespace primitive;
struct Prelude{
    DEF_FRIEND_DERIVE_DEBUG(DeMsgError)
};


}

namespace ymd{
inline OutputStream & operator <<(
    OutputStream & os, 
    const robots::bmkj::m1502e::primitive::Exception & self
){
    if(const auto str = robots::bmkj::m1502e::primitive::details::exception_to_str(self); 
        str != nullptr) return os << str;
    return os << "Unknown" << os.brackets<'('>() << 
        std::bit_cast<uint8_t>(self) << os.brackets<')'>();
}

inline OutputStream & operator <<(
    OutputStream & os, 
    const robots::bmkj::m1502e::primitive::LoopMode & self
){
    if(const auto str = robots::bmkj::m1502e::primitive::details::loopmode_to_str(self); 
        str != nullptr) return os << str;
    return os << "Unknown" << os.brackets<'('>() << 
        std::bit_cast<uint8_t>(self) << os.brackets<')'>();
}

inline OutputStream & operator <<(
    OutputStream & os, 
    const robots::bmkj::m1502e::primitive::QueryKind & self
){
    if(const auto str = robots::bmkj::m1502e::primitive::details::querykind_to_str(self); 
        str != nullptr) return os << str;
    return os << "Unknown" << os.brackets<'('>() << 
        std::bit_cast<uint8_t>(self) << os.brackets<')'>();
}
}