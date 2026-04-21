#pragma once

#include <cstdint>
#include "core/math/float/fp32.hpp"

namespace ymd::mavlink::msgs{
    
struct [[nodiscard]] Heartbeat final {
    uint8_t type;
    uint8_t autopilot;
    uint8_t base_mode;
    uint32_t custom_mode;
    uint8_t system_status;
    uint8_t mavlink_version;
};

struct [[nodiscard]] SysTime final {
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


struct [[nodiscard]] SetPositionTargetLocalNed final {
    uint32_t time_boot_ms;
    uint32_t target_system;
    uint32_t target_component;
    uint32_t coordinate_frame;
    uint16_t type_mask;
    math::fp32 x;
    math::fp32 y;
    math::fp32 z;
    math::fp32 vx;
    math::fp32 vy;
    math::fp32 vz;
    math::fp32 afx;
    math::fp32 afy;
    math::fp32 afz;
    math::fp32 yaw;
    math::fp32 yaw_rate;
};

struct [[nodiscard]] SetAttitudeTarget final {
    uint32_t time_boot_ms;
    uint8_t target_system;
    uint8_t target_component;
    uint8_t type_mask;
    math::fp32 q[4]; // Quaternion
    math::fp32 body_roll_rate;
    math::fp32 body_pitch_rate;
    math::fp32 body_yaw_rate;
    math::fp32 thrust;
};

struct [[nodiscard]] RcChannelsOverride final {
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

struct [[nodiscard]] LocalPositionNed final {
    uint32_t time_boot_ms;
    math::fp32 x;
    math::fp32 y;
    math::fp32 z;
    math::fp32 vx;
    math::fp32 vy;
    math::fp32 vz;
};

struct [[nodiscard]] GlobalPositionInt final {
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

struct [[nodiscard]] Attitude final {
    uint32_t time_boot_ms;
    math::fp32 roll;
    math::fp32 pitch;
    math::fp32 yaw;
    math::fp32 rollspeed;
    math::fp32 pitchspeed;
    math::fp32 yawspeed;
};

struct [[nodiscard]] RawImu final {
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

struct [[nodiscard]] GpsRawInt final {
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

struct [[nodiscard]] BatteryStatus final {
    uint32_t time_boot_ms;
    uint16_t voltage;
    int16_t current;
    int16_t current_consumed;
    int16_t battery_remaining;
};

struct [[nodiscard]] CommandLong final {
    uint8_t target_system;
    uint8_t target_component;
    uint16_t command;
    uint8_t confirmation;
    math::fp32 param1;
    math::fp32 param2;
    math::fp32 param3;
    math::fp32 param4;
    math::fp32 param5;
    math::fp32 param6;
    math::fp32 param7;
};

struct [[nodiscard]] CommandAck final {
    uint16_t command;
    uint8_t result;
};

}