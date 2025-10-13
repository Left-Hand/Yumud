#pragma once

#include <cstdint>

namespace ymd::mavlink::payload{
    
struct Heartbeat {
    uint8_t type;
    uint8_t autopilot;
    uint8_t base_mode;
    uint32_t custom_mode;
    uint8_t system_status;
    uint8_t mavlink_version;
};
// using t0 = reflect::member_type<0, Heartbeat>;
// static constexpr auto n = reflect::member_name<1>(Heartbeat());

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

}