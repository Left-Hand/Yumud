#pragma once

#include <cstdint>

namespace ymd::net::mavlink {


enum class MAV_TYPE:uint8_t{
    GENERIC = 0,
    FIXED_WING = 1,
    QUADROTOR = 2,
    COAXIAL = 3,
    HELICOPTER = 4,
    ANTENNA_TRACKER = 5,
    GCS = 6,
    AIRSHIP = 7,
    

}

enum class MavlinkMessageId : uint16_t {
    HEARTBEAT = 0,
    SYS_STATUS = 1,
    SYSTEM_TIME = 2,
    BATTERY_STATUS = 3,
    GPS_RAW_INT = 24,
    HIGHRES_IMU = 105,
    RAW_IMU = 27,
    ATTITUDE = 30,
    GLOBAL_POSITION_INT = 33,
    RC_CHANNELS_RAW = 35,
    MISSION_REQUEST_INT = 49,
    MISSION_ACK = 47,
    MISSION_ITEM_INT = 73,
    COMMAND_LONG = 76,
    COMMAND_ACK = 77,
    SET_MODE = 11,
    MANUAL_CONTROL = 69,
    RC_CHANNELS = 65,
    SERVO_OUTPUT_RAW = 36,
    RAW_PRESSURE = 28,
    SCALED_PRESSURE = 29,
    ATTITUDE_QUATERNION = 31,
    VFR_HUD = 74,
    GPS_STATUS = 25,
    RAW_TCPMOTOR = 147,
    RC_CHANNELS_SCALED = 34,
    NAV_CONTROLLER_OUTPUT = 62,
    MISSION_CURRENT = 42,
    GLOBAL_POSITION_INT_COV = 68,
    LOCAL_POSITION_NED = 32,
    POSITION_TARGET_LOCAL_NED = 84,
    POSITION_TARGET_GLOBAL_INT = 85,
    HIGH_LATENCY = 92,
    HIGH_LATENCY2 = 93,
    VIBRATION = 243,
    HIGH_LATENCY3 = 244,
    AUTOPILOT_VERSION = 148,
    STATUSTEXT = 253,
    DEBUG_VECT = 250,
    DEBUG = 254,
    NAMED_VALUE_FLOAT = 266,
    NAMED_VALUE_INT = 267,
    WIND = 168,
    RANGEFINDER = 173,
    TERRAIN_REQUEST = 133,
    TERRAIN_DATA = 134,
    TERRAIN_CHECK = 135,
    TERRAIN_REPORT = 136,
    SCALED_PRESSURE2 = 137,
    ATT_POS_MOCAP = 138,
    SET_ATTITUDE_TARGET = 82,
    ATTITUDE_TARGET = 83,
    SET_POSITION_TARGET_LOCAL_NED = 84,
    POSITION_TARGET_LOCAL_NED = 84,
    SET_POSITION_TARGET_GLOBAL_INT = 85,
    LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET = 89,
    HIL_STATE = 90,
    HIL_CONTROLS = 91,
    HIL_RC_INPUTS_RAW = 92,
    HIL_ACTUATOR_CONTROLS = 93,
    OPTICAL_FLOW = 100,
    GLOBAL_VISION_POSITION_ESTIMATE = 112,
    VISION_POSITION_ESTIMATE = 101,
    VISION_SPEED_ESTIMATE = 102,
    VICON_POSITION_ESTIMATE = 103,
    HIGHRES_IMU = 105,
    OPTICAL_FLOW_RAD = 106,
    HIL_SENSOR = 107,
    SIM_STATE = 108,
    RADIO_STATUS = 109,
    FILE_TRANSFER_PROTOCOL = 110,
    TIMESYNC = 111,
    CAMERA_TRIGGER = 112,
    HIL_GPS = 113,
    HIL_OPTICAL_FLOW = 114,
    HWSTATUS = 115,
    RC_CHANNELS_OVERRIDE = 116,
    MESSAGE_INTERVAL = 117,
    EXTENDED_SYS_STATE = 118,
    ADSB_VEHICLE = 126,
    COLLISION = 127,
    V2_EXTENSION = 128,
    MEMORY_VECT = 129,
    DEBUG_FLOAT_ARRAY = 130,
    RAW_ODOMETRY = 131,
    LONGER_MESSAGE = 132,
    RAW_PRESSURE2 = 137,
    MAV_CMD_NAV_WAYPOINT = 16,
    MAV_CMD_NAV_LOITER_UNLIM = 17,
    MAV_CMD_NAV_LOITER_TURNS = 18,
    MAV_CMD_NAV_LOITER_TIME = 19,
    MAV_CMD_NAV_RETURN_TO_LAUNCH = 20,
    MAV_CMD_NAV_LAND = 21,
    MAV_CMD_NAV_TAKEOFF = 22,
    MAV_CMD_NAV_ROI = 80,
    MAV_CMD_NAV_PATHPLANNING = 81,
    MAV_CMD_NAV_SPLINE_WAYPOINT = 82,
    MAV_CMD_NAV_GUIDED_ENABLE = 92,
    MAV_CMD_NAV_DELAY = 93,
    MAV_CMD_CONDITION_DELAY = 112,
    MAV_CMD_CONDITION_CHANGE_ALT = 113,
    MAV_CMD_CONDITION_DISTANCE = 114,
    MAV_CMD_CONDITION_YAW = 115,
    MAV_CMD_CONDITION_CHANNEL = 116,
    MAV_CMD_DO_SET_MODE = 176,
    MAV_CMD_DO_JUMP = 177,
    MAV_CMD_DO_CHANGE_SPEED = 178,
    MAV_CMD_DO_SET_HOME = 179,
    MAV_CMD_DO_SET_PARAMETER = 180,
    MAV_CMD_DO_SET_RELAY = 181,
    MAV_CMD_DO_REPEAT_RELAY = 182,
    MAV_CMD_DO_SET_SERVO = 183,
    MAV_CMD_DO_REPEAT_SERVO = 184,
    MAV_CMD_DO_FLIGHTTERMINATION = 185,
    MAV_CMD_DO_CHANGE_ALTITUDE = 186,
    MAV_CMD_DO_SET_CAM_TRIGG_DIST = 187,
    MAV_CMD_DO_FENCE_ENABLE = 188,
    MAV_CMD_DO_PARACHUTE = 189,
    MAV_CMD_DO_MOTOR_TEST = 193,
    MAV_CMD_DO_INVERTED_FLIGHT = 194,
    MAV_CMD_DO_GRIPPER = 195,
    MAV_CMD_DO_AUTOTUNE_ENABLE = 196,
    MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL = 197,
    MAV_CMD_DO_SET_GUIDED_SUBMODE_STANDARD = 215,
    MAV_CMD_DO_SET_GUIDED_SUBMODE_CIRCLE = 216,
    MAV_CMD_DO_SET_GUIDED_SUBMODE_SQUARE = 217,
    MAV_CMD_DO_SET_GUIDED_SUBMODE_VERICAL = 218,
    MAV_CMD_DO_SET_GUIDED_SUBMODE_HORIZONTAL = 219,
    MAV_CMD_DO_SET_MODE_SLOT = 220,
    MAV_CMD_DO_JUMP_TAG = 221,
    MAV_CMD_DO_SET_CAM_TRIGG_COUNT = 222,
    MAV_CMD_IMAGE_START_CAPTURE = 2000,
    MAV_CMD_IMAGE_STOP_CAPTURE = 2001,
    MAV_CMD_VIDEO_START_CAPTURE = 2500,
    MAV_CMD_VIDEO_STOP_CAPTURE = 2501,
    MAV_CMD_VIDEO_START_STREAMING = 2502,
    MAV_CMD_VIDEO_STOP_STREAMING = 2503,
    MAV_CMD_REQUEST_MESSAGE = 2800,
    MAV_CMD_LOG_REQUEST_DATA = 2801,
    MAV_CMD_LOG_REQUEST_END = 2802,
    MAV_CMD_LOG_REQUEST_LIST = 2803,
    MAV_CMD_LOG_ERASE = 2804,
    MAV_CMD_LOG_REQUEST_UPLOAD = 2805,
    MAV_CMD_LOG_CANCEL_UPLOAD = 2806,
    MAV_CMD_SET_MAG_OFFSETS = 2807,
    MAV_CMD_MAG_CAL_REPORT = 2808,
    MAV_CMD_MAG_CAL_ACK = 2809,
    MAV_CMD_DO_START_MAG_CAL = 2810,
    MAV_CMD_DO_ACCEPT_MAG_CAL = 2811,
    MAV_CMD_DO_CANCEL_MAG_CAL = 2812,
    MAV_CMD_DO_SET_CAM_TRIGG_DIST = 187,
    MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL = 197,
    MAV_CMD_DO_SET_CAM_TRIGG_COUNT = 222,
    MAV_CMD_DO_SET_CAM_TRIGG_DIST = 187,
    MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL = 197,
    MAV_CMD_DO_SET_CAM_TRIGG_COUNT = 222,
};

enum class MavlinkMessageType : uint8_t {
    HEARTBEAT = 0,
    SYS_STATUS = 1,
    SYSTEM_TIME = 2,
    BATTERY_STATUS = 3,
    GPS_RAW_INT = 24,
    HIGHRES_IMU = 105,
    RAW_IMU = 27,
    ATTITUDE = 30,
    GLOBAL_POSITION_INT = 33,
    RC_CHANNELS_RAW = 35,
    MISSION_REQUEST_INT = 49,
    MISSION_ACK = 47,
    MISSION_ITEM_INT = 73,
    COMMAND_LONG = 76,
    COMMAND_ACK = 77,
    SET_MODE = 11,
    MANUAL_CONTROL = 69,
    RC_CHANNELS = 65,
    SERVO_OUTPUT_RAW = 36,
    RAW_PRESSURE = 28,
    SCALED_PRESSURE = 29,
    ATTITUDE_QUATERNION = 31,
    VFR_HUD = 74,
    GPS_STATUS = 25,
    RAW_TCPMOTOR = 147,
    RC_CHANNELS_SCALED = 34,
    NAV_CONTROLLER_OUTPUT = 62,
    MISSION_CURRENT = 42,
    GLOBAL_POSITION_INT_COV = 68,
    LOCAL_POSITION_NED = 32,
    POSITION_TARGET_LOCAL_NED = 84,
    POSITION_TARGET_GLOBAL_INT = 85,
    HIGH_LATENCY = 92,
    HIGH_LATENCY2 = 93,
    VIBRATION = 243,
    HIGH_LATENCY3 = 244,
    AUTOPILOT_VERSION = 148,
    STATUSTEXT = 253,
    DEBUG_VECT = 250,
    DEBUG = 254,
    NAMED_VALUE_FLOAT = 266,
    NAMED_VALUE_INT = 267,
    WIND = 168,
    RANGEFINDER = 173,
    TERRAIN_REQUEST = 133,
    TERRAIN_DATA = 134,
    TERRAIN_CHECK = 135,
    TERRAIN_REPORT = 136,
    SCALED_PRESSURE2 = 137,
    ATT_POS_MOCAP = 138,
    SET_ATTITUDE_TARGET = 82,
    ATTITUDE_TARGET = 83,
    SET_POSITION_TARGET_LOCAL_NED = 84,
    POSITION_TARGET_LOCAL_NED = 84,
    SET_POSITION_TARGET_GLOBAL_INT = 85,
    LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET = 89,
    HIL_STATE = 90,
    HIL_CONTROLS = 91,
    HIL_RC_INPUTS_RAW = 92,
    HIL_ACTUATOR_CONTROLS = 93,
    OPTICAL_FLOW = 100,
    GLOBAL_VISION_POSITION_ESTIMATE = 112,
    VISION_POSITION_ESTIMATE = 101,
    VISION_SPEED_ESTIMATE = 102,
    VICON_POSITION_ESTIMATE = 103,
    HIGHRES_IMU = 105,
    OPTICAL_FLOW_RAD = 106,
    HIL_SENSOR = 107,
    SIM_STATE = 108,
    RADIO_STATUS = 109,
    FILE_TRANSFER_PROTOCOL = 110,
    TIMESYNC = 111,
    CAMERA_TRIGGER = 112,
    HIL_GPS = 113,
    HIL_OPTICAL_FLOW = 114,
    HWSTATUS = 115,
    RC_CHANNELS_OVERRIDE = 116,
    MESSAGE_INTERVAL = 117,
    EXTENDED_SYS_STATE = 118,
    ADSB_VEHICLE = 126,
    COLLISION = 127,
    V2_EXTENSION = 128,
    MEMORY_VECT = 129,
    DEBUG_FLOAT_ARRAY = 130,
    RAW_ODOMETRY = 131,
    LONGER_MESSAGE = 132,
    RAW_PRESSURE2 = 137,
};

enum class MavlinkSystemId : uint8_t {
    MAV_COMP_ID_ALL = 0,
    MAV_COMP_ID_AUTOPILOT1 = 1,
    MAV_COMP_ID_CAMERA = 100,
    MAV_COMP_ID_CAMERA2 = 101,
    MAV_COMP_ID_CAMERA3 = 102,
    MAV_COMP_ID_CAMERA4 = 103,
    MAV_COMP_ID_CAMERA5 = 104,
    MAV_COMP_ID_CAMERA6 = 105,
    MAV_COMP_ID_SERVO1 = 140,
    MAV_COMP_ID_SERVO2 = 141,
    MAV_COMP_ID_SERVO3 = 142,
    MAV_COMP_ID_SERVO4 = 143,
    MAV_COMP_ID_SERVO5 = 144,
    MAV_COMP_ID_SERVO6 = 145,
    MAV_COMP_ID_SERVO7 = 146,
    MAV_COMP_ID_SERVO8 = 147,
    MAV_COMP_ID_SERVO9 = 148,
    MAV_COMP_ID_SERVO10 = 149,
    MAV_COMP_ID_SERVO11 = 150,
    MAV_COMP_ID_SERVO12 = 151,
    MAV_COMP_ID_SERVO13 = 152,
    MAV_COMP_ID_SERVO14 = 153,
    MAV_COMP_ID_GIMBAL = 154,
    MAV_COMP_ID_LOG = 155,
    MAV_COMP_ID_ADSB = 156,
    MAV_COMP_ID_OSD = 157,
    MAV_COMP_ID_PERIPHERAL = 158,
    MAV_COMP_ID_QX1_GIMBAL = 159,
    MAV_COMP_ID_FLARM = 160,
    MAV_COMP_ID_TERRAIN = 161,
    MAV_COMP_ID_GCS = 190,
    MAV_COMP_ID_GCS2 = 191,
    MAV_COMP_ID_GCS3 = 192,
    MAV_COMP_ID_GCS4 = 193,
    MAV_COMP_ID_GCS5 = 194,
    MAV_COMP_ID_GCS6 = 195,
    MAV_COMP_ID_TELEMETRY_RADIO = 196,
    MAV_COMP_ID_USER1 = 197,
    MAV_COMP_ID_USER2 = 198,
    MAV_COMP_ID_USER3 = 199,
    MAV_COMP_ID_USER4 = 200,
    MAV_COMP_ID_USER5 = 201,
    MAV_COMP_ID_USER6 = 202,
    MAV_COMP_ID_USER7 = 203,
    MAV_COMP_ID_USER8 = 204,
    MAV_COMP_ID_USER9 = 205,
    MAV_COMP_ID_USER10 = 206,
    MAV_COMP_ID_USER11 = 207,
    MAV_COMP_ID_USER12 = 208,
    MAV_COMP_ID_USER13 = 209,
    MAV_COMP_ID_USER14 = 210,
    MAV_COMP_ID_USER15 = 211,
    MAV_COMP_ID_USER16 = 212,
    MAV_COMP_ID_HITL = 220,
    MAV_COMP_ID_SIMULINK = 221,
    MAV_COMP_ID_SIH = 222,
    MAV_COMP_ID_OSD = 225,
    MAV_COMP_ID_TELEMETRY_RADIO = 226,
    MAV_COMP_ID_GCS_BRIDGE = 227,
    MAV_COMP_ID_LOG_BRIDGE = 228,
    MAV_COMP_ID_CAMERA_BRIDGE = 229,
    MAV_COMP_ID_MAVLINK_BRIDGE = 230,
    MAV_COMP_ID_TUNNEL_NODE = 231,
    MAV_COMP_ID_MISSIONPLANNER = 240,
    MAV_COMP_ID_ONBOARD_COMPUTER = 241,
    MAV_COMP_ID_PATHPLANNER = 242,
    MAV_COMP_ID_OBSTACLE_AVOIDANCE = 243,
    MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY = 244,
    MAV_COMP_ID_PAIRING_MANAGER = 245,
    MAV_COMP_ID_IMU = 246,
    MAV_COMP_ID_GPS = 247,
    MAV_COMP_ID_UART_BRIDGE = 248,
    MAV_COMP_ID_WINCH = 249,
    MAV_COMP_ID_UDP_BRIDGE = 250,
    MAV_COMP_ID_TELEMETRY_RADIO = 251,
    MAV_COMP_ID_USER1 = 252,
    MAV_COMP_ID_USER2 = 253,
    MAV_COMP_ID_USER3 = 254,
    MAV_COMP_ID_USER4 = 255,
};

enum class MavlinkComponentId : uint8_t {
    MAV_COMP_ID_ALL = 0,
    MAV_COMP_ID_AUTOPILOT1 = 1,
    MAV_COMP_ID_CAMERA = 100,
    MAV_COMP_ID_CAMERA2 = 101,
    MAV_COMP_ID_CAMERA3 = 102,
    MAV_COMP_ID_CAMERA4 = 103,
    MAV_COMP_ID_CAMERA5 = 104,
    MAV_COMP_ID_CAMERA6 = 105,
    MAV_COMP_ID_SERVO1 = 14
}

}