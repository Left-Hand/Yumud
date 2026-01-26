#pragma once


#include "../uavcan_primitive.hpp"
#include "uavcan_msgs.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"

// https://cn.tmotor.com/uploadfile/2024/1101/20241101052544211.pdf

namespace ymd::uavcan::msgs{
enum class EscId:uint8_t{

};

[[nodiscard]] static constexpr uint8_t calc_sum(const uint8_t last, std::span<const uint8_t> bytes){
    uint32_t sum = last;

    #pragma GCC unroll 8
    for(size_t i = 0; i < bytes.size(); i++){
        sum += bytes[i];
    }
    return static_cast<uint8_t>(sum);
}

static constexpr auto PUSHSCI_SIGNATURE = Signature{0xCE2B6D6B6BDC0AE8};

namespace pushsci{ 



enum class [[nodiscard]] PushSciFrameId:uint8_t{
    ReqSetZero = 0x09,
    ReqSetFocParaments = 0x1e,
    ReqGetFocParaments = 0x1a,
    RespControlFrame = 0x06
};

struct [[nodiscard]] PushSciFrameHeader final{
    uint8_t frame_ttl;
    EscId esc_id;

    template<typename Receiver>
    constexpr void sink_to(Receiver & receiver){
        receiver.push_u8(frame_ttl);
        receiver.push_u8(static_cast<uint8_t>(esc_id));
    }
};

template<typename Receiver, typename Payload>
constexpr void serialize_pushsci_frame(Receiver & receiver, const PushSciFrameHeader header, Payload & payload){
    receiver.push_u8(0xec);
    receiver.push_u8(0x96);
    receiver.push_u8(static_cast<uint8_t>(Payload::FRAME_ID));
    header.sink_to(receiver);
    receiver.push_u8(payload.frame_length());
    payload.sink_to(receiver);
    const auto verify_byte = calc_sum(0, receiver.received_bytes());
    receiver.push_u8(verify_byte);
}

struct [[nodiscard]] SetZero final{
    static constexpr PushSciFrameId FRAME_ID = PushSciFrameId::ReqSetZero;
    
    static consteval uint8_t frame_length() {
        return 7;
    }

    template<typename Receiver>
    constexpr void sink_to(Receiver & receiver){
        //nothing to do
    }
};


struct [[nodiscard]]  SetFocParaments final{
    static constexpr PushSciFrameId FRAME_ID = PushSciFrameId::ReqSetFocParaments;

    // 7-8字节：电调ID
    uint16_t esc_id;
    // 9-10字节：过压保护阈值
    uint16_t overvoltage_protection_threshold;
    // 11-12字节：过流保护阈值
    uint16_t overcurrent_protection_threshold;
    // 13-14字节：过温保护阈值
    uint16_t overtemperature_protection_threshold;
    // 15-16字节：鸣叫音量
    uint16_t beep_volume;
    // 17-18字节：加速度限制
    uint16_t acceleration_limit;
    // 19-20字节：减速限制
    uint16_t deceleration_limit;
    // 21-22字节：旋转方向
    uint16_t rotation_direction;
    // 23-24字节：油门优先级（0:PWM / 1:CAN）
    uint16_t throttle_priority;
    // 25-26字节：LED设置（0-2bit:RGB灯开关；3bit:闪烁开关；4-15bit:频率）
    uint16_t led_setting;
    // 27字节：总线速率（0-4对应不同速率）
    uint8_t bus_speed;
    // 28-29字节：数据回报速率（0-400HZ）
    uint16_t data_report_rate;
    // 30字节：保存选项（0:临时 / 1:永久）
    uint8_t save_option;


    static consteval uint8_t frame_length() {
        return 0x1f;
    }

    template<typename Receiver>
    void sink_to(Receiver & receiver){
        //nothing to do
    }
};

struct [[nodiscard]] GetFocParaments final{
    static constexpr PushSciFrameId FRAME_ID = PushSciFrameId::ReqGetFocParaments;
    
    static consteval uint8_t frame_length() {
        return 7;
    }

    template<typename Receiver>
    void sink_to(Receiver & receiver){
        //nothing to do
    }
};

struct [[nodiscard]] ControlFrame final{
    static constexpr PushSciFrameId FRAME_ID = PushSciFrameId::RespControlFrame;

    enum class ControlMode:uint8_t{

    };

    ControlMode control_mode;
    uint16_t value;
    uint16_t __resv__;


    static consteval uint8_t frame_length() {
        return 0x0c;
    }


    template<typename Receiver>
    void sink_to(Receiver & receiver){
        receiver.push_u8(static_cast<uint8_t>(control_mode));
        receiver.push_u16(value);
        receiver.push_u16(__resv__);
    }
};


}

namespace pushcan{

static constexpr auto PUSHCAN_SIGNATURE = Signature{0xAACF9B4B2577BC6E};

enum class [[nodiscard]] PushCanFrameId:uint8_t{
    PowerUnitState = 0x15,
    FocParamentResponse = 0x1e
};

struct [[nodiscard]] PushCanFrameHeader final{
    uint8_t frame_ttl;
    EscId esc_id;

    template<typename Receiver>
    void sink_to(Receiver & receiver){
        receiver.push_u8(frame_ttl);
        receiver.push_u8(static_cast<uint8_t>(esc_id));
    }
};

template<typename Receiver, typename Payload>
void serialize_pushcan_frame(Receiver & receiver, const PushCanFrameHeader header, Payload & payload){
    receiver.push_u8(0x7b);
    receiver.push_u8(0x8c);
    receiver.push_u8(static_cast<uint8_t>(Payload::FRAME_ID));
    header.sink_to(receiver);
    receiver.push_u8(payload.frame_length());
    payload.sink_to(receiver);
    const auto verify_byte = calc_sum(0, receiver.received_bytes());
    receiver.push_u8(verify_byte);
}


struct [[nodiscard]] PowerUnitState final{
    static constexpr PushCanFrameId FRAME_ID = PushCanFrameId::PowerUnitState;
    // 7字节：动力单机系统状态（0x00:非锁桨/0x11:正常锁桨/0xCC:系统故障）
    uint8_t power_unit_system_state;
    // 8-9字节：电机当前位置（0-360°，分辨率0.01°）
    uint16_t motor_current_position;
    // 10-11字节：接收PWM值（分辨率0.1us）
    uint16_t received_pwm_value;
    // 12字节：实际输出油门值（分辨率0.4%）
    uint8_t actual_output_throttle_value;
    // 13-14字节：电机转速（分辨率1rpm）
    int16_t motor_speed;
    // 15-16字节：电压（分辨率0.01V）
    int16_t voltage;
    // 17-18字节：电流（分辨率0.01A）
    int16_t current;
    // 19-20字节：温度（分辨率0.01℃）
    uint16_t temperature;
    // 21-22字节：电机错误（错误位数据）
    uint16_t motor_error;
    // 23字节：电机状态（固定值 0x00）
    uint8_t motor_state;
    // 24-25字节：上调上电次数（单位：次）
    uint16_t power_on_count;
    // 26-27字节：电调运行时间（单位：秒）
    uint16_t esc_running_time;


    static consteval uint8_t frame_length() {
        return 0x1c;
    }


    template<typename Receiver>
    void sink_to(Receiver & receiver){
        //nothing to do
    }
};


// 电调配置通信帧结构体
struct [[nodiscard]] FocParamentResponse final {
    static constexpr PushCanFrameId FRAME_ID = PushCanFrameId::FocParamentResponse;

    // 7-22字节：电调硬件版本（16字节版本号）
    std::span<const uint8_t> esc_hardware_version;
    // 23-38字节：电调软件版本（16字节版本号）
    std::span<const uint8_t> esc_software_version;
    // 39-40字节：加速度限制
    uint16_t acceleration_limit;
    // 41-42字节：减速限制
    uint16_t deceleration_limit;
    // 43-44字节：旋转方向
    uint16_t rotation_direction;
    // 45-46字节：位置环使能标志
    uint16_t position_loop_enable_flag;
    // 47-48字节：油门优先级（0:PWM/1:CAN）
    uint16_t throttle_priority;
    // 49-50字节：LED设置（0-2bit:RGB灯开关；3bit:闪烁开关；4-15bit:频率）
    LedSettings led_setting;
    // 51字节：总线速率（0:100Kbps/1:500Kbps/2:250Kbps/3:125Kbps）
    CanBaud can_buad;
    // 52-53字节：数据回报速率（0-400HZ）
    uint16_t data_report_rate;
    // 54字节：保存选项（0:临时/1:永久）
    uint8_t save_option;

    static consteval uint8_t frame_length() {
        return 0x37;
    }


    template<typename Receiver>
    void sink_to(Receiver & receiver){
        //nothing to do
    }

};
}

}