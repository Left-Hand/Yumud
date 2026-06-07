#include "ddsm400_primitive.hpp"

namespace ymd::robots::waveshare::ddsm400{
namespace req_msgs{
using namespace primitive;
// 协议1：驱动电机转动
// 发送到电机：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x64	速度/位置/电流给定高8位	速度/位置/电流给定低8位	0	0	加速时间	刹车	0	CRC8
// 电机反馈：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x65	速度高8位	速度低8位	电流高8位	电流低8位	加速时间	温度	故障码	CRC8

// 加速时间：速度环模式下有效，每1rpm的加速时间，单位为1ms，当设置为1时，每1rpm的加速时间为1ms，当设置为10时，每1rpm的加速时间为 10*1ms=10ms，设置为0时，既默认为1，每1rpm的加速时间为1ms。
// 刹车：0XFF 其他值不刹车，速度环模式下有效。

struct [[nodiscard]] SetTarget final{
    static constexpr ReqCommand COMMAND = ReqCommand::SetTarget;
    SetPointCode setpoint_code;
    AccelerationTimeCode acceleration_time_code;
    Enable brake_en;

    constexpr void fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES> buf) const noexcept {
        buf[0] = static_cast<uint8_t>(setpoint_code.bits >> 8);
        buf[1] = static_cast<uint8_t>(setpoint_code.bits);
        buf[2] = 0;
        buf[3] = 0;
        buf[4] = static_cast<uint8_t>(acceleration_time_code.bits);
        buf[5] = (brake_en == EN) ? 0xff : 0x00;
        buf[6] = 0;
    }
};


// 协议2：获取其它反馈
// 发送到电机：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x74	0	0	0	0	0	0	0	CRC8
// 电机反馈：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x75	里程圈数高8位	里程圈数次高8位	里程圈数次低8位	里程圈数低8位	位置高8位	位置低8位	故障码	CRC8



struct [[nodiscard]] GetJourney final{
    static constexpr ReqCommand COMMAND = ReqCommand::GetJourney;
    constexpr void fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES> buf) const noexcept {
        std::fill(buf.begin(), buf.end(), 0);
    }
};

// 协议3：电机模式切换发送协议
// 发送到电机：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0xA0	模式值	0	0	0	0	0	0	CRC8
// 电机反馈：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0xA1	模式值	0	0	0	0	0	0	CRC8
struct [[nodiscard]] SetLoopMode final{
    static constexpr ReqCommand COMMAND = ReqCommand::SetLoopMode;
    LoopMode loop_mode;
    constexpr void fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES> buf) const noexcept {
        buf[0] = static_cast<uint8_t>(loop_mode);
        std::fill(buf.begin() + 1, buf.end(), 0);
    }
};

// 协议4：电机 ID 设置发送协议
// 发送到电机：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	0xAA	0x55	0x53	ID	0	0	0	0	0	CRC8
// 电机反馈：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x65	0	0	0	0	0	0	0	CRC8
// 注：设置 ID 时请保证上电只插了一个电机，每次上电只允许设置一次，电机接收到 5 次 ID 设置指令后进行设置。


struct [[nodiscard]] SetMotorId final{
    static constexpr ReqCommand COMMAND = ReqCommand::SetMotorId;
    MotorId target_motor_id;
    constexpr void fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES> buf) const noexcept {
        buf[0] = target_motor_id.to_u8();
        std::fill(buf.begin() + 1, buf.end(), 0);
    }
};

// 发送到电机：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x75	0	0	0	0	0	0	0	CRC8
struct [[nodiscard]] GetLoopMode final{
    static constexpr ReqCommand COMMAND = ReqCommand::GetLoopMode;
    constexpr void fill_bytes(std::span<uint8_t, NUM_PAYLOAD_BYTES> buf) const noexcept {
        std::fill(buf.begin(), buf.end(), 0);
    }
};

}

namespace resp_msgs{
using namespace primitive;
struct [[nodiscard]] Feedback final{
    using Self = Feedback;
    static constexpr RespCommand COMMAND = RespCommand::Feedback;

    SpeedCode speed_code;
    CurrentCode current_code;
    AccelerationTimeCode acceleration_time_code;
    TempratureCode temprature_code;
    FaultFlags fault_flags;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, NUM_PAYLOAD_BYTES> buf){
        return Ok(Self{
            .speed_code = SpeedCode{.bits = bytes_to_int_be<int16_t>(buf.subspan<0,2>())},
            .current_code = CurrentCode{.bits = bytes_to_int_be<int16_t>(buf.subspan<2,2>())},
            .acceleration_time_code = AccelerationTimeCode{.bits = buf[4]},
            .temprature_code = TempratureCode{.bits = buf[5]},
            .fault_flags = std::bit_cast<FaultFlags>(buf[6]),
        });
    }
};

// 里程圈数：计圈范围 -2147483467 到 2147483467，重新上电会清 0
// 位置值：0~32767 对应 0~360°

struct [[nodiscard]] Feedback2 final{
    using Self = Feedback2;
    static constexpr RespCommand COMMAND = RespCommand::Feedback2;
    
    int32_t laps_cnt;
    LapAngleCode lap_angle_code;
    FaultFlags fault_flags;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, NUM_PAYLOAD_BYTES> buf){
        return Ok(Self{
            .laps_cnt = bytes_to_int_be<int32_t>(buf.subspan<0,4>()),
            .lap_angle_code = LapAngleCode{.bits = bytes_to_int_be<uint16_t>(buf.subspan<4,2>())},
            .fault_flags = std::bit_cast<FaultFlags>(buf[6]),
        });
    }
};


struct [[nodiscard]] SetLoopMode final{
    using Self = SetLoopMode;
    static constexpr RespCommand COMMAND = RespCommand::SetLoopMode;
    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, NUM_PAYLOAD_BYTES> buf){
        (void)buf; //unused
        return Ok(Self{});
    }
};


// 电机反馈：
// 数据域	DATA[0]	DATA[1]	DATA[2]	DATA[3]	DATA[4]	DATA[5]	DATA[6]	DATA[7]	DATA[8]	DATA[9]
// 内容	ID	0x76	模式值	0	0	0	0	0	0	CRC8

struct [[nodiscard]] GetLoopMode final{
    using Self = GetLoopMode;
    static constexpr RespCommand COMMAND = RespCommand::GetLoopMode;

    enum class [[nodiscard]] RespLoopMode:uint8_t{
        OpenLoop = 0x00,
        CurrentLoop = 0x01,
        SpeedLoop = 0x02,
        PositionLoop = 0x03,
    };

    static constexpr uint8_t MAX_NUM = static_cast<uint8_t>(RespLoopMode::PositionLoop);

    RespLoopMode loop_mode;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, NUM_PAYLOAD_BYTES> buf){
        const auto b = buf[0];
        if(b > MAX_NUM) 
            return Err(DeMsgError::Unnamed);
        return Ok(Self{
            .loop_mode = static_cast<RespLoopMode>(b)
        });
    }
};
}
}