#pragma once

#include "../uavcan_primitive.hpp"
#include "core/utils/bits/bitfield_proxy.hpp"

// https://cn.tmotor.com/uploadfile/2024/1101/20241101052544211.pdf

namespace ymd::uavcan::msgs{

enum class FrameId:uint16_t{
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
    std::span<const ThrottleCode> throttle_codes;
};

struct ParamCfg final{
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

struct StatusFlag{
    enum class Mode:uint8_t{
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


struct EscStatus final{
    StatusFlag status_flag;
    f16 voltage_volts;
    f16 current_amps;
    f16 temperature_celsius;
    bs18 rpm;
    bs7 throttle_percents;
    bs5 esc_id;
};

}