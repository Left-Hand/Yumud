#pragma once

#include "core/utils/bits/bitfield_proxy.hpp"
#include "core/math/float/fp32.hpp"

namespace ymd::mavlink{


#define DEF_PROPERTY_BFPROXY(p_name, start_bit, stop_bit, p_type_name, bits)\
template <typename Self> [[nodiscard]] constexpr auto p_name(this Self && self) {\
    return ymd::make_bitfield_proxy<start_bit, stop_bit, p_type_name>(bits);}

#define DEF_PROPERTY_BIT(p_name, start_bit, bits) \
    DEF_PROPERTY_BFPROXY(p_name, start_bit, (start_bit + 1), bool, bits)


struct [[nodiscard]] HlFailureFlag final{
    uint16_t bits;

    // GPS 故障
    DEF_PROPERTY_BIT(gps, 0, &self.bits)

    // 差分压力传感器故障
    DEF_PROPERTY_BIT(differential_pressure, 1, &self.bits)

    // 绝对压力传感器故障
    DEF_PROPERTY_BIT(absolute_pressure, 2, &self.bits)

    // 3D 加速度计传感器故障
    DEF_PROPERTY_BIT(accel3d, 3, &self.bits)

    // 3D 陀螺仪传感器故障
    DEF_PROPERTY_BIT(gyro3d, 4, &self.bits)

    // 3D 磁力计传感器故障
    DEF_PROPERTY_BIT(mag3d, 5, &self.bits)

    // 地形子系统故障
    DEF_PROPERTY_BIT(terrain, 6, &self.bits)

    // 电池故障或严重低电量
    DEF_PROPERTY_BIT(battery, 7, &self.bits)

    // RC 接收机故障或无 RC 连接
    DEF_PROPERTY_BIT(rc_receiver, 8, &self.bits)

    // 机外链路（offboard link）故障
    DEF_PROPERTY_BIT(offboard_link, 9, &self.bits)

    // 发动机故障
    DEF_PROPERTY_BIT(engine, 10, &self.bits)

    // 地理围栏违规
    DEF_PROPERTY_BIT(geofence, 11, &self.bits)

    // 估计器故障，例如测量被拒绝或出现大偏差
    DEF_PROPERTY_BIT(estimator, 12, &self.bits)

    // 任务故障
    DEF_PROPERTY_BIT(mission, 13, &self.bits)

};


struct [[nodiscard]] MavGoto final{
    enum class [[nodiscard]] Kind:uint8_t{
        DoHold = 0, //在当前位置悬停
        DoContinue = 1, //继续执行任务中的下一项
        HoldAtCurrentPosition = 2, //在系统当前位置悬停
        HoldAtSpecifiedPosition = 3 //在DO_HOLD动作参数中指定的位置悬停
    };

    using enum Kind;

    Kind kind;
};


// MAV_TYPE（载具类型）
enum class [[nodiscard]] MavType : uint8_t {
    Generic = 0,              // 通用未知类型
    FixedWing = 1,            // 固定翼
    Quadrotor = 2,            // 四轴多旋翼
    Coaxial = 3,              // 共轴双桨
    Helicopter = 4,           // 直升机
    AntennaTracker = 5,       // 天线跟踪器
    Gcs = 6,                  // 地面站
    Airship = 7,              // 飞艇
    FreeBalloon = 8,          // 自由气球
    Rocket = 9,               // 火箭
    GroundRover = 10,         // 地面无人车
    SurfaceBoat = 11,         // 水面无人船
    Submarine = 12,           // 潜水器
    Hexarotor = 13,           // 六轴飞行器
    Octorotor = 14,           // 八轴飞行器
    Tricopter = 15,           // 三轴飞行器
    FlappingWing = 16,        // 扑翼机
    Kite = 17,                // 风筝
    OnboardController = 18,   // 机载控制器
    VtolDuorotor = 19,        // VTOL双旋翼
    VtolQuadrotor = 20,       // VTOL四旋翼
    VtolTiltrotor = 21,       // VTOL倾转旋翼
    VtolFixedRotor = 22,      // VTOL固定旋翼
    VtolTailsitter = 23,      // VTOL尾坐式
    VtolTiltwing = 24,        // VTOL倾转翼
    VtolReserved5 = 25,       // VTOL保留类型5
    Gimbal = 26,              // 云台
    Adsb = 27,                // ADSB系统
    Parafoil = 28,            // 降落伞
    Dodecarotor = 29,         // 十二轴飞行器
    Camera = 30,              // 相机
    ChargingStation = 31,     // 充电站
    Flarm = 32,               // FLARM防撞系统
    Servo = 33,               // 伺服机构
    Odid = 34,                // 广播式开放遥测ID
    Decarotor = 35,           // 十轴飞行器
    Battery = 36,             // 电池
    Parachute = 37,           // 降落伞
    Log = 38,                 // 日志
    Osd = 39,                 // OSD叠加显示器
    Imu = 40,                 // 惯性测量单元
    Gps = 41,                 // GPS
    Winch = 42,               // 绞盘
};


// MAV_AUTOPILOT（自动驾驶仪类型）
enum class [[nodiscard]] MavAutopilot : uint8_t {
    Generic = 0,              // 通用自动驾驶仪
    Reserved = 1,             // 保留
    Slugs = 2,                // SLUGS自动驾驶仪
    ArduPilotMega = 3,        // ArduPilotMega/AC3
    OpenPilot = 4,            // OpenPilot
    GenericWaypointsOnly = 5, // 仅支持航点的通用自动驾驶仪
    GenericWaypointsSimpleNav = 6, // 仅支持航点和简单导航的通用自动驾驶仪
    Pixhawk = 7,              // Pixhawk
    AutoQuad = 8,             // AutoQuad
    ArmaZila = 9,             // ArmaZila
    Px4 = 10,                 // PX4
    Sensefly = 11,            // SenseFly
    Test = 12,                // 测试
    Udb = 13,                 // UDB
    Fp = 14,                  // FlexiPilot
    Px4Adaptive = 15,         // PX4自适应控制
    Avex = 16,                // AvEx
    Argentum = 17,            // Argentum
    Urus = 18,                // Urus
    Kk = 19,                  // KK
    EnumEnd = 20,             // 枚举结束标志
};


// MAV_STATE（系统状态）
enum class [[nodiscard]] MavState : uint8_t {
    Uninit = 0,       // 未初始化
    Boot = 1,         // 启动中
    Calibrating = 2,  // 校准中
    Standby = 3,      // 待机
    Active = 4,       // 正常飞行
    Critical = 5,     // 临界故障
    Emergency = 6,    // 紧急故障
    PowerOff = 7,     // 关机
    FlightTermination = 8, // 飞行终止
};


// MAV_MODE_FLAG（基础模式位）
struct [[nodiscard]] MavModeFlag final {
    uint8_t bits;

    DEF_PROPERTY_BIT(armed, 0, &self.bits)                   // 安全解锁（可飞行）
    DEF_PROPERTY_BIT(manual_input_enabled, 1, &self.bits)    // 手动控制使能
    DEF_PROPERTY_BIT(hil_enabled, 2, &self.bits)             // 硬件在环仿真使能
    DEF_PROPERTY_BIT(stabilize_enabled, 3, &self.bits)       // 增稳模式使能
    DEF_PROPERTY_BIT(guided_enabled, 4, &self.bits)          // 引导模式使能
    DEF_PROPERTY_BIT(auto_enabled, 5, &self.bits)            // 自动模式使能
    DEF_PROPERTY_BIT(test_enabled, 6, &self.bits)            // 测试模式使能
    DEF_PROPERTY_BIT(custom_mode_enabled, 7, &self.bits)     // 自定义模式使能
};


// MAV_MODE（预定义模式组合）
enum class [[nodiscard]] MavMode : uint8_t {
    Preflight = 0,                    // 预飞状态（未准备飞行）
    ManualDisarmed = 64,              // 手动模式（未解锁）
    StabilizeDisarmed = 80,           // 增稳模式（未解锁）
    GuidedDisarmed = 88,              // 引导模式（未解锁）
    AutoDisarmed = 92,                // 自动模式（未解锁）
    ManualArmed = 192,                // 手动模式（已解锁）
    StabilizeArmed = 208,             // 增稳模式（已解锁）
    GuidedArmed = 216,                // 引导模式（已解锁）
    AutoArmed = 220,                  // 自动模式（已解锁）
    TestDisarmed = 66,                // 测试模式（未解锁）
    TestArmed = 194,                  // 测试模式（已解锁）
};


// GPS_FIX_TYPE（GPS定位类型）
enum class [[nodiscard]] GpsFixType : uint8_t {
    NoGps = 0,        // 无GPS
    NoFix = 1,        // 无定位
    Fix2D = 2,        // 2D定位
    Fix3D = 3,        // 3D定位
    Dgps = 4,         // 差分GPS
    RtkFloat = 5,     // RTK浮点解
    RtkFixed = 6,     // RTK固定解
    Static = 7,       // 静态定位
    Ppp = 8,          // 精密单点定位
};


// MAV_MISSION_RESULT（任务操作结果）
enum class [[nodiscard]] MavMissionResult : uint8_t {
    Accepted = 0,             // 任务接受
    Error = 1,                // 任务错误
    UnsupportedFrame = 2,     // 不支持的坐标系
    Unsupported = 3,          // 不支持的任务
    NoSpace = 4,              // 空间不足
    Invalid = 5,              // 无效任务
    InvalidParam1 = 6,        // 参数1无效
    InvalidParam2 = 7,        // 参数2无效
    InvalidParam3 = 8,        // 参数3无效
    InvalidParam4 = 9,        // 参数4无效
    InvalidParam5X = 10,      // 参数5(X)无效
    InvalidParam6Y = 11,      // 参数6(Y)无效
    InvalidParam7 = 12,       // 参数7无效
    InvalidSequence = 13,     // 序列无效
    Denied = 14,              // 任务被拒绝
    OperationCancelled = 15,  // 操作已取消
};


// MAV_FRAME（坐标系）
enum class [[nodiscard]] MavFrame : uint8_t {
    Global = 0,                           // 全球坐标系（WGS84）
    LocalNed = 1,                         // 本地NED坐标系
    Mission = 2,                          // 任务坐标系
    GlobalRelativeAlt = 3,                // 全球相对高度坐标系
    LocalEnu = 4,                         // 本地ENU坐标系
    GlobalInt = 5,                        // 全球整数坐标系
    GlobalRelativeAltInt = 6,             // 全球相对高度整数坐标系
    LocalOffsetNed = 7,                   // 本地偏移NED坐标系
    BodyNed = 8,                          // 机体NED坐标系
    BodyOffsetNed = 9,                    // 机体偏移NED坐标系
    GlobalTerrainAlt = 10,                // 全球地形高度坐标系
    GlobalTerrainAltInt = 11,             // 全球地形高度整数坐标系
    BodyFrd = 12,                         // 机体FRD坐标系
    Reserved13 = 13,                      // 保留
    LocalFrd = 14,                        // 本地FRD坐标系
    LocalFlu = 15,                        // 本地FLU坐标系
};


// MAV_RESULT（命令执行结果）
enum class [[nodiscard]] MavResult : uint8_t {
    Accepted = 0,              // 接受
    TemporarilyRejected = 1,   // 临时拒绝
    Denied = 2,                // 拒绝
    Unsupported = 3,           // 不支持
    Failed = 4,                // 失败
    InProgress = 5,            // 进行中
    Cancelled = 6,             // 已取消
};


// MAV_BATTERY_FUNCTION（电池功能）
enum class [[nodiscard]] MavBatteryFunction : uint8_t {
    Unknown = 0,      // 未知
    All = 1,          // 所有功能
    Propulsion = 2,   // 推进
    Avionics = 3,     // 航电
    HotSwap = 4,      // 热插拔（电池类型）
};


// MAV_BATTERY_TYPE（电池类型）
enum class [[nodiscard]] MavBatteryType : uint8_t {
    Unknown = 0,  // 未知
    Lipo = 1,     // 锂聚合物
    Life = 2,     // 锂铁
    Lion = 3,     // 锂离子
    Nimh = 4,     // 镍氢
};


// MAV_BATTERY_CHARGE_STATE（充电状态）
enum class [[nodiscard]] MavBatteryChargeState : uint8_t {
    Undefined = 0,  // 未定义
    Ok = 1,         // 正常
    Low = 2,        // 低电量
    Critical = 3,   // 危险
    Emergency = 4,  // 紧急
    Failed = 5,     // 故障
    Unhealthy = 6,  // 不健康
    Charging = 7,   // 充电中
};


// MAV_PARAM_TYPE（参数类型）
enum class [[nodiscard]] MavParamType : uint8_t {
    Uint8 = 1,    // 无符号8位整数
    Int8 = 2,     // 有符号8位整数
    Uint16 = 3,   // 无符号16位整数
    Int16 = 4,    // 有符号16位整数
    Uint32 = 5,   // 无符号32位整数
    Int32 = 6,    // 有符号32位整数
    Uint64 = 7,   // 无符号64位整数
    Int64 = 8,    // 有符号64位整数
    Real32 = 9,   // 32位浮点数
    Real64 = 10,  // 64位浮点数
};


// MAV_SYS_STATUS_SENSOR（传感器状态位图）
struct [[nodiscard]] MavSysStatusSensor final {
    uint32_t bits;

    DEF_PROPERTY_BIT(sensor_3d_gyro, 0, &self.bits)                    // 3D陀螺仪
    DEF_PROPERTY_BIT(sensor_3d_accel, 1, &self.bits)                   // 3D加速度计
    DEF_PROPERTY_BIT(sensor_3d_mag, 2, &self.bits)                     // 3D磁力计
    DEF_PROPERTY_BIT(sensor_absolute_pressure, 3, &self.bits)          // 绝对压力
    DEF_PROPERTY_BIT(sensor_differential_pressure, 4, &self.bits)      // 差分压力
    DEF_PROPERTY_BIT(sensor_gps, 5, &self.bits)                        // GPS
    DEF_PROPERTY_BIT(sensor_optical_flow, 6, &self.bits)               // 光流
    DEF_PROPERTY_BIT(sensor_vision_position, 7, &self.bits)            // 计算机视觉位置
    DEF_PROPERTY_BIT(sensor_laser_position, 8, &self.bits)             // 激光定位
    DEF_PROPERTY_BIT(sensor_external_ground_truth, 9, &self.bits)      // 外部地面真值
    DEF_PROPERTY_BIT(sensor_angular_rate_control, 10, &self.bits)      // 3D角速率控制
    DEF_PROPERTY_BIT(sensor_attitude_stabilization, 11, &self.bits)     // 姿态稳定
    DEF_PROPERTY_BIT(sensor_yaw_position, 12, &self.bits)              // 偏航位置
    DEF_PROPERTY_BIT(sensor_z_altitude_control, 13, &self.bits)        // Z/高度控制
    DEF_PROPERTY_BIT(sensor_xy_position_control, 14, &self.bits)       // XY位置控制
    DEF_PROPERTY_BIT(sensor_motor_outputs, 15, &self.bits)             // 电机输出/控制
    DEF_PROPERTY_BIT(sensor_rc_receiver, 16, &self.bits)               // RC接收机
    DEF_PROPERTY_BIT(sensor_3d_gyro2, 17, &self.bits)                  // 第二个3D陀螺仪
    DEF_PROPERTY_BIT(sensor_3d_accel2, 18, &self.bits)                 // 第二个3D加速度计
    DEF_PROPERTY_BIT(sensor_3d_mag2, 19, &self.bits)                   // 第二个3D磁力计
    DEF_PROPERTY_BIT(geofence, 20, &self.bits)                         // 地理围栏
    DEF_PROPERTY_BIT(ahrs, 21, &self.bits)                             // AHRS子系统健康状况
    DEF_PROPERTY_BIT(terrain, 22, &self.bits)                          // 地形子系统健康状况
    DEF_PROPERTY_BIT(reverse_motor, 23, &self.bits)                    // 电机反转
    DEF_PROPERTY_BIT(logging, 24, &self.bits)                          // 日志记录
    DEF_PROPERTY_BIT(sensor_battery, 25, &self.bits)                   // 电池
    DEF_PROPERTY_BIT(sensor_proximity, 26, &self.bits)                 // 接近传感器
    DEF_PROPERTY_BIT(sensor_satcom, 27, &self.bits)                    // 卫星通信
    DEF_PROPERTY_BIT(prearm_check, 28, &self.bits)                     // 预解锁检查状态
    DEF_PROPERTY_BIT(obstacle_avoidance, 29, &self.bits)               // 避障/碰撞预防
    DEF_PROPERTY_BIT(sensor_propulsion, 30, &self.bits)                // 推进（执行器、ESC、电机或螺旋桨）
    DEF_PROPERTY_BIT(extension_used, 31, &self.bits)                   // 扩展位用于进一步的传感器状态位
};


// FENCE_BREACH（围栏违规类型）
enum class [[nodiscard]] FenceBreach : uint8_t {
    None = 0,       // 无最后围栏违规
    MinAlt = 1,     // 违规最小高度
    MaxAlt = 2,     // 违规最大高度
    Boundary = 3,   // 违规围栏边界
};


// FENCE_MITIGATE（围栏缓解措施）
enum class [[nodiscard]] FenceMitigate : uint8_t {
    Unknown = 0,    // 未知
    None = 1,       // 未采取任何行动
    VelLimit = 2,   // 激活速度限制以防止违规
};


// FENCE_TYPE（围栏类型）
struct [[nodiscard]] FenceType final {
    uint8_t bits;

    DEF_PROPERTY_BIT(alt_max, 0, &self.bits)    // 最大高度围栏
    DEF_PROPERTY_BIT(circle, 1, &self.bits)     // 圆形围栏
    DEF_PROPERTY_BIT(polygon, 2, &self.bits)    // 多边形围栏
    DEF_PROPERTY_BIT(alt_min, 3, &self.bits)    // 最小高度围栏
};


// MAV_MOUNT_MODE（云台模式）
enum class [[nodiscard]] MavMountMode : uint8_t {
    Retract = 0,            // 从永久内存加载并保持安全位置（Roll,Pitch,Yaw）并停止稳定
    Neutral = 1,            // 从永久内存加载并保持中性位置（Roll,Pitch,Yaw）
    MavlinkTargeting = 2,   // 加载中性位置并开始MAVLink Roll,Pitch,Yaw控制并稳定
    RcTargeting = 3,        // 加载中性位置并开始RC Roll,Pitch,Yaw控制并稳定
    GpsPoint = 4,           // 加载中性位置并开始指向Lat,Lon,Alt
    SysIdTarget = 5,        // 云台跟踪具有指定系统ID的系统
    HomeLocation = 6,       // 云台跟踪家庭位置
    WpNextOffset = 7,       // 云台跟踪下一个航路点位置并带偏移
};


// GIMBAL_DEVICE_CAP_FLAGS（云台设备能力标志）
struct [[nodiscard]] GimbalDeviceCapFlags final {
    uint16_t bits;

    DEF_PROPERTY_BIT(has_retract, 0, &self.bits)   // 云台设备支持收回位置
};


// MAVLINK_DATA_STREAM_TYPE（数据流类型）
enum class [[nodiscard]] MavlinkDataStreamType : uint8_t {
    ImgJpeg = 0,    // JPEG图像
    ImgBmp = 1,     // BMP图像
    ImgRaw8U = 2,   // 8位原始图像
    ImgRaw32U = 3,  // 32位原始图像
    ImgPgm = 4,     // PGM图像
    ImgPng = 5,     // PNG图像
};




// MAVLINK心跳消息中报告的组件类型。飞控必须报告其安装的飞行器类型（例如MAV_TYPE_OCTOROTOR）。
// 所有其他组件必须报告适合其类型的值（例如相机必须使用MAV_TYPE_CAMERA）。
// MAVLINK component type reported in HEARTBEAT message. Flight controllers must report the type of the 
// vehicle on which they are mounted (e.g. MAV_TYPE_OCTOROTOR). 
// All other components must report a value appropriate for their type (e.g. a camera must use MAV_TYPE_CAMERA).
enum class [[nodiscard]] MavComponentType : uint8_t {
    Generic = 0,                    ///< 通用微型飞行器 / Generic micro air vehicle
    FixedWing = 1,                  ///< 固定翼飞机 / Fixed wing aircraft
    Quadrotor = 2,                  ///< 四旋翼 / Quadrotor
    Coaxial = 3,                    ///< 共轴直升机 / Coaxial helicopter
    Helicopter = 4,                 ///< 带尾桨的普通直升机 / Normal helicopter with tail rotor
    AntennaTracker = 5,             ///< 地面装置 / Ground installation
    Gcs = 6,                        ///< 操作员控制单元/地面站 / Operator control unit / ground control station
    Airship = 7,                    ///< 受控飞艇 / Airship, controlled
    FreeBalloon = 8,                ///< 自由气球，不受控 / Free balloon, uncontrolled
    Rocket = 9,                     ///< 火箭 / Rocket
    GroundRover = 10,               ///< 地面漫游车 / Ground rover
    SurfaceBoat = 11,               ///< 水面船只，船，舰 / Surface vessel, boat, ship
    Submarine = 12,                 ///< 潜艇 / Submarine
    Hexarotor = 13,                 ///< 六旋翼 / Hexarotor
    Octocopter = 14,                ///< 八旋翼 / Octorotor (renamed from Octocopter for consistency)
    Tricopter = 15,                 ///< 三旋翼 / Tricopter
    FlappingWing = 16,              ///< 扑翼机 / Flapping wing
    Kite = 17,                      ///< 风筝 / Kite
    OnboardController = 18,         ///< 机载伴随控制器 / Onboard companion controller
    VtolTailsitterDuorotor = 19,    ///< 双旋翼尾坐式VTOL，垂直运行时额外使用控制面 / Two-rotor Tailsitter VTOL that additionally uses control surfaces in vertical operation
    VtolTailsitterQuadrotor = 20,   ///< 四旋翼尾坐式VTOL，垂直运行时使用V形四配置 / Quad-rotor Tailsitter VTOL using a V-shaped quad config in vertical operation
    VtolTiltrotor = 21,             ///< 倾转旋翼VTOL / Tiltrotor VTOL
    VtolFixedrotor = 22,            ///< 垂直起降固定旋翼 / VTOL with separate fixed rotors for hover and cruise flight
    VtolTailsitter = 23,            ///< 尾坐式VTOL / Tailsitter VTOL
    VtolTiltwing = 24,              ///< 倾转翼VTOL / Tiltwing VTOL
    VtolReserved5 = 25,             ///< VTOL预留5 / VTOL reserved 5
    Gimbal = 26,                    ///< 云台 / Gimbal
    Adsb = 27,                      ///< ADSB系统 / ADSB system
    Parafoil = 28,                  ///< 可操控的非刚性翼膜 / Steerable, nonrigid airfoil
    Dodecarotor = 29,               ///< 十二旋翼 / Dodecarotor
    Camera = 30,                    ///< 相机 / Camera
    ChargingStation = 31,           ///< 充电站 / Charging station
    Flarm = 32,                     ///< FLARM防撞系统 / FLARM collision avoidance system
    Servo = 33,                     ///< 伺服电机 / Servo
    Odid = 34,                      ///< 开放无人机ID / Open Drone ID
    Decarotor = 35,                 ///< 十旋翼 / Decarotor
    Battery = 36,                   ///< 电池 / Battery
    Parachute = 37,                 ///< 降落伞 / Parachute
    Log = 38,                       ///< 日志 / Log
    Osd = 39,                       ///< OSD / OSD
    Imu = 40,                       ///< 惯性测量单元 / IMU
    Gps = 41,                       ///< GPS / GPS
    Winch = 42,                     ///< 绞盘 / Winch
    GenericMulticopter = 43,        ///< 不符合特定类型或类型未知的通用多旋翼 / Generic multirotor that does not fit into a specific type or whose type is unknown
    Illuminator = 44,               ///< 照明器 / Illuminator
    SpacecraftOrbiter = 45,         ///< 航天器轨道器 / Orbiter spacecraft
    GroundQuadruped = 46,           ///< 通用四足地面车辆 / A generic four-legged ground vehicle
    VtolGyrodyn = 47,               ///< 直升机与自转旋翼机混合VTOL / VTOL hybrid of helicopter and autogyro
    Gripper = 48,                   ///< 机械抓手 / Gripper
    Radio = 49                      ///< 无线电 / Radio
};



enum class [[nodiscard]] MavMessageId : uint8_t {
    Heartbeat = 0,
    SysStatus = 1,
    SystemTime = 2,
    BatteryStatus = 3,
    GpsRawInt = 24,
    HighresImu = 105,
    RawImu = 27,
    Attitude = 30,
    GlobalPositionInt = 33,
    RcChannelsRaw = 35,
    MissionRequestInt = 49,
    MissionAck = 47,
    MissionItemInt = 73,
    CommandLong = 76,
    CommandAck = 77,
    SetMode = 11,
    ManualControl = 69,
    RcChannels = 65,
    ServoOutputRaw = 36,
    RawPressure = 28,
    ScaledPressure = 29,
    AttitudeQuaternion = 31,
    VfrHud = 74,
    GpsStatus = 25,
    RawTcpmotor = 147,
    RcChannelsScaled = 34,
    NavControllerOutput = 62,
    MissionCurrent = 42,
    GlobalPositionIntCov = 68,
    LocalPositionNed = 32,
    PositionTargetLocalNed = 84,
    PositionTargetGlobalInt = 85,
    HighLatency = 92,
    HighLATENCY2 = 93,
    Vibration = 243,
    HighLATENCY3 = 244,
    AutopilotVersion = 148,
    Statustext = 253,
    DebugVect = 250,
    Debug = 254,
    Wind = 168,
    Rangefinder = 173,
    TerrainRequest = 133,
    TerrainData = 134,
    TerrainCheck = 135,
    TerrainReport = 136,
    ScaledPRESSURE2 = 137,
    AttPosMocap = 138,
    SetAttitudeTarget = 82,
    AttitudeTarget = 83,
    SetPositionTargetLocalNed = 84,
    SetPositionTargetGlobalInt = 85,
    LocalPositionNedSystemGlobalOffset = 89,
    HilState = 90,
    HilControls = 91,
    HilRcInputsRaw = 92,
    HilActuatorControls = 93,
    OpticalFlow = 100,
    GlobalVisionPositionEstimate = 112,
    VisionPositionEstimate = 101,
    VisionSpeedEstimate = 102,
    ViconPositionEstimate = 103,
    OpticalFlowRad = 106,
    HilSensor = 107,
    SimState = 108,
    RadioStatus = 109,
    FileTransferProtocol = 110,
    Timesync = 111,
    CameraTrigger = 112,
    HilGps = 113,
    HilOpticalFlow = 114,
    Hwstatus = 115,
    RcChannelsOverride = 116,
    MessageInterval = 117,
    ExtendedSysState = 118,
    AdsbVehicle = 126,
    Collision = 127,
    V2Extension = 128,
    MemoryVect = 129,
    DebugFloatArray = 130,
    RawOdometry = 131,
    LongerMessage = 132,
    RawPRESSURE2 = 137,
};

#if 0

enum class [[nodiscard]] MavlinkSystemId : uint8_t {
    All = 0,
    AUTOPILOT1 = 1,
    Camera = 100,
    Camera2 = 101,
    Camera3 = 102,
    Camera4 = 103,
    Camera5 = 104,
    Camera6 = 105,
    Servo1 = 140,
    Servo2 = 141,
    Servo3 = 142,
    Servo4 = 143,
    Servo5 = 144,
    Servo6 = 145,
    Servo7 = 146,
    Servo8 = 147,
    Servo9 = 148,
    Servo10 = 149,
    Servo11 = 150,
    Servo12 = 151,
    Servo13 = 152,
    Servo14 = 153,
    Gimbal = 154,
    Log = 155,
    Adsb = 156,
    Osd = 157,
    Peripheral = 158,
    QX1Gimbal = 159,
    Flarm = 160,
    Terrain = 161,
    Gcs = 190,
    GCS2 = 191,
    GCS3 = 192,
    GCS4 = 193,
    GCS5 = 194,
    GCS6 = 195,
    TelemetryRadio = 196,
    User1 = 197,
    User2 = 198,
    User3 = 199,
    User4 = 200,
    User5 = 201,
    User6 = 202,
    User7 = 203,
    User8 = 204,
    User9 = 205,
    User10 = 206,
    User11 = 207,
    User12 = 208,
    User13 = 209,
    User14 = 210,
    User15 = 211,
    User16 = 212,
    Hitl = 220,
    Simulink = 221,
    Sih = 222,
    Osd = 225,
    TelemetryRadio = 226,
    GcsBridge = 227,
    LogBridge = 228,
    CameraBridge = 229,
    MavlinkBridge = 230,
    TunnelNode = 231,
    Missionplanner = 240,
    OnboardComputer = 241,
    Pathplanner = 242,
    ObstacleAvoidance = 243,
    VisualInertialOdometry = 244,
    PairingManager = 245,
    Imu = 246,
    Gps = 247,
    UartBridge = 248,
    Winch = 249,
    UdpBridge = 250,
    TelemetryRadio = 251,
};
#endif



template<size_t N>
struct OwnedNtstr{
    std::array<char, N> str;
};

using fp32 = math::fp32;
}