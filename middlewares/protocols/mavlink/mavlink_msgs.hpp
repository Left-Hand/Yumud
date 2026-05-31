#pragma once

#include "mavlink_primitive.hpp"

// https://mavlink.io/en/messages/common.html

namespace ymd::mavlink::msgs {

// 定义消息结构的宏
#define DEF_MAVMSG_BEGIN(msg_typename, msg_id) \
struct [[nodiscard]] msg_typename final {\
    static constexpr MavMessageId MSG_ID = MavMessageId{msg_id};

#define DEF_MAVMSG_MEMBER(offset, member_name, type)\
    type member_name;

#define DEF_MAVMSG_MEMBER_ARRAY(offset, member_name, type, arr_size)\
    std::array<type, arr_size> member_name;

#define DEF_MAVMSG_END \
    };


// HEARTBEAT (0)
// 心跳包，标识系统/组件在线、类型与状态。
DEF_MAVMSG_BEGIN(Heartbeat, 0)

    // 载具/组件类型（四轴、直升机、相机等）
    DEF_MAVMSG_MEMBER(0, type, MavType);
    
    // 飞控类型（如 ArduPilot、PX4）
    DEF_MAVMSG_MEMBER(1, autopilot, MavAutopilot);

    // 系统模式位图（解锁、武装、手动/自动）
    DEF_MAVMSG_MEMBER(2, base_mode, MavModeFlag);

    // 飞控自定义模式标识
    DEF_MAVMSG_MEMBER(3, custom_mode, uint32_t);

    // 系统状态（待机、飞行、故障）
    DEF_MAVMSG_MEMBER(7, system_status, MavState);

    // MAVLink 版本（协议自动填充）
    DEF_MAVMSG_MEMBER(8, mavlink_version, uint8_t);

DEF_MAVMSG_END


struct LoadPercents{
    uint16_t count;
};

struct VoltageCode{
    //mv
    uint16_t count;
};

struct CurrentCode{
    //cA
    int16_t count;

    constexpr bool is_invalid(){
        return count == -1;
    }
};

// SYS_STATUS (1)
// 系统核心状态（传感器、电池、负载、通信）。
DEF_MAVMSG_BEGIN(SysStatus, 1)

    // 传感器在位掩码（0=不存在，1=存在）
    DEF_MAVMSG_MEMBER(0, onboard_control_sensors_present, MavSysStatusSensor);
    
    // 传感器使能掩码（0=关闭，1=开启）
    DEF_MAVMSG_MEMBER(4, onboard_control_sensors_enabled, MavSysStatusSensor);

    // 传感器健康掩码（0=故障，1=正常）
    DEF_MAVMSG_MEMBER(8, onboard_control_sensors_health, MavSysStatusSensor);

    // 主循环最大占用率（0–1000，建议<1000）
    DEF_MAVMSG_MEMBER(3, load, LoadPercents);

    // 电池电压
    DEF_MAVMSG_MEMBER(4, voltage_battery, VoltageCode);

    // 电池电流
    DEF_MAVMSG_MEMBER(5, current_battery, CurrentCode);

    // 剩余电量百分比
    DEF_MAVMSG_MEMBER(6, battery_remaining, int8_t);

    // 通信丢包率（UART/I2C/SPI/CAN）
    DEF_MAVMSG_MEMBER(7, drop_rate_comm, uint16_t);

    // 通信错误数
    DEF_MAVMSG_MEMBER(8, errors_comm, uint16_t);

    // 飞控自定义错误码1
    DEF_MAVMSG_MEMBER(9, errors_count1, uint16_t);

    // 飞控自定义错误码2
    DEF_MAVMSG_MEMBER(10, errors_count2, uint16_t);

    // 飞控自定义错误码3
    DEF_MAVMSG_MEMBER(11, errors_count3, uint16_t);

    // 飞控自定义错误码4
    DEF_MAVMSG_MEMBER(12, errors_count4, uint16_t);

    // 扩展传感器在位掩码
    DEF_MAVMSG_MEMBER(13, onboard_control_sensors_present_extended, uint32_t);

    // 扩展传感器使能掩码
    DEF_MAVMSG_MEMBER(14, onboard_control_sensors_enabled_extended, uint32_t);

    // 扩展传感器健康掩码
    DEF_MAVMSG_MEMBER(15, onboard_control_sensors_health_extended, uint32_t);

DEF_MAVMSG_END


// SYSTEM_TIME (2)
// 系统时间（UNIX 时间+启动时间），用于日志与同步。
DEF_MAVMSG_BEGIN(SystemTime, 2)

    // UNIX 纪元时间戳（微秒）
    DEF_MAVMSG_MEMBER(0, time_unix_usec, uint64_t);

    // 系统启动后时间戳（毫秒）
    DEF_MAVMSG_MEMBER(1, time_boot_ms, uint32_t);

DEF_MAVMSG_END


// PING (4)
// 链路时延测试（已废弃，推荐 TIMESYNC）。
DEF_MAVMSG_BEGIN(Ping, 4)

    // 时间戳（UNIX 或启动后）
    DEF_MAVMSG_MEMBER(0, time_usec, uint64_t);

    // PING 序列号
    DEF_MAVMSG_MEMBER(1, seq, uint32_t);

    // 目标系统 ID（0=广播）
    DEF_MAVMSG_MEMBER(2, target_system, uint8_t);

    // 目标组件 ID（0=广播）
    DEF_MAVMSG_MEMBER(3, target_component, uint8_t);

DEF_MAVMSG_END


// CHANGE_OPERATOR_CONTROL (5)
// 请求控制此MAV（载具）的权限切换。
DEF_MAVMSG_BEGIN(ChangeOperatorControl, 5)

    // 系统ID，地面站请求控制的载具
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 0: 请求控制此MAV，1: 释放控制权
    DEF_MAVMSG_MEMBER(1, control_request, uint8_t);

    // 0: 密钥明文，1-255: 未来不同哈希/加密变体
    DEF_MAVMSG_MEMBER(2, version, uint8_t);

    // 密码/密钥，25个字符以内，NULL结尾
    DEF_MAVMSG_MEMBER(3, passkey, OwnedNtstr<25>);

DEF_MAVMSG_END


// CHANGE_OPERATOR_CONTROL_ACK (6)
// 控制权切换请求的接受/拒绝确认消息。
DEF_MAVMSG_BEGIN(ChangeOperatorControlAck, 6)

    // 发送此消息的地面站ID
    DEF_MAVMSG_MEMBER(0, gcs_system_id, uint8_t);

    // 0: 请求控制此MAV，1: 释放控制权
    DEF_MAVMSG_MEMBER(1, control_request, uint8_t);

    // 0: 确认，1: 否认-密码错误，2: 否认-不支持的加密方式，3: 否认-已在控制中
    DEF_MAVMSG_MEMBER(2, ack, uint8_t);

DEF_MAVMSG_END


// AUTH_KEY (7)
// 发送识别此系统的加密签名/密钥（注意：需通过加密通道传输以确保安全）。
DEF_MAVMSG_BEGIN(AuthKey, 7)

    // 加密密钥
    DEF_MAVMSG_MEMBER(0, key, OwnedNtstr<32>);

DEF_MAVMSG_END


// PARAM_REQUEST_READ (20)
// 请求读取指定参数ID的板载参数值（参数存储为键[字符串] -> 值[fp32]）。
DEF_MAVMSG_BEGIN(ParamRequestRead, 20)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 参数ID字符串，以NULL结尾（若小于16字符）或无NULL终止符（若恰好16字符）
    DEF_MAVMSG_MEMBER(2, param_id, OwnedNtstr<16>);

    // 参数索引，设为-1表示使用参数ID作为标识符
    DEF_MAVMSG_MEMBER(18, param_index, int16_t);

DEF_MAVMSG_END


// PARAM_REQUEST_LIST (21)
// 请求此组件的所有参数（接收方将以PARAM_VALUE消息广播所有参数值）。
DEF_MAVMSG_BEGIN(ParamRequestList, 21)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

DEF_MAVMSG_END


// PARAM_VALUE (22)
// 发送命令将指定参数设置为某个值（设置完成后应通过PARAM_VALUE消息广播当前值）。
DEF_MAVMSG_BEGIN(ParamSet, 22)

    // 参数ID字符串
    DEF_MAVMSG_MEMBER(2, param_id, OwnedNtstr<16>);

    // 	机载参数值
    DEF_MAVMSG_MEMBER(3, param_value, fp32);

    // 机载参数类型
    DEF_MAVMSG_MEMBER(4, param_type, MavParamType);

    // 车载参数总数
    DEF_MAVMSG_MEMBER(4, param_count, uint16_t);

    // 该车载参数的索引
    DEF_MAVMSG_MEMBER(4, param_index, uint16_t);

DEF_MAVMSG_END


// PARAM_SET (23)
// 参数的当前值（响应参数请求或参数变化时广播）。
DEF_MAVMSG_BEGIN(ParamSet, 23)
    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 参数ID字符串
    DEF_MAVMSG_MEMBER(0, param_id, OwnedNtstr<16>);

    // 参数值
    DEF_MAVMSG_MEMBER(1, param_value, fp32);

    // 参数类型
    DEF_MAVMSG_MEMBER(2, param_type, MavParamType);

DEF_MAVMSG_END


struct DegE7{
    int32_t bits;
};

// GPS_RAW_INT (24)
// GPS 原始数据（非融合定位）。
DEF_MAVMSG_BEGIN(GpsRawInt, 24)

    // 时间戳
    DEF_MAVMSG_MEMBER(0, time_usec, uint64_t);

    // 定位类型（0=无，2=2D，3=3D）
    DEF_MAVMSG_MEMBER(1, fix_type, GpsFixType);

    // 纬度（WGS84，度数×1e7）
    DEF_MAVMSG_MEMBER(2, lat, DegE7);

    // 经度（WGS84，度数×1e7）
    DEF_MAVMSG_MEMBER(3, lon, DegE7);

    // 海拔（毫米，MSL）
    DEF_MAVMSG_MEMBER(4, alt, int32_t);

    // 水平精度（厘米）
    DEF_MAVMSG_MEMBER(5, eph, uint16_t);

    // 垂直精度（厘米）
    DEF_MAVMSG_MEMBER(6, epv, uint16_t);

    // 地速（厘米/秒）
    DEF_MAVMSG_MEMBER(7, vel, int16_t);

    // 航向角（百分度）
    DEF_MAVMSG_MEMBER(8, cog, uint16_t);

    // 可见卫星数
    DEF_MAVMSG_MEMBER(9, satellites_visible, uint8_t);

DEF_MAVMSG_END


// SCALED_IMU (26)
// 通常9DOF传感器的缩放IMU读数（包含磁力计、陀螺仪、加速度计数据）。
DEF_MAVMSG_BEGIN(ScaledImu, 26)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // X轴加速度
    DEF_MAVMSG_MEMBER(1, xacc, int16_t);

    // Y轴加速度
    DEF_MAVMSG_MEMBER(2, yacc, int16_t);

    // Z轴加速度
    DEF_MAVMSG_MEMBER(3, zacc, int16_t);

    // X轴角速度
    DEF_MAVMSG_MEMBER(4, xgyro, int16_t);

    // Y轴角速度
    DEF_MAVMSG_MEMBER(5, ygyro, int16_t);

    // Z轴角速度
    DEF_MAVMSG_MEMBER(6, zgyro, int16_t);

    // X轴磁场强度
    DEF_MAVMSG_MEMBER(7, xmag, int16_t);

    // Y轴磁场强度
    DEF_MAVMSG_MEMBER(8, ymag, int16_t);

    // Z轴磁场强度
    DEF_MAVMSG_MEMBER(9, zmag, int16_t);

DEF_MAVMSG_END


// SCALED_PRESSURE (29)
// 气压数据（已缩放）。
DEF_MAVMSG_BEGIN(ScaledPressure, 29)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // 绝对气压
    DEF_MAVMSG_MEMBER(1, press_abs, fp32);

    // 差分压力（高度计）
    DEF_MAVMSG_MEMBER(2, press_diff, fp32);

    // 温度
    DEF_MAVMSG_MEMBER(3, temperature, int16_t);

DEF_MAVMSG_END


// ATTITUDE (30)
// 姿态数据（欧拉角+角速度）。
DEF_MAVMSG_BEGIN(Attitude, 30)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // 横滚角
    DEF_MAVMSG_MEMBER(1, roll, fp32);

    // 俯仰角
    DEF_MAVMSG_MEMBER(2, pitch, fp32);

    // 偏航角
    DEF_MAVMSG_MEMBER(3, yaw, fp32);

    // 横滚角速度
    DEF_MAVMSG_MEMBER(4, rollspeed, fp32);

    // 俯仰角速度
    DEF_MAVMSG_MEMBER(5, pitchspeed, fp32);

    // 偏航角速度
    DEF_MAVMSG_MEMBER(6, yawspeed, fp32);

DEF_MAVMSG_END


// GLOBAL_POSITION_INT (33)
// 融合后全局位置（经纬高+相对高度）。
DEF_MAVMSG_BEGIN(GlobalPositionInt, 33)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // 纬度（WGS84）
    DEF_MAVMSG_MEMBER(1, lat, int32_t);

    // 经度（WGS84）
    DEF_MAVMSG_MEMBER(2, lon, int32_t);

    // 海拔（MSL）
    DEF_MAVMSG_MEMBER(3, alt, int32_t);

    // 相对高度（起飞点）
    DEF_MAVMSG_MEMBER(4, relative_alt, int32_t);

    // 北向速度
    DEF_MAVMSG_MEMBER(5, vx, int16_t);

    // 东向速度
    DEF_MAVMSG_MEMBER(6, vy, int16_t);

    // 下向速度
    DEF_MAVMSG_MEMBER(7, vz, int16_t);

    // 航向角（百分度）
    DEF_MAVMSG_MEMBER(8, hdg, uint16_t);

DEF_MAVMSG_END


// RC_CHANNELS_RAW (35)
// 遥控器原始通道值（0-65535）。
DEF_MAVMSG_BEGIN(RcChannelsRaw, 35)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // RC接收机端口号（0-3）
    DEF_MAVMSG_MEMBER(1, port, uint8_t);

    // 通道1原始值
    DEF_MAVMSG_MEMBER(2, chan1_raw, uint16_t);

    // 通道2原始值
    DEF_MAVMSG_MEMBER(3, chan2_raw, uint16_t);

    // 通道3原始值
    DEF_MAVMSG_MEMBER(4, chan3_raw, uint16_t);

    // 通道4原始值
    DEF_MAVMSG_MEMBER(5, chan4_raw, uint16_t);

    // 通道5原始值
    DEF_MAVMSG_MEMBER(6, chan5_raw, uint16_t);

    // 通道6原始值
    DEF_MAVMSG_MEMBER(7, chan6_raw, uint16_t);

    // 通道7原始值
    DEF_MAVMSG_MEMBER(8, chan7_raw, uint16_t);

    // 通道8原始值
    DEF_MAVMSG_MEMBER(9, chan8_raw, uint16_t);

    // 接收信号强度指示（0-100，0=无效）
    DEF_MAVMSG_MEMBER(10, rssi, uint8_t);

DEF_MAVMSG_END


// SERVO_OUTPUT_RAW (36)
// 伺服输出原始脉冲宽度值（0-65535）。
DEF_MAVMSG_BEGIN(ServoOutputRaw, 36)

    // 时间戳（微秒）
    DEF_MAVMSG_MEMBER(0, time_usec, uint32_t);

    // 端口号（0为MAIN，1为AUX）
    DEF_MAVMSG_MEMBER(1, port, uint8_t);

    // 伺服1输出脉冲宽度
    DEF_MAVMSG_MEMBER(2, servo1_raw, uint16_t);

    // 伺服2输出脉冲宽度
    DEF_MAVMSG_MEMBER(3, servo2_raw, uint16_t);

    // 伺服3输出脉冲宽度
    DEF_MAVMSG_MEMBER(4, servo3_raw, uint16_t);

    // 伺服4输出脉冲宽度
    DEF_MAVMSG_MEMBER(5, servo4_raw, uint16_t);

    // 伺服5输出脉冲宽度
    DEF_MAVMSG_MEMBER(6, servo5_raw, uint16_t);

    // 伺服6输出脉冲宽度
    DEF_MAVMSG_MEMBER(7, servo6_raw, uint16_t);

    // 伺服7输出脉冲宽度
    DEF_MAVMSG_MEMBER(8, servo7_raw, uint16_t);

    // 伺服8输出脉冲宽度
    DEF_MAVMSG_MEMBER(9, servo8_raw, uint16_t);

DEF_MAVMSG_END


// MISSION_ITEM (39)
// 任务项（坐标、高度、动作等）。
DEF_MAVMSG_BEGIN(MissionItem, 39)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(2, seq, uint16_t);

    // 坐标系
    DEF_MAVMSG_MEMBER(3, frame, MavFrame);

    // 命令ID
    DEF_MAVMSG_MEMBER(4, command, uint16_t);

    // 0: 任务项，1: 当前任务
    DEF_MAVMSG_MEMBER(5, current, uint8_t);

    // 自动继续到下一任务
    DEF_MAVMSG_MEMBER(6, autocontinue, uint8_t);

    // 参数1
    DEF_MAVMSG_MEMBER(7, param1, fp32);

    // 参数2
    DEF_MAVMSG_MEMBER(8, param2, fp32);

    // 参数3
    DEF_MAVMSG_MEMBER(9, param3, fp32);

    // 参数4
    DEF_MAVMSG_MEMBER(10, param4, fp32);

    // X位置或参数
    DEF_MAVMSG_MEMBER(11, x, fp32);

    // Y位置或参数
    DEF_MAVMSG_MEMBER(12, y, fp32);

    // Z位置或参数
    DEF_MAVMSG_MEMBER(13, z, fp32);

DEF_MAVMSG_END


// MISSION_REQUEST (40)
// 请求特定任务项（用于任务上传/下载）。
DEF_MAVMSG_BEGIN(MissionRequest, 40)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(2, seq, uint16_t);

DEF_MAVMSG_END


// MISSION_SET_CURRENT (41)
// 设置当前任务（激活特定任务项）。
DEF_MAVMSG_BEGIN(MissionSetCurrent, 41)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(2, seq, uint16_t);

DEF_MAVMSG_END


// MISSION_CURRENT (42)
// 当前执行的任务序号。
DEF_MAVMSG_BEGIN(MissionCurrent, 42)

    // 当前任务序号
    DEF_MAVMSG_MEMBER(0, seq, uint16_t);

DEF_MAVMSG_END


// MISSION_REQUEST_LIST (43)
// 请求任务列表信息（总任务数）。
DEF_MAVMSG_BEGIN(MissionRequestList, 43)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

DEF_MAVMSG_END


// MISSION_COUNT (44)
// 任务总数（响应任务列表请求）。
DEF_MAVMSG_BEGIN(MissionCount, 44)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 任务总数
    DEF_MAVMSG_MEMBER(2, count, uint16_t);

DEF_MAVMSG_END


// MISSION_CLEAR_ALL (45)
// 清除所有任务（删除任务列表）。
DEF_MAVMSG_BEGIN(MissionClearAll, 45)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

DEF_MAVMSG_END


// MISSION_ITEM_REACHED (46)
// 任务项已到达（报告任务执行进度）。
DEF_MAVMSG_BEGIN(MissionItemReached, 46)

    // 已到达的任务序号
    DEF_MAVMSG_MEMBER(0, seq, uint16_t);

DEF_MAVMSG_END


// MISSION_ACK (47)
// 任务操作确认（任务上传/下载结果）。
DEF_MAVMSG_BEGIN(MissionAck, 47)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 任务操作结果
    DEF_MAVMSG_MEMBER(2, type, MavMissionResult);

DEF_MAVMSG_END


// GPS_GLOBAL_ORIGIN (49)
// GPS全球原点（基准坐标系）的位置信息。
DEF_MAVMSG_BEGIN(GpsGlobalOrigin, 49)

    // 纬度
    DEF_MAVMSG_MEMBER(0, latitude, int32_t);

    // 经度
    DEF_MAVMSG_MEMBER(1, longitude, int32_t);

    // 海拔（毫米）
    DEF_MAVMSG_MEMBER(2, altitude, int32_t);

    // 时间戳（微秒）
    DEF_MAVMSG_MEMBER(3, time_usec, uint64_t);

DEF_MAVMSG_END



// RC_CHANNELS (65)
// 遥控器通道原始值（更新版本，支持更多通道）。
DEF_MAVMSG_BEGIN(RcChannels, 65)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(0, time_boot_ms, uint32_t);

    // 通道数量
    DEF_MAVMSG_MEMBER(1, chancount, uint8_t);

    // 通道1原始值
    DEF_MAVMSG_MEMBER(2, chan1_raw, uint16_t);

    // 通道2原始值
    DEF_MAVMSG_MEMBER(3, chan2_raw, uint16_t);

    // 通道3原始值
    DEF_MAVMSG_MEMBER(4, chan3_raw, uint16_t);

    // 通道4原始值
    DEF_MAVMSG_MEMBER(5, chan4_raw, uint16_t);

    // 通道5原始值
    DEF_MAVMSG_MEMBER(6, chan5_raw, uint16_t);

    // 通道6原始值
    DEF_MAVMSG_MEMBER(7, chan6_raw, uint16_t);

    // 通道7原始值
    DEF_MAVMSG_MEMBER(8, chan7_raw, uint16_t);

    // 通道8原始值
    DEF_MAVMSG_MEMBER(9, chan8_raw, uint16_t);

    // 通道9原始值
    DEF_MAVMSG_MEMBER(10, chan9_raw, uint16_t);

    // 通道10原始值
    DEF_MAVMSG_MEMBER(11, chan10_raw, uint16_t);

    // 通道11原始值
    DEF_MAVMSG_MEMBER(12, chan11_raw, uint16_t);

    // 通道12原始值
    DEF_MAVMSG_MEMBER(13, chan12_raw, uint16_t);

    // 通道13原始值
    DEF_MAVMSG_MEMBER(14, chan13_raw, uint16_t);

    // 通道14原始值
    DEF_MAVMSG_MEMBER(15, chan14_raw, uint16_t);

    // 通道15原始值
    DEF_MAVMSG_MEMBER(16, chan15_raw, uint16_t);

    // 通道16原始值
    DEF_MAVMSG_MEMBER(17, chan16_raw, uint16_t);

    // 通道17原始值（可选）
    DEF_MAVMSG_MEMBER(18, chan17_raw, uint16_t);

    // 通道18原始值（可选）
    DEF_MAVMSG_MEMBER(19, chan18_raw, uint16_t);

    // 接收信号强度指示
    DEF_MAVMSG_MEMBER(20, rssi, uint8_t);

DEF_MAVMSG_END


// RC_CHANNELS_OVERRIDE (70)
// 遥控通道覆盖（地面站发送虚拟遥控值）。
DEF_MAVMSG_BEGIN(RcChannelsOverride, 70)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 通道1覆盖值
    DEF_MAVMSG_MEMBER(2, chan1_raw, uint16_t);

    // 通道2覆盖值
    DEF_MAVMSG_MEMBER(3, chan2_raw, uint16_t);

    // 通道3覆盖值
    DEF_MAVMSG_MEMBER(4, chan3_raw, uint16_t);

    // 通道4覆盖值
    DEF_MAVMSG_MEMBER(5, chan4_raw, uint16_t);

    // 通道5覆盖值
    DEF_MAVMSG_MEMBER(6, chan5_raw, uint16_t);

    // 通道6覆盖值
    DEF_MAVMSG_MEMBER(7, chan6_raw, uint16_t);

    // 通道7覆盖值
    DEF_MAVMSG_MEMBER(8, chan7_raw, uint16_t);

    // 通道8覆盖值
    DEF_MAVMSG_MEMBER(9, chan8_raw, uint16_t);

DEF_MAVMSG_END



// COMMAND_LONG (76)
// 长命令（7个参数，如解锁、起飞等）。
DEF_MAVMSG_BEGIN(CommandLong, 76)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(0, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(1, target_component, uint8_t);

    // 命令ID
    DEF_MAVMSG_MEMBER(2, command, uint16_t);

    // 确认次数（0为首次发送）
    DEF_MAVMSG_MEMBER(3, confirmation, uint8_t);

    // 参数1
    DEF_MAVMSG_MEMBER(4, param1, fp32);

    // 参数2
    DEF_MAVMSG_MEMBER(5, param2, fp32);

    // 参数3
    DEF_MAVMSG_MEMBER(6, param3, fp32);

    // 参数4
    DEF_MAVMSG_MEMBER(7, param4, fp32);

    // 参数5
    DEF_MAVMSG_MEMBER(8, param5, fp32);

    // 参数6
    DEF_MAVMSG_MEMBER(9, param6, fp32);

    // 参数7
    DEF_MAVMSG_MEMBER(10, param7, fp32);

DEF_MAVMSG_END


// COMMAND_ACK (77)
// 命令确认（命令执行结果）。
DEF_MAVMSG_BEGIN(CommandAck, 77)

    // 命令ID
    DEF_MAVMSG_MEMBER(0, command, uint16_t);

    // 命令执行结果
    DEF_MAVMSG_MEMBER(1, result, MavResult);

    // 进度百分比（0-100，255表示不适用）
    DEF_MAVMSG_MEMBER(2, progress, uint8_t);

    // 附加结果参数
    DEF_MAVMSG_MEMBER(3, result_param2, int32_t);

    // 目标系统ID
    DEF_MAVMSG_MEMBER(4, target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(5, target_component, uint8_t);

DEF_MAVMSG_END


// ACTUATOR_CONTROL_TARGET (140)
// 执行器控制目标（姿态+油门）。
DEF_MAVMSG_BEGIN(ActuatorControlTarget, 140)

    // 时间戳
    DEF_MAVMSG_MEMBER(0, time_usec, uint64_t);

    // 执行器组 ID（多实例区分）
    DEF_MAVMSG_MEMBER(1, group_mlx, uint8_t);

    // 控制量（-1~+1：横滚、俯仰、偏航、油门、襟翼、扰流板、刹车、起落架）
    DEF_MAVMSG_MEMBER_ARRAY(2, controls, fp32, 8);

DEF_MAVMSG_END


// ALTITUDE (141)
// 多维度高度数据（单调/绝对/相对/地形）。
DEF_MAVMSG_BEGIN(Altitude, 141)

    // 时间戳
    DEF_MAVMSG_MEMBER(0, time_usec, uint64_t);

    // 单调高度（启动后不重置）
    DEF_MAVMSG_MEMBER(1, altitude_monotonic, fp32);

    // 绝对海拔（MSL）
    DEF_MAVMSG_MEMBER(2, altitude_amsl, fp32);

    // 本地坐标系高度
    DEF_MAVMSG_MEMBER(3, altitude_local, fp32);

    // 相对起飞点高度
    DEF_MAVMSG_MEMBER(4, altitude_relative, fp32);

    // 相对地形高度（<-1000=无效）
    DEF_MAVMSG_MEMBER(5, altitude_terrain, fp32);

    // 离地间隙（负=无效）
    DEF_MAVMSG_MEMBER(6, bottom_clearance, fp32);

DEF_MAVMSG_END


// BATTERY_STATUS (147)
// 电池详细状态（多电芯电池）。
DEF_MAVMSG_BEGIN(BatteryStatus, 147)

    // 电池ID
    DEF_MAVMSG_MEMBER(0, id, uint8_t);

    // 电池功能
    DEF_MAVMSG_MEMBER(1, battery_function, MavBatteryFunction);

    // 电池类型
    DEF_MAVMSG_MEMBER(2, type, MavBatteryType);

    // 电池温度
    DEF_MAVMSG_MEMBER(3, temperature, int16_t);

    // 电芯电压数组
    DEF_MAVMSG_MEMBER_ARRAY(4, voltages, uint16_t, 10);

    // 电池电流
    DEF_MAVMSG_MEMBER(5, current_battery, int16_t);

    // 已消耗电流
    DEF_MAVMSG_MEMBER(6, current_consumed, int32_t);

    // 已消耗能量
    DEF_MAVMSG_MEMBER(7, energy_consumed, int32_t);

    // 剩余电量百分比
    DEF_MAVMSG_MEMBER(8, battery_remaining, int8_t);

    // 剩余使用时间
    DEF_MAVMSG_MEMBER(9, time_remaining, uint32_t);

    // 充电状态
    DEF_MAVMSG_MEMBER(10, charge_state, MavBatteryChargeState);

DEF_MAVMSG_END


// AUTOPILOT_VERSION (181)
// 自动驾驶仪版本信息（固件版本、能力等）。
DEF_MAVMSG_BEGIN(AutopilotVersion, 181)

    // 自动驾驶仪能力位图
    DEF_MAVMSG_MEMBER(0, capabilities, uint64_t);

    // 飞行软件版本
    DEF_MAVMSG_MEMBER(1, flight_sw_version, uint32_t);

    // 中间件软件版本
    DEF_MAVMSG_MEMBER(2, middleware_sw_version, uint32_t);

    // 操作系统版本
    DEF_MAVMSG_MEMBER(3, os_sw_version, uint32_t);

    // 板卡硬件版本
    DEF_MAVMSG_MEMBER(4, board_version, uint32_t);

    // 飞行软件自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(5, flight_custom_version, uint8_t, 8);

    // 中间件自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(6, middleware_custom_version, uint8_t, 8);

    // 操作系统自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(7, os_custom_version, uint8_t, 8);

    // 供应商ID
    DEF_MAVMSG_MEMBER(8, vendor_id, int16_t);

    // 产品ID
    DEF_MAVMSG_MEMBER(9, product_id, int16_t);

    // 硬件UID
    DEF_MAVMSG_MEMBER(10, uid, uint64_t);

    // 扩展硬件UID
    DEF_MAVMSG_MEMBER_ARRAY(11, uid2, uint8_t, 18);

DEF_MAVMSG_END


// HIGH_LATENCY2 (235)
// 高延迟链路精简遥测（替代旧版HIGH_LATENCY，适用于卫星或LoRa等低带宽链路）。
DEF_MAVMSG_BEGIN(HighLatency2, 235)

    // 时间戳（自纪元以来的秒数）
    DEF_MAVMSG_MEMBER(0, timestamp, uint32_t);

    // 载具类型
    DEF_MAVMSG_MEMBER(1, type, MavType);

    // 自动驾驶仪类型
    DEF_MAVMSG_MEMBER(2, autopilot, MavAutopilot);

    // 航向角
    DEF_MAVMSG_MEMBER(3, heading, uint16_t);

    // 纬度
    DEF_MAVMSG_MEMBER(4, latitude, int32_t);

    // 经度
    DEF_MAVMSG_MEMBER(5, longitude, int32_t);

    // 海拔高度
    DEF_MAVMSG_MEMBER(6, altitude, int16_t);

    // 目标高度
    DEF_MAVMSG_MEMBER(7, target_altitude, int16_t);

    // 目标纬度
    DEF_MAVMSG_MEMBER(8, latitude_int, int32_t);

    // 目标经度
    DEF_MAVMSG_MEMBER(9, longitude_int, int32_t);

    // 目标航向
    DEF_MAVMSG_MEMBER(10, target_heading, uint8_t);

    // 到目标距离
    DEF_MAVMSG_MEMBER(11, target_distance, uint16_t);

    // 油门百分比
    DEF_MAVMSG_MEMBER(12, throttle, uint8_t);

    // 空速
    DEF_MAVMSG_MEMBER(13, airspeed, uint8_t);

    // 空速设定点
    DEF_MAVMSG_MEMBER(14, airspeed_sp, uint8_t);

    // 地速
    DEF_MAVMSG_MEMBER(15, groundspeed, uint8_t);

    // 风速
    DEF_MAVMSG_MEMBER(16, windspeed, uint8_t);

    // 风向
    DEF_MAVMSG_MEMBER(17, wind_direction, uint8_t);

    // GPS水平精度
    DEF_MAVMSG_MEMBER(18, eph, uint8_t);

    // GPS垂直精度
    DEF_MAVMSG_MEMBER(19, epv, uint8_t);

    // 空气温度
    DEF_MAVMSG_MEMBER(20, temperature_air, int8_t);

    // 爬升率
    DEF_MAVMSG_MEMBER(21, climb_rate, int8_t);

    // 电池百分比
    DEF_MAVMSG_MEMBER(22, battery, int8_t);

    // 自定义字段0
    DEF_MAVMSG_MEMBER(23, custom0, int8_t);

    // 自定义字段1
    DEF_MAVMSG_MEMBER(24, custom1, int8_t);

    // 自定义字段2
    DEF_MAVMSG_MEMBER(25, custom2, uint8_t);

DEF_MAVMSG_END


} // namespace ymd::mavlink::msgs