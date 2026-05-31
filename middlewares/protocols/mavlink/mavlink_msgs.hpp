#pragma once

#include "mavlink_primitive.hpp"
#include "primitive/arithmetic/angular.hpp"
#include "core/utils/unit/unit.hpp"

// https://mavlink.io/en/messages/common.html

namespace ymd::mavlink::msgs {

#define DEF_GENERIC_TYPENAME(base, ...) base<__VA_ARGS__>

// 定义消息结构的宏
#define DEF_MAVMSG_BEGIN(msg_typename, msg_id) \
struct [[nodiscard]] msg_typename final {\
    static constexpr MavMessageId MSG_ID = MavMessageId{msg_id};

#define DEF_MAVMSG_MEMBER(member_name, type)\
    type member_name;

#define DEF_MAVMSG_MEMBER_ARRAY(member_name, type, arr_size)\
    std::array<type, arr_size> member_name;

#define DEF_MAVMSG_END \
    };


// HEARTBEAT (0)
// 心跳包，标识系统/组件在线、类型与状态。
DEF_MAVMSG_BEGIN(Heartbeat, 0)

    // 载具/组件类型（四轴、直升机、相机等）
    DEF_MAVMSG_MEMBER(type, MavType);
    
    // 飞控类型（如 ArduPilot、PX4）
    DEF_MAVMSG_MEMBER(autopilot, MavAutopilot);

    // 系统模式位图（解锁、武装、手动/自动）
    DEF_MAVMSG_MEMBER(base_mode, MavModeFlag);

    // 飞控自定义模式标识
    DEF_MAVMSG_MEMBER(custom_mode, uint32_t);

    // 系统状态（待机、飞行、故障）
    DEF_MAVMSG_MEMBER(system_status, MavState);

    // MAVLink 版本（协议自动填充）
    DEF_MAVMSG_MEMBER(mavlink_version, uint8_t);

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
    DEF_MAVMSG_MEMBER(onboard_control_sensors_present, MavSysStatusSensor);
    
    // 传感器使能掩码（0=关闭，1=开启）
    DEF_MAVMSG_MEMBER(onboard_control_sensors_enabled, MavSysStatusSensor);

    // 传感器健康掩码（0=故障，1=正常）
    DEF_MAVMSG_MEMBER(onboard_control_sensors_health, MavSysStatusSensor);

    // 主循环最大占用率（0–1000，建议<1000）
    DEF_MAVMSG_MEMBER(load, LoadPercents);

    // 电池电压
    DEF_MAVMSG_MEMBER(voltage_battery, VoltageCode);

    // 电池电流
    DEF_MAVMSG_MEMBER(current_battery, CurrentCode);

    // 剩余电量百分比
    DEF_MAVMSG_MEMBER(battery_remaining, int8_t);

    // 通信丢包率（UART/I2C/SPI/CAN）
    DEF_MAVMSG_MEMBER(drop_rate_comm, uint16_t);

    // 通信错误数
    DEF_MAVMSG_MEMBER(errors_comm, uint16_t);

    // 飞控自定义错误码1
    DEF_MAVMSG_MEMBER_ARRAY(errors_count, uint16_t, 4);

    // 扩展传感器在位掩码
    DEF_MAVMSG_MEMBER(onboard_control_sensors_present_extended, uint32_t);

    // 扩展传感器使能掩码
    DEF_MAVMSG_MEMBER(onboard_control_sensors_enabled_extended, uint32_t);

    // 扩展传感器健康掩码
    DEF_MAVMSG_MEMBER(onboard_control_sensors_health_extended, uint32_t);

DEF_MAVMSG_END


// SYSTEM_TIME (2)
// 系统时间（UNIX 时间+启动时间），用于日志与同步。
DEF_MAVMSG_BEGIN(SystemTime, 2)

    // UNIX 纪元时间戳（微秒）
    DEF_MAVMSG_MEMBER(time_unix_usec, uint64_t);

    // 系统启动后时间戳（毫秒）
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

DEF_MAVMSG_END


// PING (4)
// 链路时延测试（已废弃，推荐 TIMESYNC）。
DEF_MAVMSG_BEGIN(Ping, 4)

    // 时间戳（UNIX 或启动后）
    DEF_MAVMSG_MEMBER(time_usec, uint64_t);

    // PING 序列号
    DEF_MAVMSG_MEMBER(seq, uint32_t);

    // 目标系统 ID（0=广播）
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件 ID（0=广播）
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

DEF_MAVMSG_END


// CHANGE_OPERATOR_CONTROL (5)
// 请求控制此MAV（载具）的权限切换。
DEF_MAVMSG_BEGIN(ChangeOperatorControl, 5)

    // 系统ID，地面站请求控制的载具
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 0: 请求控制此MAV，1: 释放控制权
    DEF_MAVMSG_MEMBER(control_request, uint8_t);

    // 0: 密钥明文，1-255: 未来不同哈希/加密变体
    DEF_MAVMSG_MEMBER(version, uint8_t);

    // 密码/密钥，25个字符以内，NULL结尾
    DEF_MAVMSG_MEMBER(passkey, OwnedNtstr<25>);

DEF_MAVMSG_END


// CHANGE_OPERATOR_CONTROL_ACK (6)
// 控制权切换请求的接受/拒绝确认消息。
DEF_MAVMSG_BEGIN(ChangeOperatorControlAck, 6)

    // 发送此消息的地面站ID
    DEF_MAVMSG_MEMBER(gcs_system_id, uint8_t);

    // 0: 请求控制此MAV，1: 释放控制权
    DEF_MAVMSG_MEMBER(control_request, uint8_t);

    // 0: 确认，1: 否认-密码错误，2: 否认-不支持的加密方式，3: 否认-已在控制中
    DEF_MAVMSG_MEMBER(ack, uint8_t);

DEF_MAVMSG_END


// AUTH_KEY (7)
// 发送识别此系统的加密签名/密钥（注意：需通过加密通道传输以确保安全）。
DEF_MAVMSG_BEGIN(AuthKey, 7)

    // 加密密钥
    DEF_MAVMSG_MEMBER(key, OwnedNtstr<32>);

DEF_MAVMSG_END


// PARAM_REQUEST_READ (20)
// 请求读取指定参数ID的板载参数值（参数存储为键[字符串] -> 值[fp32]）。
DEF_MAVMSG_BEGIN(ParamRequestRead, 20)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 参数ID字符串，以NULL结尾（若小于16字符）或无NULL终止符（若恰好16字符）
    DEF_MAVMSG_MEMBER(param_id, OwnedNtstr<16>);

    // 参数索引，设为-1表示使用参数ID作为标识符
    DEF_MAVMSG_MEMBER(param_index, int16_t);

DEF_MAVMSG_END


// PARAM_REQUEST_LIST (21)
// 请求此组件的所有参数（接收方将以PARAM_VALUE消息广播所有参数值）。
DEF_MAVMSG_BEGIN(ParamRequestList, 21)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

DEF_MAVMSG_END


// PARAM_VALUE (22)
// 发送命令将指定参数设置为某个值（设置完成后应通过PARAM_VALUE消息广播当前值）。
DEF_MAVMSG_BEGIN(ParamValue, 22)

    // 参数ID字符串
    DEF_MAVMSG_MEMBER(param_id, OwnedNtstr<16>);

    // 	机载参数值
    DEF_MAVMSG_MEMBER(param_value, fp32);

    // 机载参数类型
    DEF_MAVMSG_MEMBER(param_type, MavParamType);

    // 车载参数总数
    DEF_MAVMSG_MEMBER(param_count, uint16_t);

    // 该车载参数的索引
    DEF_MAVMSG_MEMBER(param_index, uint16_t);

DEF_MAVMSG_END


// PARAM_SET (23)
// 参数的当前值（响应参数请求或参数变化时广播）。
DEF_MAVMSG_BEGIN(ParamSet, 23)
    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 参数ID字符串
    DEF_MAVMSG_MEMBER(param_id, OwnedNtstr<16>);

    // 参数值
    DEF_MAVMSG_MEMBER(param_value, fp32);

    // 参数类型
    DEF_MAVMSG_MEMBER(param_type, MavParamType);

DEF_MAVMSG_END


struct DegE7{
    int32_t bits;
};

// GPS_RAW_INT (24)
// GPS 原始数据（非融合定位）。
DEF_MAVMSG_BEGIN(GpsRawInt, 24)

    // 时间戳
    DEF_MAVMSG_MEMBER(time_usec, uint64_t);

    // 定位类型（0=无，2=2D，3=3D）
    DEF_MAVMSG_MEMBER(fix_type, GpsFixType);

    // 纬度（WGS84，度数×1e7）
    DEF_MAVMSG_MEMBER(lat, DegE7);

    // 经度（WGS84，度数×1e7）
    DEF_MAVMSG_MEMBER(lon, DegE7);

    // 海拔（毫米，MSL）
    DEF_MAVMSG_MEMBER(alt, int32_t);

    // 水平精度（厘米）
    DEF_MAVMSG_MEMBER(eph, uint16_t);

    // 垂直精度（厘米）
    DEF_MAVMSG_MEMBER(epv, uint16_t);

    // 地速（厘米/秒）
    DEF_MAVMSG_MEMBER(vel, int16_t);

    // 航向角（百分度）
    DEF_MAVMSG_MEMBER(cog, uint16_t);

    // 可见卫星数
    DEF_MAVMSG_MEMBER(satellites_visible, uint8_t);

DEF_MAVMSG_END


// SCALED_IMU (26)
// 通常9DOF传感器的缩放IMU读数（包含磁力计、陀螺仪、加速度计数据）。
DEF_MAVMSG_BEGIN(ScaledImu, 26)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // X轴加速度
    DEF_MAVMSG_MEMBER(xacc, int16_t);

    // Y轴加速度
    DEF_MAVMSG_MEMBER(yacc, int16_t);

    // Z轴加速度
    DEF_MAVMSG_MEMBER(zacc, int16_t);

    // X轴角速度
    DEF_MAVMSG_MEMBER(xgyro, int16_t);

    // Y轴角速度
    DEF_MAVMSG_MEMBER(ygyro, int16_t);

    // Z轴角速度
    DEF_MAVMSG_MEMBER(zgyro, int16_t);

    // X轴磁场强度
    DEF_MAVMSG_MEMBER(xmag, int16_t);

    // Y轴磁场强度
    DEF_MAVMSG_MEMBER(ymag, int16_t);

    // Z轴磁场强度
    DEF_MAVMSG_MEMBER(zmag, int16_t);

DEF_MAVMSG_END


// SCALED_PRESSURE (29)
// 气压数据（已缩放）。
DEF_MAVMSG_BEGIN(ScaledPressure, 29)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // 绝对气压
    DEF_MAVMSG_MEMBER(press_abs, fp32);

    // 差分压力（高度计）
    DEF_MAVMSG_MEMBER(press_diff, fp32);

    // 温度
    DEF_MAVMSG_MEMBER(temperature, int16_t);

DEF_MAVMSG_END


// ATTITUDE (30)
// 姿态数据（欧拉角+角速度）。
DEF_MAVMSG_BEGIN(Attitude, 30)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // 横滚角
    DEF_MAVMSG_MEMBER(roll, fp32);

    // 俯仰角
    DEF_MAVMSG_MEMBER(pitch, fp32);

    // 偏航角
    DEF_MAVMSG_MEMBER(yaw, fp32);

    // 横滚角速度
    DEF_MAVMSG_MEMBER(rollspeed, fp32);

    // 俯仰角速度
    DEF_MAVMSG_MEMBER(pitchspeed, fp32);

    // 偏航角速度
    DEF_MAVMSG_MEMBER(yawspeed, fp32);

DEF_MAVMSG_END


// LOCAL_POSITION_NED (32)
// 滤波的局部位置（例如融合计算机视觉和加速度计）。坐标系为右手，Z轴向下（航空框架，NED / 东北向下约定）
DEF_MAVMSG_BEGIN(LocalPositionNed, 32)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // x(m)
    DEF_MAVMSG_MEMBER(x, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // y(m)
    DEF_MAVMSG_MEMBER(y, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // z(m)
    DEF_MAVMSG_MEMBER(z, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vx, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vy, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vz(m/s)
    DEF_MAVMSG_MEMBER(vz, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));


DEF_MAVMSG_END



// GLOBAL_POSITION_INT (33)
// 融合后全局位置（经纬高+相对高度）。
DEF_MAVMSG_BEGIN(GlobalPositionInt, 33)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // 纬度（WGS84）
    DEF_MAVMSG_MEMBER(lat, int32_t);

    // 经度（WGS84）
    DEF_MAVMSG_MEMBER(lon, int32_t);

    // 海拔（MSL）
    DEF_MAVMSG_MEMBER(alt, int32_t);

    // 相对高度（起飞点）
    DEF_MAVMSG_MEMBER(relative_alt, int32_t);

    // 北向速度
    DEF_MAVMSG_MEMBER(vx, int16_t);

    // 东向速度
    DEF_MAVMSG_MEMBER(vy, int16_t);

    // 下向速度
    DEF_MAVMSG_MEMBER(vz, int16_t);

    // 航向角（百分度）
    DEF_MAVMSG_MEMBER(hdg, uint16_t);

DEF_MAVMSG_END


// RC_CHANNELS_RAW (35)
// 遥控器原始通道值（0-65535）。
DEF_MAVMSG_BEGIN(RcChannelsRaw, 35)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // RC接收机端口号（0-3）
    DEF_MAVMSG_MEMBER(port, uint8_t);

    // 通道1原始值
    DEF_MAVMSG_MEMBER_ARRAY(channels, uint16_t, 8);

    // 接收信号强度指示（0-100，0=无效）
    DEF_MAVMSG_MEMBER(rssi, uint8_t);

DEF_MAVMSG_END


// SERVO_OUTPUT_RAW (36)
// 伺服输出原始脉冲宽度值（0-65535）。
DEF_MAVMSG_BEGIN(ServoOutputRaw, 36)

    // 时间戳（微秒）
    DEF_MAVMSG_MEMBER(time_usec, uint32_t);

    // 端口号（0为MAIN，1为AUX）
    DEF_MAVMSG_MEMBER(port, uint8_t);

    // 伺服1输出脉冲宽度
    DEF_MAVMSG_MEMBER_ARRAY(servo_channels, uint16_t, 8);

DEF_MAVMSG_END


// MISSION_ITEM (39)
// 任务项（坐标、高度、动作等）。
DEF_MAVMSG_BEGIN(MissionItem, 39)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(seq, uint16_t);

    // 坐标系
    DEF_MAVMSG_MEMBER(frame, MavFrame);

    // 命令ID
    DEF_MAVMSG_MEMBER(command, uint16_t);

    // 0: 任务项，1: 当前任务
    DEF_MAVMSG_MEMBER(current, uint8_t);

    // 自动继续到下一任务
    DEF_MAVMSG_MEMBER(autocontinue, uint8_t);

    // 参数1
    DEF_MAVMSG_MEMBER(param1, fp32);

    // 参数2
    DEF_MAVMSG_MEMBER(param2, fp32);

    // 参数3
    DEF_MAVMSG_MEMBER(param3, fp32);

    // 参数4
    DEF_MAVMSG_MEMBER(param4, fp32);

    // X位置或参数
    DEF_MAVMSG_MEMBER(x, fp32);

    // Y位置或参数
    DEF_MAVMSG_MEMBER(y, fp32);

    // Z位置或参数
    DEF_MAVMSG_MEMBER(z, fp32);

DEF_MAVMSG_END


// MISSION_REQUEST (40)
// 请求特定任务项（用于任务上传/下载）。
DEF_MAVMSG_BEGIN(MissionRequest, 40)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(seq, uint16_t);

DEF_MAVMSG_END


// MISSION_SET_CURRENT (41)
// 设置当前任务（激活特定任务项）。
DEF_MAVMSG_BEGIN(MissionSetCurrent, 41)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 任务序号
    DEF_MAVMSG_MEMBER(seq, uint16_t);

DEF_MAVMSG_END


// MISSION_CURRENT (42)
// 当前执行的任务序号。
DEF_MAVMSG_BEGIN(MissionCurrent, 42)

    // 当前任务序号
    DEF_MAVMSG_MEMBER(seq, uint16_t);

DEF_MAVMSG_END


// MISSION_REQUEST_LIST (43)
// 请求任务列表信息（总任务数）。
DEF_MAVMSG_BEGIN(MissionRequestList, 43)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

DEF_MAVMSG_END


// MISSION_COUNT (44)
// 任务总数（响应任务列表请求）。
DEF_MAVMSG_BEGIN(MissionCount, 44)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 任务总数
    DEF_MAVMSG_MEMBER(count, uint16_t);

DEF_MAVMSG_END


// MISSION_CLEAR_ALL (45)
// 清除所有任务（删除任务列表）。
DEF_MAVMSG_BEGIN(MissionClearAll, 45)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

DEF_MAVMSG_END


// MISSION_ITEM_REACHED (46)
// 任务项已到达（报告任务执行进度）。
DEF_MAVMSG_BEGIN(MissionItemReached, 46)

    // 已到达的任务序号
    DEF_MAVMSG_MEMBER(seq, uint16_t);

DEF_MAVMSG_END


// MISSION_ACK (47)
// 任务操作确认（任务上传/下载结果）。
DEF_MAVMSG_BEGIN(MissionAck, 47)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 任务操作结果
    DEF_MAVMSG_MEMBER(type, MavMissionResult);

DEF_MAVMSG_END


// GPS_GLOBAL_ORIGIN (49)
// GPS全球原点（基准坐标系）的位置信息。
DEF_MAVMSG_BEGIN(GpsGlobalOrigin, 49)

    // 纬度
    DEF_MAVMSG_MEMBER(latitude, int32_t);

    // 经度
    DEF_MAVMSG_MEMBER(longitude, int32_t);

    // 海拔（毫米）
    DEF_MAVMSG_MEMBER(altitude, DEF_GENERIC_TYPENAME(unit::MilliMeter, int32_t));

    // 时间戳（微秒）
    DEF_MAVMSG_MEMBER(time_usec, DEF_GENERIC_TYPENAME(std::chrono::duration, uint64_t, std::micro));

DEF_MAVMSG_END



// RC_CHANNELS (65)
// 遥控器通道原始值（更新版本，支持更多通道）。
DEF_MAVMSG_BEGIN(RcChannels, 65)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    // 通道数量
    DEF_MAVMSG_MEMBER(chancount, uint8_t);

    // 通道1原始值
    DEF_MAVMSG_MEMBER_ARRAY(channels, uint16_t, 18);

    // 接收信号强度指示
    DEF_MAVMSG_MEMBER(rssi, uint8_t);

DEF_MAVMSG_END


// RC_CHANNELS_OVERRIDE (70)
// 遥控通道覆盖（地面站发送虚拟遥控值）。
DEF_MAVMSG_BEGIN(RcChannelsOverride, 70)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 通道1覆盖值
    DEF_MAVMSG_MEMBER_ARRAY(channels, uint16_t, 8);


DEF_MAVMSG_END


// COMMAND_INT (75)
// 向MAV发送一个包含最多七个参数的命令，其中参数5和6为整数，其他值为float。
// 这比COMMAND_LONG更受青睐，因为它允许指定MAV_FRAME来解释位置信息，如高度。
// COMMAND_INT在发送参数5和6的纬度和经度数据时也更受青睐，因为它能实现更高的精度。
// 参数5和6将位置数据编码为缩放整数，缩放依赖于实际命令值。NaN 或 INT32_MAX 分别可用于浮点/整数参数，
// 表示可选/默认值（例如使用组件当前纬度、偏航而非特定值）。
// 命令微服务文档见 https://mavlink.io/en/services/command.html
DEF_MAVMSG_BEGIN(CommandInt, 75)
    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 命令ID
    DEF_MAVMSG_MEMBER(frame, MavFrame);

    DEF_MAVMSG_MEMBER(command, MavCmd);

    //not used
    DEF_MAVMSG_MEMBER(current, uint8_t);
    
    //not used
    DEF_MAVMSG_MEMBER(autocontinue, uint8_t);

    // 参数1
    DEF_MAVMSG_MEMBER(param1, fp32);

    // 参数2
    DEF_MAVMSG_MEMBER(param2, fp32);

    // 参数3
    DEF_MAVMSG_MEMBER(param3, fp32);

    // 参数4
    DEF_MAVMSG_MEMBER(param4, fp32);

    // 本地：x 位置（米 * 1e4），全球：纬度（度数）* 10^7
    // invalid:INT32_MAX
    DEF_MAVMSG_MEMBER(x, int32_t);

    // 本地：x 位置（米 * 1e4），全球：经度（度数）* 10^7
    // invalid:INT32_MAX
    DEF_MAVMSG_MEMBER(y, int32_t);

    // 全局高度：以米为单位（相对或绝对，取决于帧
    DEF_MAVMSG_MEMBER(z, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

DEF_MAVMSG_END



// COMMAND_LONG (76)
// 向MAV发送最多七个参数的命令。COMMAND_INT通常在发送包含位置信息的MAV_CMD命令时更受青睐;
// 它提供了更高的精度，并允许指定MAV_FRAME（否则这些可能存在歧义，尤其是高度方面）。
// 命令微服务文档见 https://mavlink.io/en/services/command.html
DEF_MAVMSG_BEGIN(CommandLong, 76)

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    // 命令ID
    DEF_MAVMSG_MEMBER(command, uint16_t);

    // 确认次数（0为首次发送）
    DEF_MAVMSG_MEMBER(confirmation, uint8_t);

    // 参数1
    DEF_MAVMSG_MEMBER_ARRAY(params, fp32, 7);

DEF_MAVMSG_END


// COMMAND_ACK (77)
// 命令确认（命令执行结果）。
DEF_MAVMSG_BEGIN(CommandAck, 77)

    // 命令ID
    DEF_MAVMSG_MEMBER(command, uint16_t);

    // 命令执行结果
    DEF_MAVMSG_MEMBER(result, MavResult);

    // 进度百分比（0-100，255表示不适用）
    DEF_MAVMSG_MEMBER(progress, uint8_t);

    // 附加结果参数
    DEF_MAVMSG_MEMBER(result_param2, int32_t);

    // 目标系统ID
    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    // 目标组件ID
    DEF_MAVMSG_MEMBER(target_component, uint8_t);

DEF_MAVMSG_END

template<typename T>
struct Radians{
    T count;
};

// SET_POSITION_TARGET_LOCAL_NED (84)
// 在本地东北下坐标系中设定目标车辆位置。由外部控制器用于指挥车辆（手动控制器或其他系统）。
DEF_MAVMSG_BEGIN(SetPositionTargetLocalNed, 84)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    DEF_MAVMSG_MEMBER(target_system, uint8_t);

    DEF_MAVMSG_MEMBER(target_component, uint8_t);

    DEF_MAVMSG_MEMBER(coordinate_frame, MavFrame);

    DEF_MAVMSG_MEMBER(type_mask, PositionTargetTypemask);
    

    // x(m)
    DEF_MAVMSG_MEMBER(x, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // y(m)
    DEF_MAVMSG_MEMBER(y, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // z(m)
    DEF_MAVMSG_MEMBER(z, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vx, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vy, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vz(m/s)
    DEF_MAVMSG_MEMBER(vz, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afx, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afy, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afz, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));
    
    // yaw setpoint
    DEF_MAVMSG_MEMBER(yaw, DEF_GENERIC_TYPENAME(Radians, fp32));

    // yaw setpoint
    DEF_MAVMSG_MEMBER(yaw_rate, DEF_GENERIC_TYPENAME(Radians, fp32));


DEF_MAVMSG_END


// POSITION_TARGET_GLOBAL_INT (87)
DEF_MAVMSG_BEGIN(PositionTargetGlobalInt, 87)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    DEF_MAVMSG_MEMBER(coordinate_frame, MavFrame);

    DEF_MAVMSG_MEMBER(type_mask, PositionTargetTypemask);

    DEF_MAVMSG_MEMBER(lat_int, DegE7);

    DEF_MAVMSG_MEMBER(lon_int, DegE7);

    DEF_MAVMSG_MEMBER(alt, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vx, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vx(m/s)
    DEF_MAVMSG_MEMBER(vy, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // vz(m/s)
    DEF_MAVMSG_MEMBER(vz, DEF_GENERIC_TYPENAME(unit::MetersPerSecond, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afx, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afy, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));

    // (m/s^2)
    DEF_MAVMSG_MEMBER(afz, DEF_GENERIC_TYPENAME(unit::MetersPerSecondSquared, fp32));
    
    // yaw setpoint
    DEF_MAVMSG_MEMBER(yaw, DEF_GENERIC_TYPENAME(Radians, fp32));

    // yaw setpoint
    DEF_MAVMSG_MEMBER(yaw_rate, DEF_GENERIC_TYPENAME(Radians, fp32));


DEF_MAVMSG_END




// LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET (89)
DEF_MAVMSG_BEGIN(LocalPositionNedSystemGlobalOffset, 89)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_boot_ms, uint32_t);

    DEF_MAVMSG_MEMBER(coordinate_frame, MavFrame);

    DEF_MAVMSG_MEMBER(type_mask, PositionTargetTypemask);

    DEF_MAVMSG_MEMBER(lat_int, DegE7);

    DEF_MAVMSG_MEMBER(lon_int, DegE7);

    DEF_MAVMSG_MEMBER(alt, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // x(m)
    DEF_MAVMSG_MEMBER(x, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // y(m)
    DEF_MAVMSG_MEMBER(y, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    // z(m)
    DEF_MAVMSG_MEMBER(z, DEF_GENERIC_TYPENAME(unit::Meter, fp32));

    DEF_MAVMSG_MEMBER(roll, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(pitch, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(yaw, DEF_GENERIC_TYPENAME(Radians, fp32));

DEF_MAVMSG_END



// HIL_STATE (90)
DEF_MAVMSG_BEGIN(HilState, 90)

    // 启动时间戳
    DEF_MAVMSG_MEMBER(time_usec, DEF_GENERIC_TYPENAME(std::chrono::duration, uint64_t, std::micro));

    DEF_MAVMSG_MEMBER(roll, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(pitch, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(yaw, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(rollspeed, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(pitchspeed, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(yawspeed, DEF_GENERIC_TYPENAME(Radians, fp32));

    DEF_MAVMSG_MEMBER(lat, DegE7);

    DEF_MAVMSG_MEMBER(lon, DegE7);

    DEF_MAVMSG_MEMBER(alt, DEF_GENERIC_TYPENAME(unit::MilliMeter, int32_t));

    DEF_MAVMSG_MEMBER(vx, DEF_GENERIC_TYPENAME(unit::CentiMetersPerSecond, int16_t));

    DEF_MAVMSG_MEMBER(vy, DEF_GENERIC_TYPENAME(unit::CentiMetersPerSecond, int16_t));

    DEF_MAVMSG_MEMBER(vz, DEF_GENERIC_TYPENAME(unit::CentiMetersPerSecond, int16_t));


    struct mG{
        int16_t bits;
    };


    DEF_MAVMSG_MEMBER(xacc, mG);

    DEF_MAVMSG_MEMBER(yacc, mG);

    DEF_MAVMSG_MEMBER(zacc, mG);
DEF_MAVMSG_END



// ACTUATOR_CONTROL_TARGET (140)
// 执行器控制目标（姿态+油门）。
DEF_MAVMSG_BEGIN(ActuatorControlTarget, 140)

    // 时间戳
    DEF_MAVMSG_MEMBER(time_usec, uint64_t);

    // 执行器组 ID（多实例区分）
    DEF_MAVMSG_MEMBER(group_mlx, uint8_t);

    // 控制量（-1~+1：横滚、俯仰、偏航、油门、襟翼、扰流板、刹车、起落架）
    DEF_MAVMSG_MEMBER_ARRAY(controls, fp32, 8);

DEF_MAVMSG_END


// ALTITUDE (141)
// 多维度高度数据（单调/绝对/相对/地形）。
DEF_MAVMSG_BEGIN(Altitude, 141)

    // 时间戳
    DEF_MAVMSG_MEMBER(time_usec, uint64_t);

    // 单调高度（启动后不重置）
    DEF_MAVMSG_MEMBER(altitude_monotonic, fp32);

    // 绝对海拔（MSL）
    DEF_MAVMSG_MEMBER(altitude_amsl, fp32);

    // 本地坐标系高度
    DEF_MAVMSG_MEMBER(altitude_local, fp32);

    // 相对起飞点高度
    DEF_MAVMSG_MEMBER(altitude_relative, fp32);

    // 相对地形高度（<-1000=无效）
    DEF_MAVMSG_MEMBER(altitude_terrain, fp32);

    // 离地间隙（负=无效）
    DEF_MAVMSG_MEMBER(bottom_clearance, fp32);

DEF_MAVMSG_END


// BATTERY_STATUS (147)
// 电池详细状态（多电芯电池）。
DEF_MAVMSG_BEGIN(BatteryStatus, 147)

    // 电池ID
    DEF_MAVMSG_MEMBER(id, uint8_t);

    // 电池功能
    DEF_MAVMSG_MEMBER(battery_function, MavBatteryFunction);

    // 电池类型
    DEF_MAVMSG_MEMBER(type, MavBatteryType);

    // 电池温度
    DEF_MAVMSG_MEMBER(temperature, int16_t);

    // 电芯电压数组
    DEF_MAVMSG_MEMBER_ARRAY(voltages, uint16_t, 10);

    // 电池电流
    DEF_MAVMSG_MEMBER(current_battery, int16_t);

    // 已消耗电流
    DEF_MAVMSG_MEMBER(current_consumed, int32_t);

    // 已消耗能量
    DEF_MAVMSG_MEMBER(energy_consumed, int32_t);

    // 剩余电量百分比
    DEF_MAVMSG_MEMBER(battery_remaining, int8_t);

    // 剩余使用时间
    DEF_MAVMSG_MEMBER(time_remaining, uint32_t);

    // 充电状态
    DEF_MAVMSG_MEMBER(charge_state, MavBatteryChargeState);

DEF_MAVMSG_END


// AUTOPILOT_VERSION (181)
// 自动驾驶仪版本信息（固件版本、能力等）。
DEF_MAVMSG_BEGIN(AutopilotVersion, 181)

    // 自动驾驶仪能力位图
    DEF_MAVMSG_MEMBER(capabilities, uint64_t);

    // 飞行软件版本
    DEF_MAVMSG_MEMBER(flight_sw_version, uint32_t);

    // 中间件软件版本
    DEF_MAVMSG_MEMBER(middleware_sw_version, uint32_t);

    // 操作系统版本
    DEF_MAVMSG_MEMBER(os_sw_version, uint32_t);

    // 板卡硬件版本
    DEF_MAVMSG_MEMBER(board_version, uint32_t);

    // 飞行软件自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(flight_custom_version, uint8_t, 8);

    // 中间件自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(middleware_custom_version, uint8_t, 8);

    // 操作系统自定义版本
    DEF_MAVMSG_MEMBER_ARRAY(os_custom_version, uint8_t, 8);

    // 供应商ID
    DEF_MAVMSG_MEMBER(vendor_id, int16_t);

    // 产品ID
    DEF_MAVMSG_MEMBER(product_id, int16_t);

    // 硬件UID
    DEF_MAVMSG_MEMBER(uid, uint64_t);

    // 扩展硬件UID
    DEF_MAVMSG_MEMBER_ARRAY(uid2, uint8_t, 18);

DEF_MAVMSG_END


// HIGH_LATENCY2 (235)
// 高延迟链路精简遥测（替代旧版HIGH_LATENCY，适用于卫星或LoRa等低带宽链路）。
DEF_MAVMSG_BEGIN(HighLatency2, 235)

    // 时间戳（自纪元以来的秒数）
    DEF_MAVMSG_MEMBER(timestamp, uint32_t);

    // 载具类型
    DEF_MAVMSG_MEMBER(type, MavType);

    // 自动驾驶仪类型
    DEF_MAVMSG_MEMBER(autopilot, MavAutopilot);

    // 航向角
    DEF_MAVMSG_MEMBER(heading, uint16_t);

    // 纬度
    DEF_MAVMSG_MEMBER(latitude, int32_t);

    // 经度
    DEF_MAVMSG_MEMBER(longitude, int32_t);

    // 海拔高度
    DEF_MAVMSG_MEMBER(altitude, int16_t);

    // 目标高度
    DEF_MAVMSG_MEMBER(target_altitude, int16_t);

    // 目标纬度
    DEF_MAVMSG_MEMBER(latitude_int, int32_t);

    // 目标经度
    DEF_MAVMSG_MEMBER(longitude_int, int32_t);

    // 目标航向
    DEF_MAVMSG_MEMBER(target_heading, uint8_t);

    // 到目标距离
    DEF_MAVMSG_MEMBER(target_distance, uint16_t);

    // 油门百分比
    DEF_MAVMSG_MEMBER(throttle, uint8_t);

    // 空速
    DEF_MAVMSG_MEMBER(airspeed, uint8_t);

    // 空速设定点
    DEF_MAVMSG_MEMBER(airspeed_sp, uint8_t);

    // 地速
    DEF_MAVMSG_MEMBER(groundspeed, uint8_t);

    // 风速
    DEF_MAVMSG_MEMBER(windspeed, uint8_t);

    // 风向
    DEF_MAVMSG_MEMBER(wind_direction, uint8_t);

    // GPS水平精度
    DEF_MAVMSG_MEMBER(eph, uint8_t);

    // GPS垂直精度
    DEF_MAVMSG_MEMBER(epv, uint8_t);

    // 空气温度
    DEF_MAVMSG_MEMBER(temperature_air, int8_t);

    // 爬升率
    DEF_MAVMSG_MEMBER(climb_rate, int8_t);

    // 电池百分比
    DEF_MAVMSG_MEMBER(battery, int8_t);

    // 自定义字段0
    DEF_MAVMSG_MEMBER(custom0, int8_t);

    // 自定义字段1
    DEF_MAVMSG_MEMBER(custom1, int8_t);

    // 自定义字段2
    DEF_MAVMSG_MEMBER(custom2, uint8_t);

DEF_MAVMSG_END


} // namespace ymd::mavlink::msgs