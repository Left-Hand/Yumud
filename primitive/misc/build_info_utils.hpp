#pragma once

#include <cstdint>


namespace ymd{

namespace utils{

static constexpr uint16_t exact_year_from_datestr(const char * str){
    uint16_t sum = 0;

    sum += (str[7]-'0') * 1000;
    sum += (str[8]-'0') * 100;
    sum += (str[9]-'0') * 10;
    sum += (str[10]-'0');

    return sum;
}

static constexpr uint16_t exact_month_from_datestr(const char * str){
    return (
            str[0] == 'J' && str[1] == 'a' && str[2] == 'n' ? 1 :
            str[0] == 'F' ? 2 :
            str[0] == 'M' && str[2] == 'r' ? 3 :
            str[0] == 'A' && str[1] == 'p' ? 4 :
            str[0] == 'M' ?  5 :
            str[0] == 'J' && str[1] == 'u' && str[2] == 'n' ? 6 :
            str[0] == 'J' ? 7 :
            str[0] == 'A' ? 8 :
            str[0] == 'S' ? 9 :
            str[0] == 'O' ? 10 :
            str[0] == 'N' ? 11 : 12
    );
}

static constexpr uint8_t exact_day_from_datestr(const char * str){
    return ((str[4] == ' ' ? 0 : str[4]-'0') * 10 + (str[5]-'0'));
}

static constexpr uint8_t exact_hour_from_timestr(const char * str){
    return ((str[0]-'0') * 10 + (str[1]-'0'));
}

static constexpr uint8_t exact_minute_from_timestr(const char * str){
    return ((str[3]-'0') * 10 + (str[4]-'0'));
}

static constexpr uint8_t exact_second_from_timestr(const char * str){
    return ((str[6]-'0') * 10 + (str[7]-'0'));
}
}


static constexpr uint16_t BUILT_YEAR = utils::exact_year_from_datestr(__DATE__);
static constexpr uint8_t BUILT_MONTH = utils::exact_month_from_datestr(__DATE__);

static constexpr uint8_t BUILT_DAY = utils::exact_day_from_datestr(__DATE__);

static constexpr uint8_t BUILT_HOUR = utils::exact_hour_from_timestr(__TIME__);

static constexpr uint8_t BUILT_MINUTE = utils::exact_minute_from_timestr(__TIME__);

static constexpr uint8_t BUILT_SECOND = utils::exact_second_from_timestr(__TIME__);
}