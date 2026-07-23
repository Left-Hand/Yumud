#include "../build_info_utils.hpp"

using namespace ymd;

namespace{


[[maybe_unused]] static void test_date(){
    #define TEST_CASE(date_str, year, month, day)\
    {\
        static constexpr auto _year = utils::exact_year_from_datestr(date_str);\
        static constexpr auto _month = utils::exact_month_from_datestr(date_str);\
        static constexpr auto _day = utils::exact_day_from_datestr(date_str);\
        static_assert(_year == year);\
        static_assert(_month == month);\
        static_assert(_day == day);\
    }

    // Basic cases
    TEST_CASE("Jul 23 2026", 2026, 7, 23)
    TEST_CASE("Jan 01 2024", 2024, 1, 1)
    TEST_CASE("Dec 31 2023", 2023, 12, 31)
    
    // Month variations
    TEST_CASE("Feb 29 2020", 2020, 2, 29)  // Leap year
    TEST_CASE("Feb 28 2021", 2021, 2, 28)  // Non-leap year
    TEST_CASE("Mar 15 2025", 2025, 3, 15)
    TEST_CASE("Apr 01 2022", 2022, 4, 1)
    TEST_CASE("May 30 2019", 2019, 5, 30)
    TEST_CASE("Jun 10 2018", 2018, 6, 10)
    TEST_CASE("Aug 20 2017", 2017, 8, 20)
    TEST_CASE("Sep 05 2016", 2016, 9, 5)
    TEST_CASE("Oct 31 2015", 2015, 10, 31)
    TEST_CASE("Nov 11 2014", 2014, 11, 11)
    
    // Different year ranges
    TEST_CASE("Jan 01 2000", 2000, 1, 1)  // Millennium
    TEST_CASE("Dec 31 1999", 1999, 12, 31)  // Pre-millennium
    TEST_CASE("Jul 04 1776", 1776, 7, 4)  // Historical date
    TEST_CASE("Jan 01 1970", 1970, 1, 1)  // Unix epoch
    TEST_CASE("Jan 01 2038", 2038, 1, 1)  // Year 2038 problem
    
    // Single digit dates
    TEST_CASE("Jan 01 2023", 2023, 1, 1)
    TEST_CASE("Jan 09 2023", 2023, 1, 9)
    TEST_CASE("Mar 03 2023", 2023, 3, 3)
    
    // Edge day values
    TEST_CASE("Jan 31 2023", 2023, 1, 31)
    TEST_CASE("Mar 31 2023", 2023, 3, 31)
    TEST_CASE("Apr 30 2023", 2023, 4, 30)
    TEST_CASE("Jun 30 2023", 2023, 6, 30)
    TEST_CASE("Sep 30 2023", 2023, 9, 30)
    TEST_CASE("Nov 30 2023", 2023, 11, 30)
    
    // Additional leap year cases
    TEST_CASE("Feb 29 2000", 2000, 2, 29)  // Century leap year
    TEST_CASE("Feb 29 2400", 2400, 2, 29)  // Future century leap year
    TEST_CASE("Feb 28 1900", 1900, 2, 28)  // Century non-leap year
    TEST_CASE("Feb 28 2100", 2100, 2, 28)  // Future century non-leap year

    #undef TEST_CASE
}

[[maybe_unused]] static void test_time(){
    #define TEST_CASE(time_str, hour, minute, second)\
    {\
        static constexpr auto _hour = utils::exact_hour_from_timestr(time_str);\
        static constexpr auto _minute = utils::exact_minute_from_timestr(time_str);\
        static constexpr auto _second = utils::exact_second_from_timestr(time_str);\
        static_assert(_hour == hour);\
        static_assert(_minute == minute);\
        static_assert(_second == second);\
    }

    // Basic cases
    TEST_CASE("00:00:00", 0, 0, 0)
    TEST_CASE("12:30:45", 12, 30, 45)
    TEST_CASE("23:59:59", 23, 59, 59)
    
    // Hour boundaries
    TEST_CASE("00:00:01", 0, 0, 1)
    TEST_CASE("00:01:00", 0, 1, 0)
    TEST_CASE("01:00:00", 1, 0, 0)
    TEST_CASE("12:00:00", 12, 0, 0)  // Noon
    TEST_CASE("23:00:00", 23, 0, 0)
    TEST_CASE("23:59:00", 23, 59, 0)
    TEST_CASE("23:00:59", 23, 0, 59)
    
    // Various times throughout the day
    TEST_CASE("01:23:45", 1, 23, 45)
    TEST_CASE("02:34:56", 2, 34, 56)
    TEST_CASE("03:45:01", 3, 45, 1)
    TEST_CASE("04:56:12", 4, 56, 12)
    TEST_CASE("05:12:34", 5, 12, 34)
    TEST_CASE("06:34:56", 6, 34, 56)
    TEST_CASE("07:45:12", 7, 45, 12)
    TEST_CASE("08:56:23", 8, 56, 23)
    TEST_CASE("09:12:34", 9, 12, 34)
    TEST_CASE("10:23:45", 10, 23, 45)
    TEST_CASE("11:34:56", 11, 34, 56)
    TEST_CASE("13:45:01", 13, 45, 1)
    TEST_CASE("14:56:12", 14, 56, 12)
    TEST_CASE("15:12:34", 15, 12, 34)
    TEST_CASE("16:34:56", 16, 34, 56)
    TEST_CASE("17:45:12", 17, 45, 12)
    TEST_CASE("18:56:23", 18, 56, 23)
    TEST_CASE("19:12:34", 19, 12, 34)
    TEST_CASE("20:23:45", 20, 23, 45)
    TEST_CASE("21:34:56", 21, 34, 56)
    TEST_CASE("22:45:01", 22, 45, 1)
    
    // Single digit hours
    TEST_CASE("00:00:00", 0, 0, 0)
    TEST_CASE("01:23:45", 1, 23, 45)
    TEST_CASE("02:34:56", 2, 34, 56)
    TEST_CASE("03:45:01", 3, 45, 1)
    TEST_CASE("04:56:12", 4, 56, 12)
    TEST_CASE("05:12:34", 5, 12, 34)
    TEST_CASE("06:34:56", 6, 34, 56)
    TEST_CASE("07:45:12", 7, 45, 12)
    TEST_CASE("08:56:23", 8, 56, 23)
    TEST_CASE("09:12:34", 9, 12, 34)
    
    // Edge minute values
    TEST_CASE("00:59:00", 0, 59, 0)
    TEST_CASE("00:00:59", 0, 0, 59)
    TEST_CASE("00:59:59", 0, 59, 59)
    TEST_CASE("23:00:00", 23, 0, 0)
    TEST_CASE("23:59:00", 23, 59, 0)
    TEST_CASE("23:00:59", 23, 0, 59)
    TEST_CASE("12:59:59", 12, 59, 59)
    TEST_CASE("12:00:59", 12, 0, 59)
    TEST_CASE("12:59:00", 12, 59, 0)

    #undef TEST_CASE
}
}