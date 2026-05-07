#pragma once

#include "lkmotor_primitive.hpp"


namespace ymd::robots::lkmotor{

static constexpr std::array<uint8_t, 8> make_head_and_0x00(const uint8_t head){
    alignas(4) std::array<uint8_t, 8> buf;
    if(std::is_constant_evaluated()){
        buf.fill(0);
        buf[0] = head;
    }else{
        auto p = reinterpret_cast<uint32_t *>(buf.data());
        p[0] = static_cast<uint32_t>(head);
        p[1] = 0;
    }

    return buf;
}


struct [[nodiscard]] EncoderPosition final{
    uint16_t bits;        // 编码器位置
};


struct [[nodiscard]] EncoderRaw final{
    uint16_t bits;             // 原始位置
};


struct [[nodiscard]] EncoderOffset final{
    uint16_t bits;              // 零偏
};


struct [[nodiscard]] PhaseCurrent final{
    int16_t phase_a_raw;          // A相电流
    int16_t phase_b_raw;          // B相电流
    int16_t phase_c_raw;          // C相电流
};


namespace req_msgs{
// 抱闸器控制和状态读取命令
struct [[nodiscard]] ManipulateBraker final{
    static constexpr Command COMMAND = Command::ManipulateBraker;

    BrakerCommand braker_command;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ManipulateBraker));
        buf[1] = static_cast<uint8_t>(braker_command);
        return buf;
    }
};


// 开环控制命令
struct [[nodiscard]] OpenloopControl final{
    static constexpr Command COMMAND = Command::OpenloopControl;

    PowerCode power_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::OpenloopControl));
        buf[4] = static_cast<uint8_t>(power_code.bits & 0xFF);
        buf[5] = static_cast<uint8_t>(power_code.bits >> 8);
        return buf;
    }
};


// 转矩闭环控制命令
struct [[nodiscard]] TorqueClosedLoopControl final{
    static constexpr Command COMMAND = Command::TorqueClosedLoopControl;

    TorqueCode torque_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::TorqueClosedLoopControl));
        buf[4] = static_cast<uint8_t>(torque_code.bits & 0xFF);
        buf[5] = static_cast<uint8_t>(torque_code.bits >> 8);
        return buf;
    }
};


// 速度闭环控制命令
struct [[nodiscard]] SpeedClosedLoopControl final{
    static constexpr Command COMMAND = Command::SpeedClosedLoopControl;

    SpeedLimCode torque_limit_code;
    SpeedCode speed_control_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::SpeedClosedLoopControl),
            0x00, // NULL
            static_cast<uint8_t>(torque_limit_code.bits & 0xFF),
            static_cast<uint8_t>(torque_limit_code.bits >> 8),
            static_cast<uint8_t>(speed_control_code.bits & 0xFF),
            static_cast<uint8_t>((speed_control_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((speed_control_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(speed_control_code.bits >> 24)
        };

        return buf;
    }
};


// 多圈位置闭环控制命令1
struct [[nodiscard]] MultiTurnPosClosedLoopControl1 final{
    static constexpr Command COMMAND = Command::MultiTurnPosClosedLoopControl1;

    AngleCode angle_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::MultiTurnPosClosedLoopControl1),
            0x00, // NULL
            0x00, // NULL
            0x00, // NULL
            static_cast<uint8_t>(angle_code.bits & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(angle_code.bits >> 24)
        };

        return buf;
    }
};


// 多圈位置闭环控制命令2
struct [[nodiscard]] MultiTurnPosClosedLoopControl2 final{
    static constexpr Command COMMAND = Command::MultiTurnPosClosedLoopControl2;

    SpeedLimCode max_speed_code;
    AngleCode angle_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::MultiTurnPosClosedLoopControl2),
            0x00, // NULL
            static_cast<uint8_t>(max_speed_code.bits & 0xFF),
            static_cast<uint8_t>(max_speed_code.bits >> 8),
            static_cast<uint8_t>(angle_code.bits & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(angle_code.bits >> 24)
        };

        return buf;
    }
};


// 单圈位置闭环控制命令1
struct [[nodiscard]] SoloTurnPosClosedLoopControl1 final{
    static constexpr Command COMMAND = Command::SoloTurnPosClosedLoopControl1;

    bool is_ccw; // 0x00顺时针，0x01逆时针
    AngleCode angle_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::SoloTurnPosClosedLoopControl1),
            static_cast<uint8_t>(is_ccw ? 0x01 : 0x00),
            0x00, // NULL
            0x00, // NULL
            static_cast<uint8_t>(angle_code.bits & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(angle_code.bits >> 24)
        };

        return buf;
    }
};


// 单圈位置闭环控制命令2
struct [[nodiscard]] SoloLapPositionControl2 final{
    static constexpr Command COMMAND = Command::SoloLapPositionControl2;

    bool is_ccw; // 0x00顺时针，0x01逆时针
    SpeedLimCode speed_lim_code;
    AngleCode angle_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::SoloLapPositionControl2),
            static_cast<uint8_t>(is_ccw ? 0x01 : 0x00),
            static_cast<uint8_t>(speed_lim_code.bits & 0xFF),
            static_cast<uint8_t>(speed_lim_code.bits >> 8),

            static_cast<uint8_t>(angle_code.bits & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((angle_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(angle_code.bits >> 24)
        };

        return buf;
    }
};


// 增量位置闭环控制命令1
struct [[nodiscard]] IncrementalPosClosedLoopControl1 final{
    static constexpr Command COMMAND = Command::IncrementalPosClosedLoopControl1;

    AngleDeltaCode delta_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::IncrementalPosClosedLoopControl1),
            0x00, // NULL
            0x00, // NULL
            0x00, // NULL
            static_cast<uint8_t>(delta_code.bits & 0xFF),
            static_cast<uint8_t>((delta_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((delta_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(delta_code.bits >> 24)
        };

        return buf;
    }
};


// 增量位置闭环控制命令2
struct [[nodiscard]] IncrementalPosClosedLoopControl2 final{
    static constexpr Command COMMAND = Command::IncrementalPosClosedLoopControl2;

    SpeedLimCode speed_lim_code;
    AngleDeltaCode delta_code;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = std::array<uint8_t, 8>{
            static_cast<uint8_t>(Command::IncrementalPosClosedLoopControl2),
            0x00, // NULL
            static_cast<uint8_t>(speed_lim_code.bits & 0xFF),
            static_cast<uint8_t>(speed_lim_code.bits >> 8),
            static_cast<uint8_t>(delta_code.bits & 0xFF),
            static_cast<uint8_t>((delta_code.bits >> 8) & 0xFF),
            static_cast<uint8_t>((delta_code.bits >> 16) & 0xFF),
            static_cast<uint8_t>(delta_code.bits >> 24)
        };

        return buf;
    }
};


// 读取控制参数命令
struct [[nodiscard]] ReadParam final{
    static constexpr Command COMMAND = Command::ReadParam;

    ParamId param_id;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ReadParam));
        buf[1] = static_cast<uint8_t>(param_id);
        return buf;
    }
};


// 写入控制参数命令
struct [[nodiscard]] WriteParam final{
    static constexpr Command COMMAND = Command::WriteParam;

    ParamId param_id;
    std::array<uint8_t, 6> param_value;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::WriteParam));
        buf[1] = static_cast<uint8_t>(param_id);
        for(int i = 0; i < 6; ++i) {
            buf[i + 2] = param_value[i];
        }
        return buf;
    }
};

// 读取设定参数命令
struct [[nodiscard]] ReadSettingParam final{
    static constexpr Command COMMAND = Command::ReadSettingParam;

    SettingParamId setting_param_id;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ReadSettingParam));
        buf[1] = static_cast<uint8_t>(static_cast<uint16_t>(setting_param_id) & 0xFF);
        buf[2] = static_cast<uint8_t>(static_cast<uint16_t>(setting_param_id) >> 8);
        return buf;
    }
};


// 写入设定参数命令
struct [[nodiscard]] WriteSettingParam final{
    static constexpr Command COMMAND = Command::WriteSettingParam;

    SettingParamId setting_param_id;
    std::array<uint8_t, 5> param_data;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::WriteSettingParam));
        buf[1] = static_cast<uint8_t>(static_cast<uint16_t>(setting_param_id) & 0xFF);
        buf[2] = static_cast<uint8_t>(static_cast<uint16_t>(setting_param_id) >> 8);
        for(int i = 0; i < 5; ++i) {
            buf[i + 3] = param_data[i];
        }
        return buf;
    }
};


// 保存设定参数命令
struct [[nodiscard]] SaveSettingParam final{
    static constexpr Command COMMAND = Command::SaveSettingParam;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        alignas(4) auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::SaveSettingParam));
        buf[1] = 0x05;
        buf[2] = 0xfa;
        return buf;
    }
};


// 电机重启命令
struct [[nodiscard]] Reboot final{
    static constexpr Command COMMAND = Command::Reboot;

    constexpr std::array<uint8_t, 8> to_u8x8() const {
        return make_head_and_0x00(static_cast<uint8_t>(Command::Reboot));
    }
};


}



struct KpCode{
    uint16_t bits;
};

struct KiCode{
    uint16_t bits;
};

struct KdCode{
    uint16_t bits;
};



#if 0
namespace resp_msgs {

// 读取电机状态1和错误标志命令的回复
struct [[nodiscard]] GetStatus1AndErrors final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    VoltageCode bus_voltage;                 // DATA[2-3] - 母线电压
    Current bus_current;                 // DATA[4-5] - 母线电流
    MotorState motor_state;              // DATA[6] - 电机状态
    ErrorState error_state;              // DATA[7] - 错误状态

    static constexpr uint8_t CMD_BYTE = 0x9a;
};


// 清除电机错误标志命令的回复
struct [[nodiscard]] ClearErrors final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    VoltageCode bus_voltage;                 // DATA[2-3] - 母线电压
    Current bus_current;                 // DATA[4-5] - 母线电流
    MotorState motor_state;              // DATA[6] - 电机状态
    ErrorState error_state;              // DATA[7] - 错误状态

    static constexpr uint8_t CMD_BYTE = 0x9b;
};


// 读取电机状态2命令的回复
struct [[nodiscard]] GetStatus2 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0x9c;
};


// 读取电机状态3命令的回复
struct [[nodiscard]] GetStatus3 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    PhaseCurrent phase_currents;         // DATA[2-7] - 三相电流

    static constexpr uint8_t CMD_BYTE = 0x9d;
};


// 抱闸器控制和状态读取命令的回复
struct [[nodiscard]] ManipulateBraker final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    uint8_t brake_status;                // DATA[1] - 抱闸状态: 0x00=刹车; 0x01=释放

    static constexpr uint8_t CMD_BYTE = 0x8c;
};


// 开环控制命令的回复
struct [[nodiscard]] OpenloopControl final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa0;
};


// 转矩闭环控制命令的回复
struct [[nodiscard]] TorqueClosedLoopControl final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa1;
};


// 速度闭环控制命令的回复
struct [[nodiscard]] SpeedClosedLoopControl final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa2;
};


// 多圈位置闭环控制命令1的回复
struct [[nodiscard]] MultiTurnPosClosedLoopControl1 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa3;
};


// 多圈位置闭环控制命令2的回复
struct [[nodiscard]] MultiTurnPosClosedLoopControl2 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa4;
};


// 单圈位置闭环控制命令1的回复
struct [[nodiscard]] SoloTurnPosClosedLoopControl1 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa5;
};


// 单圈位置闭环控制命令2的回复
struct [[nodiscard]] SoloLapPositionControl2 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa6;
};


// 增量位置闭环控制命令1的回复
struct [[nodiscard]] IncrementalPosClosedLoopControl1 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa7;
};


// 增量位置闭环控制命令2的回复
struct [[nodiscard]] IncrementalPosClosedLoopControl2 final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    TemperatureCode temperature;             // DATA[1] - 电机温度
    Current torque_power_current;        // DATA[2-3] - 转矩电流/功率
    int16_t speed_raw;                   // DATA[4-5] - 速度
    EncoderPosition enc_pos;             // DATA[6-7] - 编码器位置

    static constexpr uint8_t CMD_BYTE = 0xa8;
};


// 读取控制参数命令的回复
struct [[nodiscard]] ReadParam final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    uint8_t param_id;                    // DATA[1] - 参数ID
    std::array<uint8_t, 6> param_value;  // DATA[2-7] - 参数值

    static constexpr uint8_t CMD_BYTE = 0xc0;
};


// 读取电机编码器数据命令的回复
struct [[nodiscard]] ReadEncoderData final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    uint8_t null_byte;                   // DATA[1] - NULL
    EncoderPosition enc_pos;             // DATA[2-3] - 编码器位置
    EncoderRaw raw_pos;                  // DATA[4-5] - 原始位置
    EncoderOffset zero_offset;           // DATA[6-7] - 零偏

    static constexpr uint8_t CMD_BYTE = 0x90;
};


// 校准编码器命令的回复
struct [[nodiscard]] CalibrateEncoder final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    int32_t calibration_value;           // DATA[1-4] - 校准值
    int16_t calibration_ratio;           // DATA[5-6] - 校准比例
    uint8_t status;                      // DATA[7] - 校准状态

    static constexpr uint8_t CMD_BYTE = 0x18;
};


// 设置当前位置为电机零点（写入ROM）的回复
struct [[nodiscard]] SetCurrentAsZeroPointRom final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    EncoderOffset zero_offset;           // DATA[1-2] - 编码器零偏值
    std::array<uint8_t, 5> reserved;     // DATA[3-7] - 保留

    static constexpr uint8_t CMD_BYTE = 0x19;
};


// 读取多圈角度命令的回复
struct [[nodiscard]] ReadMultiTurnAngle final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 7> multi_turn_angle; // DATA[1-7] - 多圈角度（7字节）

    static constexpr uint8_t CMD_BYTE = 0x92;
};


// 读取单圈角度命令的回复
struct [[nodiscard]] ReadSingleTurnAngle final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 3> reserved;     // DATA[1-3] - 保留
    uint32_t single_turn_angle;          // DATA[4-7] - 单圈角度

    static constexpr uint8_t CMD_BYTE = 0x94;
};


// 设置当前位置为零点（写入RAM）的回复
struct [[nodiscard]] SetCurrentAsZeroPointRam final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 7> reserved;     // DATA[1-7] - 保留

    static constexpr uint8_t CMD_BYTE = 0x95;
};


// 读取设定参数命令的回复
struct [[nodiscard]] ReadSettingParam final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 7> param_value;  // DATA[1-7] - 参数值

    static constexpr uint8_t CMD_BYTE = 0x40;
};


// 写入设定参数命令的回复
struct [[nodiscard]] WriteSettingParam final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 7> param_value;  // DATA[1-7] - 参数值

    static constexpr uint8_t CMD_BYTE = 0x42;
};


// 保存设定参数命令的回复
struct [[nodiscard]] SaveSettingParam final{
    uint8_t cmd_byte;                    // DATA[0] - 命令字节
    std::array<uint8_t, 2> reserved1;    // DATA[1-2] - 保留
    uint8_t success_flag;                // DATA[2] - 成功标志: 0x01=成功; 0x00=失败
    std::array<uint8_t, 4> reserved2;    // DATA[3-7] - 保留

    static constexpr uint8_t CMD_BYTE = 0x44;
};


}


#endif

namespace param_msgs{
static constexpr auto WRITE_PARAM_COMMAND = Command::WriteParam;

struct PositionLoopPid{
    static constexpr ParamId PARAM_ID = ParamId::SetPositionLoopPid;

    KpCode kp_code;
    KiCode ki_code;
    KdCode kd_code;

    constexpr void fill_bytes(std::span<uint8_t, 6> bytes) const {
        bytes[0] = static_cast<uint8_t>(kp_code.bits);
        bytes[1] = static_cast<uint8_t>(kp_code.bits >> 8);
        bytes[2] = static_cast<uint8_t>(ki_code.bits);
        bytes[3] = static_cast<uint8_t>(ki_code.bits >> 8);
        bytes[4] = static_cast<uint8_t>(kd_code.bits);
        bytes[5] = static_cast<uint8_t>(kd_code.bits >> 8);
    }
};


struct SpeedLoopPid{
    static constexpr ParamId PARAM_ID = ParamId::SetSpeedLoopPid;

    KpCode kp_code;
    KiCode ki_code;
    KdCode kd_code;

    constexpr void fill_bytes(std::span<uint8_t, 6> bytes) const {
        bytes[0] = static_cast<uint8_t>(kp_code.bits);
        bytes[1] = static_cast<uint8_t>(kp_code.bits >> 8);
        bytes[2] = static_cast<uint8_t>(ki_code.bits);
        bytes[3] = static_cast<uint8_t>(ki_code.bits >> 8);
        bytes[4] = static_cast<uint8_t>(kd_code.bits);
        bytes[5] = static_cast<uint8_t>(kd_code.bits >> 8);
    }
};


struct CurrentLoopPid{
    static constexpr ParamId PARAM_ID = ParamId::SetCurrentLoopPid;

    KpCode kp_code;
    KiCode ki_code;
    KdCode kd_code;

    constexpr void fill_bytes(std::span<uint8_t, 6> bytes) const {
        bytes[0] = static_cast<uint8_t>(kp_code.bits);
        bytes[1] = static_cast<uint8_t>(kp_code.bits >> 8);
        bytes[2] = static_cast<uint8_t>(ki_code.bits);
        bytes[3] = static_cast<uint8_t>(ki_code.bits >> 8);
        bytes[4] = static_cast<uint8_t>(kd_code.bits);
        bytes[5] = static_cast<uint8_t>(kd_code.bits >> 8);
    }
};


}




struct FrameFactory{
    const NodeId motor_id;

    // 电机状态控制命令
    constexpr hal::ClassicCanFrame 
    get_status1_and_errors() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::GetStatus1AndErrors));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    clear_errors() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ClearErrors));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    get_status2() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::GetStatus2));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    get_status3() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::GetStatus3));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    motor_off() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::MotorOff));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    motor_on() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::MotorOn));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    motor_stop() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::MotorStop));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    manipulate_braker(const req_msgs::ManipulateBraker & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    openloop_control(const req_msgs::OpenloopControl & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    torque_closed_loop_control(const req_msgs::TorqueClosedLoopControl & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    speed_closed_loop_control(const req_msgs::SpeedClosedLoopControl & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    multi_turn_pos_closed_loop_control1(const req_msgs::MultiTurnPosClosedLoopControl1 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    multi_turn_pos_closed_loop_control2(const req_msgs::MultiTurnPosClosedLoopControl2 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    solo_turn_pos_closed_loop_control1(const req_msgs::SoloTurnPosClosedLoopControl1 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    solo_lap_position_control2(const req_msgs::SoloLapPositionControl2 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    incremental_pos_closed_loop_control1(const req_msgs::IncrementalPosClosedLoopControl1 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    incremental_pos_closed_loop_control2(const req_msgs::IncrementalPosClosedLoopControl2 & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    read_param(const req_msgs::ReadParam & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    template<typename ParamMsg>
    constexpr hal::ClassicCanFrame 
    write_param(const ParamMsg & msg) const {
        alignas(4) std::array<uint8_t, 8> buf;
        buf[0] = static_cast<uint8_t>(Command::WriteParam);
        buf[1] = static_cast<uint8_t>(ParamMsg::PARAM_ID);
        msg.fill_bytes(std::span<uint8_t, 6>(buf.data() + 2, 6));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    read_encoder_data() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ReadEncoderData));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    calibrate_encoder() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::CalibrateEncoder));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    set_current_as_zero_point_rom() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::SetCurrentAsZeroPointRom));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    read_multi_turn_angle() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ReadMultiTurnAngle));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    read_single_turn_angle() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::ReadSingleTurnAngle));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    set_current_as_zero_point_ram() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::SetCurrentAsZeroPointRam));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    read_setting_param(const req_msgs::ReadSettingParam & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    write_setting_param(const req_msgs::WriteSettingParam & msg) const {
        return make_can_frame(msg.to_u8x8());
    }

    constexpr hal::ClassicCanFrame 
    save_setting_param() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::SaveSettingParam));
        return make_can_frame(std::move(buf));
    }

    constexpr hal::ClassicCanFrame 
    reboot() const {
        auto && buf = make_head_and_0x00(static_cast<uint8_t>(Command::Reboot));
        return make_can_frame(std::move(buf));
    }

private:
    constexpr hal::ClassicCanFrame make_can_frame(std::array<uint8_t, 8> && u8x8) const {
        return hal::ClassicCanFrame::from_parts(
            motor_id.to_canid(),
            hal::ClassicCanPayload::from_u8x8(u8x8)
        );
    }
};

}