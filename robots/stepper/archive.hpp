#ifndef __STEPPER_ARCHIVE_HPP__

#define __STEPPER_ARCHIVE_HPP__

#include "constants.hpp"


//at24c02 can max contain 256 bytes

#define YEAR (((__DATE__[9]-'0')) * 10 + (__DATE__[10]-'0'))
#define MONTH (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n' ? 1 : \
            __DATE__[0] == 'F' ? 2 : \
            __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3 : \
            __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4 : \
            __DATE__[0] == 'M' ?  5 : \
            __DATE__[0] == 'J' && __DATE__[1] == 'u' ? 6 : \
            __DATE__[0] == 'J' ? 7 : \
            __DATE__[0] == 'A' ? 8 : \
            __DATE__[0] == 'S' ? 9 : \
            __DATE__[0] == 'O' ? 10 : \
            11)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4]-'0') * 10 + (__DATE__[5]-'0'))
#define HOUR ((__TIME__[0]-'0') * 10 + __TIME__[1]-'0')
#define MINUTE ((__TIME__[3]-'0') * 10 + __TIME__[4]-'0')

namespace StepperUtils{
    static constexpr uint8_t build_version = 10;
    static constexpr uint8_t drv_type = 42;
    static constexpr char drv_branch = 'm';
    static constexpr uint8_t year = YEAR;
    static constexpr uint8_t month = MONTH;
    static constexpr uint8_t day = DAY;
    static constexpr uint8_t hour = HOUR;
    static constexpr uint8_t minute = MINUTE;

struct Archive{
    struct{
        uint8_t bver = build_version;
        uint8_t dtype = drv_type;
        char dbranch = drv_branch;
        uint8_t y = year;
        uint8_t m = month;
        uint8_t d = day;
        uint8_t h = hour;
        uint8_t mi = minute;
    };

    uint16_t offs[50];
};

}
#endif