#pragma once

#include <cstdint>

namespace ymd::mavlink{
struct [[nodiscard]] MavModeFlagBitfields final{
    uint8_t custom_mode_enabled:1;     ///< 0b00000001 特定系统的自定义模式已启用。当使用此标志启用自定义模式时，应忽略所有其他标志 / system-specific custom mode is enabled. When using this flag to enable a custom mode all other flags should be ignored.
    uint8_t test_enabled:1;            ///< 0b00000010 系统已启用测试模式。此标志用于临时系统测试，不应在稳定版本中使用 / system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations.
    uint8_t auto_enabled:1;            ///< 0b00000100 自主模式已启用，系统自行寻找目标位置。引导标志可设置或不设置，取决于实际实现 / autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation.
    uint8_t guided_enabled:1;          ///< 0b00001000 引导模式已启用，系统飞行航路点/任务项目 / guided mode enabled, system flies waypoints / mission items.
    uint8_t stabilize_enabled:1;       ///< 0b00010000 系统电子稳定其姿态（和可选位置）。然而它需要进一步的控制输入才能移动 / system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around.
    uint8_t hil_enabled:1;             ///< 0b00100000 硬件在环仿真。所有电机/执行器被阻塞，但内部软件完全运行 / hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational.
    uint8_t manual_input_enabled:1;    ///< 0b01000000 启用遥控输入 / remote control input is enabled.
    uint8_t safety_armed:1;            ///< 0b10000000 MAV安全设置为已解锁。电机已启用/运行/可以启动。准备飞行。附加说明：发送MAV_CMD_DO_SET_MODE命令时应忽略此标志，而应使用MAV_CMD_COMPONENT_ARM_DISARM。此标志仍可用于报告解锁状态 / MAV safety set to armed. Motors are enabled / running / can start. Ready to fly. Additional note: this flag is to be ignore when sent in the command MAV_CMD_DO_SET_MODE and MAV_CMD_COMPONENT_ARM_DISARM shall be used instead. The flag can still be used to report the armed state.
};

// 微型飞行器/自动驾驶仪类别。这标识了单个型号。
// Micro air vehicle / autopilot classes. This identifies the individual model.
enum class [[nodiscard]] MavAutopilot : uint8_t {
    Generic = 0,                           ///< 通用自动驾驶仪，支持全部功能 / Generic autopilot, full support for everything
    Reserved = 1,                          ///< 预留供将来使用 / Reserved for future use
    Slugs = 2,                             ///< SLUGS自动驾驶仪，http://slugsuav.soe.ucsc.edu / SLUGS autopilot, http://slugsuav.soe.ucsc.edu
    Ardupilotmega = 3,                     ///< ArduPilot - Plane/Copter/Rover/Sub/Tracker，https://ardupilot.org / ArduPilot - Plane/Copter/Rover/Sub/Tracker, https://ardupilot.org
    Openpilot = 4,                         ///< OpenPilot，http://openpilot.org / OpenPilot, http://openpilot.org
    GenericWaypointsOnly = 5,              ///< 仅支持简单航路点的通用自动驾驶仪 / Generic autopilot only supporting simple waypoints
    GenericWaypointsAndSimpleNavigationOnly = 6, ///< 支持航路点和其他简单导航命令的通用自动驾驶仪 / Generic autopilot supporting waypoints and other simple navigation commands
    GenericMissionFull = 7,                ///< 支持完整任务命令集的通用自动驾驶仪 / Generic autopilot supporting the full mission command set
    Invalid = 8,                           ///< 无有效自动驾驶仪，例如地面站或其他MAVLink组件 / No valid autopilot, e.g. a GCS or other MAVLink component
    Ppz = 9,                               ///< PPZ无人机 - http://nongnu.org/paparazzi / PPZ UAV - http://nongnu.org/paparazzi
    Udb = 10,                              ///< 无人机开发板 / UAV Dev Board
    Fp = 11,                               ///< FlexiPilot
    Px4 = 12,                              ///< PX4自动驾驶仪 - http://px4.io/ / PX4 Autopilot - http://px4.io/
    Smaccmpilot = 13,                      ///< SMACCMPilot - http://smaccmpilot.org
    Autoquad = 14,                         ///< AutoQuad -- http://autoquad.org
    Armazila = 15,                         ///< Armazila -- http://armazila.com
    Aerob = 16,                            ///< Aerob -- http://aerob.ru
    Asluav = 17,                           ///< ASLUAV自动驾驶仪 -- http://www.asl.ethz.ch / ASLUAV autopilot -- http://www.asl.ethz.ch
    Smartap = 18,                          ///< SmartAP自动驾驶仪 - http://sky-drones.com / SmartAP Autopilot - http://sky-drones.com
    Airrails = 19,                         ///< AirRails - http://uaventure.com
    Reflex = 20                            ///< Fusion Reflex - https://fusion.engineering
};

static constexpr MavAutopilot MAV_AUTOPILOT_GENERIC = MavAutopilot::Generic;
static constexpr MavAutopilot MAV_AUTOPILOT_RESERVED = MavAutopilot::Reserved;
static constexpr MavAutopilot MAV_AUTOPILOT_SLUGS = MavAutopilot::Slugs;
static constexpr MavAutopilot MAV_AUTOPILOT_ARDUPILOTMEGA = MavAutopilot::Ardupilotmega;
static constexpr MavAutopilot MAV_AUTOPILOT_OPENPILOT = MavAutopilot::Openpilot;
static constexpr MavAutopilot MAV_AUTOPILOT_GENERIC_WAYPOINTS_ONLY = MavAutopilot::GenericWaypointsOnly;
static constexpr MavAutopilot MAV_AUTOPILOT_GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY = MavAutopilot::GenericWaypointsAndSimpleNavigationOnly;
static constexpr MavAutopilot MAV_AUTOPILOT_GENERIC_MISSION_FULL = MavAutopilot::GenericMissionFull;
static constexpr MavAutopilot MAV_AUTOPILOT_INVALID = MavAutopilot::Invalid;
static constexpr MavAutopilot MAV_AUTOPILOT_PPZ = MavAutopilot::Ppz;
static constexpr MavAutopilot MAV_AUTOPILOT_UDB = MavAutopilot::Udb;
static constexpr MavAutopilot MAV_AUTOPILOT_FP = MavAutopilot::Fp;
static constexpr MavAutopilot MAV_AUTOPILOT_PX4 = MavAutopilot::Px4;
static constexpr MavAutopilot MAV_AUTOPILOT_SMACCMPILOT = MavAutopilot::Smaccmpilot;
static constexpr MavAutopilot MAV_AUTOPILOT_AUTOQUAD = MavAutopilot::Autoquad;
static constexpr MavAutopilot MAV_AUTOPILOT_ARMAZILA = MavAutopilot::Armazila;
static constexpr MavAutopilot MAV_AUTOPILOT_AEROB = MavAutopilot::Aerob;
static constexpr MavAutopilot MAV_AUTOPILOT_ASLUAV = MavAutopilot::Asluav;
static constexpr MavAutopilot MAV_AUTOPILOT_SMARTAP = MavAutopilot::Smartap;
static constexpr MavAutopilot MAV_AUTOPILOT_AIRRALS = MavAutopilot::Airrails;
static constexpr MavAutopilot MAV_AUTOPILOT_REFLEX = MavAutopilot::Reflex;

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


static constexpr MavComponentType MAV_TYPE_GENERIC = MavComponentType::Generic;
static constexpr MavComponentType MAV_TYPE_FIXED_WING = MavComponentType::FixedWing;
static constexpr MavComponentType MAV_TYPE_QUADROTOR = MavComponentType::Quadrotor;
static constexpr MavComponentType MAV_TYPE_COAXIAL = MavComponentType::Coaxial;
static constexpr MavComponentType MAV_TYPE_HELICOPTER = MavComponentType::Helicopter;
static constexpr MavComponentType MAV_TYPE_ANTENNA_TRACKER = MavComponentType::AntennaTracker;
static constexpr MavComponentType MAV_TYPE_GCS = MavComponentType::Gcs;
static constexpr MavComponentType MAV_TYPE_AIRSHIP = MavComponentType::Airship;
static constexpr MavComponentType MAV_TYPE_FREE_BALLOON = MavComponentType::FreeBalloon;
static constexpr MavComponentType MAV_TYPE_ROCKET = MavComponentType::Rocket;
static constexpr MavComponentType MAV_TYPE_GROUND_ROVER = MavComponentType::GroundRover;
static constexpr MavComponentType MAV_TYPE_SURFACE_BOAT = MavComponentType::SurfaceBoat;
static constexpr MavComponentType MAV_TYPE_SUBMARINE = MavComponentType::Submarine;
static constexpr MavComponentType MAV_TYPE_HEXAROTOR = MavComponentType::Hexarotor;
static constexpr MavComponentType MAV_TYPE_OCTOROTOR = MavComponentType::Octocopter;  // Note: renamed from Octocopter
static constexpr MavComponentType MAV_TYPE_TRICOPTER = MavComponentType::Tricopter;
static constexpr MavComponentType MAV_TYPE_FLAPPING_WING = MavComponentType::FlappingWing;
static constexpr MavComponentType MAV_TYPE_KITE = MavComponentType::Kite;
static constexpr MavComponentType MAV_TYPE_ONBOARD_CONTROLLER = MavComponentType::OnboardController;
static constexpr MavComponentType MAV_TYPE_VTOL_TAILSITTER_DUOROTOR = MavComponentType::VtolTailsitterDuorotor;
static constexpr MavComponentType MAV_TYPE_VTOL_TAILSITTER_QUADROTOR = MavComponentType::VtolTailsitterQuadrotor;
static constexpr MavComponentType MAV_TYPE_VTOL_TILTROTOR = MavComponentType::VtolTiltrotor;
static constexpr MavComponentType MAV_TYPE_VTOL_FIXEDROTOR = MavComponentType::VtolFixedrotor;
static constexpr MavComponentType MAV_TYPE_VTOL_TAILSITTER = MavComponentType::VtolTailsitter;
static constexpr MavComponentType MAV_TYPE_VTOL_TILTWING = MavComponentType::VtolTiltwing;
static constexpr MavComponentType MAV_TYPE_VTOL_RESERVED5 = MavComponentType::VtolReserved5;
static constexpr MavComponentType MAV_TYPE_GIMBAL = MavComponentType::Gimbal;
static constexpr MavComponentType MAV_TYPE_ADSB = MavComponentType::Adsb;
static constexpr MavComponentType MAV_TYPE_PARAFOIL = MavComponentType::Parafoil;
static constexpr MavComponentType MAV_TYPE_DODECAROTOR = MavComponentType::Dodecarotor;
static constexpr MavComponentType MAV_TYPE_CAMERA = MavComponentType::Camera;
static constexpr MavComponentType MAV_TYPE_CHARGING_STATION = MavComponentType::ChargingStation;
static constexpr MavComponentType MAV_TYPE_FLARM = MavComponentType::Flarm;
static constexpr MavComponentType MAV_TYPE_SERVO = MavComponentType::Servo;
static constexpr MavComponentType MAV_TYPE_ODID = MavComponentType::Odid;
static constexpr MavComponentType MAV_TYPE_DECAROTOR = MavComponentType::Decarotor;
static constexpr MavComponentType MAV_TYPE_BATTERY = MavComponentType::Battery;
static constexpr MavComponentType MAV_TYPE_PARACHUTE = MavComponentType::Parachute;
static constexpr MavComponentType MAV_TYPE_LOG = MavComponentType::Log;
static constexpr MavComponentType MAV_TYPE_OSD = MavComponentType::Osd;
static constexpr MavComponentType MAV_TYPE_IMU = MavComponentType::Imu;
static constexpr MavComponentType MAV_TYPE_GPS = MavComponentType::Gps;
static constexpr MavComponentType MAV_TYPE_WINCH = MavComponentType::Winch;
static constexpr MavComponentType MAV_TYPE_GENERIC_MULTIROTOR = MavComponentType::GenericMulticopter;
static constexpr MavComponentType MAV_TYPE_ILLUMINATOR = MavComponentType::Illuminator;
static constexpr MavComponentType MAV_TYPE_SPACECRAFT_ORBITER = MavComponentType::SpacecraftOrbiter;
static constexpr MavComponentType MAV_TYPE_GROUND_QUADRUPED = MavComponentType::GroundQuadruped;
static constexpr MavComponentType MAV_TYPE_VTOL_GYRODYNE = MavComponentType::VtolGyrodyn;
static constexpr MavComponentType MAV_TYPE_GRIPPER = MavComponentType::Gripper;
static constexpr MavComponentType MAV_TYPE_RADIO = MavComponentType::Radio;
}