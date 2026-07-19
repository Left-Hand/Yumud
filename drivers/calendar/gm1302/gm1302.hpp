#pragma once


#include <cstdint>
#include "core/utils/Result.hpp"

namespace ymd::drivers::gm1302{

//GM1302地址定义
enum class [[nodiscard]] RegAddr:uint8_t{
    SEC           = 0x80,		//秒数据地址
    MIN           = 0x82,		//分数据地址
    HOUR          = 0x84,		//时数据地址
    DAY           = 0x86,		//日数据地址
    MONTH         = 0x88,		//月数据地址
    WEEK          = 0x8a,		//星期数据地址
    YEAR          = 0x8c,		//年数据地址
    CONTROL       = 0x8e,		//控制数据地址
    CHARGER       = 0x90, 		//充电功能地址			 
    CLKBURST      = 0xbe,
};



struct [[nodiscard]] Hms final{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    template<typename Receiver>
    Result<void, typename Receiver::Error> write(
        Receiver & rcv
    ){
        #define GM1302_WRITE_BYTE(addr, data)\
        if(const auto res = rcv.write_byte(addr, data);\
            res.is_err()) return Err(res.unwrap_err());\

        GM1302_WRITE_BYTE(RegAddr::CONTROL, 0x00)
        GM1302_WRITE_BYTE(RegAddr::SEC, 0x80)
        GM1302_WRITE_BYTE(RegAddr::HOUR, hour)
        GM1302_WRITE_BYTE(RegAddr::MIN, minute)
        GM1302_WRITE_BYTE(RegAddr::SEC, second)
        GM1302_WRITE_BYTE(RegAddr::CHARGER, 0xA5)
        GM1302_WRITE_BYTE(RegAddr::CONTROL, 0x80)

        #undef GM1302_WRITE_BYTE
        return Ok();
    }
};

struct [[nodiscard]] YearMonthDayWeek final{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;

    template<typename Receiver>
    Result<void, typename Receiver::Error> write(
        Receiver & rcv
    ){
        #define GM1302_WRITE_BYTE(addr, data)\
        if(const auto res = rcv.write_byte(addr, data);\
            res.is_err()) return Err(res.unwrap_err());\

        GM1302_WRITE_BYTE(RegAddr::CONTROL, 0x00)
        GM1302_WRITE_BYTE(RegAddr::YEAR, 0x80)
        GM1302_WRITE_BYTE(RegAddr::HOUR,    to_bcd(hour))
        GM1302_WRITE_BYTE(RegAddr::MIN,     to_bcd(minute))
        GM1302_WRITE_BYTE(RegAddr::SEC,     to_bcd(second))
        GM1302_WRITE_BYTE(RegAddr::CHARGER, 0xA5)
        GM1302_WRITE_BYTE(RegAddr::CONTROL, 0x80)

        #undef GM1302_WRITE_BYTE
        return Ok();
    }

    static constexpr uint8_t to_bcd(uint8_t bits){
        return ((bits / 10) << 4) | (bits % 10);
    }
};


}