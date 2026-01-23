#pragma once

#include "../uavcan_primitive.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"

// https://cn.tmotor.com/uploadfile/2024/1101/20241101052544211.pdf
// https://www.ckesc.com/wp-content/uploads/2025/03/CKESC-UAVCAN2.1-min.pdf

namespace ymd::uavcan::msgs{
struct StatusFlag{
    enum class [[nodiscard]] Mode:uint8_t{
        Off = 0b000,
        Idle = 0b001,
        SoftStartup = 0b010,
        Running,
        SoftDeacc,
        Error,
        LowSpeedForwardOars,
        LowSpeedReserveOars
    };

    using Self = StatusFlag;
    uint32_t bits;

    constexpr auto is_over_voltage() const {return make_bitfield_proxy<0, 1, bool>(bits);}
    constexpr auto is_under_voltage() const {return make_bitfield_proxy<1, 2, bool>(bits);}
    constexpr auto is_over_current() const {return make_bitfield_proxy<2, 3, bool>(bits);}
    constexpr auto is_throttle_lost() const {return make_bitfield_proxy<3, 4, bool>(bits);}

    constexpr auto is_throttle_abnormal() const {return make_bitfield_proxy<4, 5, bool>(bits);}
    constexpr auto is_mos_over_temperature() const {return make_bitfield_proxy<5, 6, bool>(bits);}
    constexpr auto is_capacitor_over_temperature() const {return make_bitfield_proxy<6, 7, bool>(bits);}
    constexpr auto is_stall() const {return make_bitfield_proxy<7, 8, bool>(bits);}
    constexpr auto is_opa_abnormal() const {return make_bitfield_proxy<8, 9, bool>(bits);}
    constexpr auto is_upper_bridge_abnormal() const {return make_bitfield_proxy<9, 10, bool>(bits);}
    constexpr auto is_lower_bridge_abnormal() const {return make_bitfield_proxy<10, 11, bool>(bits);}
    constexpr auto is_encoder_abnormal() const {return make_bitfield_proxy<11, 12, bool>(bits);}
    constexpr auto mode() const {return make_bitfield_proxy<12, 15, Mode>(bits);}
    constexpr auto encoder_bits() const {return make_bitfield_proxy<16, 31, uint16_t>(bits);}

};

enum class [[nodiscard]] CanBaud:uint8_t{ 
    // 0-1Mbps
    // 1-500Kbps
    // 2-250Kbps
    // 3-125Kbps
    // 4-100Kbps
    // 5-50Kbps

    _1M = 0,
    _500K = 1,
    _250K = 2,
    _125K = 3,
    _100K = 4,
    _50K = 5
};

struct [[nodiscard]] LedSettings final{
    using Self = LedSettings;

    uint16_t red_en:1;
    uint16_t green_en:1;
    uint16_t blue_en:1;
    uint16_t blink_en:1;
    uint16_t blink_freq_bits:12;

    constexpr uq16 blink_freq_hz() const {
        return blink_freq_bits * 0.1_uq16;
    }

    void set_blink_freq_hz(const uq16 freq_hz){ 
        blink_freq_bits = static_cast<uint16_t>(freq_hz * 10);
    }

    [[nodiscard]] static constexpr Self from_bits(const uint16_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }
};

enum class [[nodiscard]] FrameId:uint16_t{
    RawCommand = 0x1030,
    ParamCfg = 0x1033,
    EscStatus = 0x1034,
    PushSci = 0x1038,
    PushCan = 0x1039,
    ParamGet = 0x1332
};

struct [[nodiscard]] ThrottleCode final{
    using Self = ThrottleCode;

    bs14 bits;

    constexpr bool is_zero(){
        return bits.is_zero();
    }

    constexpr bool is_negative(){
        return bits.highest_bit();
    }

    constexpr int16_t to_i16(){
        return std::bit_cast<int16_t>(bits.to_bits());
    }

    static constexpr Self from_i16(const int16_t int_val){
        return Self{bs14::from_bits(int_val)};
    }
};


// RawCommand 发送油门的命令，为广播帧不需要应答，总线上所有ESC 同时接收解析。
// 2：每个油门通道占用14bit ，最高位bit13 为符号位，数字油门范围为-8191~8191 ，0 表示0 油门，8191
// 表示 满油门，协议暂不支持负数字油门，给定负值判断为油门异常，状态位会返回油门异常，错误
// 油门未达到 超时时间则保持上次正常油门，超时则油门会自动归零。
// 3：用户根据实际电调数目情况选择使用N轴RawCommand 数据帧，建议单条CAN通道最多连接8个CAN
// 节点以保证通信质量。
struct RawCommand{
    static constexpr FrameId FRAME_ID = FrameId::RawCommand;
    std::span<const ThrottleCode> throttle_codes;
};

struct ParamCfg final{
    static constexpr FrameId FRAME_ID = FrameId::ParamCfg;
    uint8_t esc_index;
    uint32_t esc_uuid;
    uint16_t esc_id_set;
    uint16_t esc_ov_threshold;
    uint16_t esc_oc_threshold;
    uint16_t esc_ot_threshold;
    uint16_t esc_acc_threshold;
    uint16_t esc_dacc_threshold;
    int16_t esc_rotate_dir;
    uint8_t esc_timing;
    uint8_t esc_signal_priority;
    uint16_t esc_led_mode;
    uint8_t esc_can_rate;
    uint16_t esc_fdb_rate;
    uint8_t esc_save_option;
};


struct EscStatus final{
    static constexpr FrameId FRAME_ID = FrameId::EscStatus;
    StatusFlag status_flag;
    f16 voltage_volts;
    f16 current_amps;
    f16 temperature_celsius;
    bs18 rpm;
    bs7 throttle_percents;
    bs5 esc_id;
};



// 1：ParamGet参数获取返回指令为广播帧不需要应答，总线上所有节点同时接收。
// 2：数据帧中包含ID位，仅飞控或者调参器需要对数据进行解析处理，该帧为参数设置、参数获取的反馈指令，用
// 户可按需选用。
// 3：用户根据实际需求设置发送速率，总线上运行时总帧率应该控制在2400帧(CAN速率1Mbps)以内，非运行时总
// 帧率应该控制在4000帧(CAN速率1Mbps)以内，避免过高的回报速率带来的高总线负荷与处理负荷。
struct [[nodiscard]] ParamGet final{
    static constexpr FrameId FRAME_ID = FrameId::ParamGet;

    // 1字节
    uint8_t tune_id;
    // 4字节（序号2-5）
    uint32_t target_uuid;
    // 2字节（序号6-7）
    uint16_t target_tune_id;
    // 2字节（序号8-9）
    uint16_t overvoltage_threshold;
    // 2字节（序号10-11）
    uint16_t overcurrent_threshold;
    // 2字节（序号12-13）
    uint16_t overtemperature_threshold;
    // 2字节（序号14-15）
    uint16_t acceleration_limit;
    // 2字节（序号16-17）
    uint16_t deceleration_limit;
    // 2字节（序号18-19）
    uint16_t rotation_direction;
    // 1字节（序号20）
    uint8_t step_angle;
    // 2字节（序号21-22）
    uint16_t power_on_count;
    // 4字节（序号23-26）
    uint32_t total_power_on_time;
    // 4字节（序号27-30）
    uint32_t production_date;
    // 4字节（序号31-34）
    uint32_t fault_count;
    // 1字节（序号35）
    uint8_t throttle_priority;
    // 2字节（序号36-37）
    LedSettings led_settings;
    // 1字节（序号38）

    CanBaud can_baud;
    // 2字节（序号39-40）
    uint16_t data_report_rate;
    // 1字节（序号41）
    uint8_t save_option;
    // 32字节（序号42-73）
    union{std::array<uint32_t, 8> __reserved__;};
};
}