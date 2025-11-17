#pragma once

#include <cstdint>

namespace ymd::mavlink::msgs{
    
struct Heartbeat {
    uint8_t type;
    uint8_t autopilot;
    uint8_t base_mode;
    uint32_t custom_mode;
    uint8_t system_status;
    uint8_t mavlink_version;
};

struct SysTime{
    uint64_t time_unix_usec;
    uint32_t time_boot_ms;
    uint16_t time_offset;
    uint8_t gps_usec;
    uint8_t gps_week;
    uint8_t gps_week_day;
    uint8_t gps_leap_seconds;
    uint8_t gps_valid;
    uint8_t gps_fix_type;
};


struct SetPositionTargetLocalNed {
    uint32_t time_boot_ms;
    uint32_t target_system;
    uint32_t target_component;
    uint32_t coordinate_frame;
    uint16_t type_mask;
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float vz;
    float afx;
    float afy;
    float afz;
    float yaw;
    float yaw_rate;
};

struct SetAttitudeTarget {
    uint32_t time_boot_ms;
    uint8_t target_system;
    uint8_t target_component;
    uint8_t type_mask;
    float q[4]; // Quaternion
    float body_roll_rate;
    float body_pitch_rate;
    float body_yaw_rate;
    float thrust;
};

struct RcChannelsOverride {
    uint8_t target_system;
    uint8_t target_component;
    uint16_t chan1_raw;
    uint16_t chan2_raw;
    uint16_t chan3_raw;
    uint16_t chan4_raw;
    uint16_t chan5_raw;
    uint16_t chan6_raw;
    uint16_t chan7_raw;
    uint16_t chan8_raw;
};

struct LocalPositionNed {
    uint32_t time_boot_ms;
    float x;
    float y;
    float z;
    float vx;
    float vy;
    float vz;
};

struct GlobalPositionInt {
    uint32_t time_boot_ms;
    int32_t lat; // Latitude in degrees * 1E7
    int32_t lon; // Longitude in degrees * 1E7
    int32_t alt; // Altitude in mm
    int32_t relative_alt; // Altitude above ground in mm
    int16_t vx; // Ground X Speed (Latitude * 100 in cm/s)
    int16_t vy; // Ground Y Speed (Longitude * 100 in cm/s)
    int16_t vz; // Ground Z Speed (Altitude * 100 in cm/s)
    uint16_t hdg; // Compass heading in degrees * 100
};

struct Attitude {
    uint32_t time_boot_ms;
    float roll;
    float pitch;
    float yaw;
    float rollspeed;
    float pitchspeed;
    float yawspeed;
};

struct RawImu {
    uint64_t time_usec;
    int16_t xacc;
    int16_t yacc;
    int16_t zacc;
    int16_t xgyro;
    int16_t ygyro;
    int16_t zgyro;
    int16_t xmag;
    int16_t ymag;
    int16_t zmag;
};

struct GpsRawInt {
    uint64_t time_usec;
    uint8_t fix_type;
    int32_t lat;
    int32_t lon;
    int32_t alt;
    uint16_t eph;
    uint16_t epv;
    uint16_t vel;
    uint16_t cog;
    uint8_t satellites_visible;
};

struct BatteryStatus {
    uint32_t time_boot_ms;
    uint16_t voltage;
    int16_t current;
    int16_t current_consumed;
    int16_t battery_remaining;
};

struct CommandLong {
    uint8_t target_system;
    uint8_t target_component;
    uint16_t command;
    uint8_t confirmation;
    float param1;
    float param2;
    float param3;
    float param4;
    float param5;
    float param6;
    float param7;
};

struct CommandAck {
    uint16_t command;
    uint8_t result;
};

}