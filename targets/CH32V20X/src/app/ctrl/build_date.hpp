#pragma once

#include "core/utils/Option.hpp"
#include "core/string/StringView.hpp"

namespace ymd{
struct Date{
    using Year = uint8_t;
    // using Month = uint8_t;

    struct Month{
        enum class Kind:uint8_t{
            Jan = 1,
            Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
        };


        using enum Kind;

        static constexpr const char * MONTH_STR[] = 
            {"Jan","Feb","Mar","Apr","May","Jun",
            "Jul","Aug","Sep","Oct","Nov","Dec"};

        Kind kind;

        static constexpr Option<Month> from_str(const StringView str){
            // Parse month abbreviation (first 3 chars)

            for (uint8_t m = 0; m < 12; ++m) {
                if (str[0] == MONTH_STR[m][0] && 
                    str[1] == MONTH_STR[m][1] && 
                    str[2] == MONTH_STR[m][2]) {
                    return Some(Month{std::bit_cast<Kind>(uint8_t(m + 1))});
                }
            }
        }

        friend OutputStream & operator <<(OutputStream & os, const Month & self){
            switch(self.kind){
                case Kind::Jan ... Kind::Dec: {
                    const auto str = MONTH_STR[std::bit_cast<uint8_t>(self.kind) - 1];
                    return os << StringView(str,3);

                }
                default: __builtin_unreachable();
            }
        }
    };

    using Day = uint8_t;
    
    Year year;
    Month month;
    Day day;



    static consteval Date from_compiler(){
        return from_str(StringView(__DATE__)).unwrap();
    }

    static constexpr Option<Date> from_str(const StringView str) {
        // Parse day (next 2 chars, space-padded)
        uint8_t d = (str[4] == ' ') ? 
            (str[5] - '0') : 
            ((str[4] - '0') * 10 + (str[5] - '0'));
        
        // Parse year (last 4 chars)
        uint8_t y = (str[8] - '0') * 100 +
                        (str[9] - '0') * 10 +
                        (str[10] - '0');
        
        const auto may_month = Month::from_str(str.substr(0,3));
        if(may_month.is_none()) return None;
        return Some(Date{y, may_month.unwrap(), d});
    }

    friend OutputStream & operator <<(OutputStream & os, const Date & self){
        return os << os.brackets<'{'>() 
            << self.month << '/'
            << self.year << '/' 
            << self.day
            << os.brackets<'}'>()
            ;
    }
};

struct Time{
    using Hour = uint8_t;
    using Minute = uint8_t;
    using Seconds = uint8_t;

    Hour hour;
    Minute minute;
    Seconds seconds;

    // Compile-time time initialization from __TIME__ macro
    static constexpr Time from_compiler() {
        constexpr const char* time_str = __TIME__;
        return {
            static_cast<Hour>((time_str[0]-'0')*10 + (time_str[1]-'0')),
            static_cast<Minute>((time_str[3]-'0')*10 + (time_str[4]-'0')),
            static_cast<Seconds>((time_str[6]-'0')*10 + (time_str[7]-'0'))
        };
    }

    friend OutputStream & operator <<(OutputStream & os, const Time & self){
        return os << os.brackets<'{'>() 
                << self.hour << os.brackets<':'>()
                << self.minute << os.brackets<':'>()
                << self.seconds
                << os.brackets<'}'>()
            ;
    }
};

// static constexpr auto t = Time::from_compiler();
// static constexpr auto d = Date::from_compiler();

struct Author final{
    static constexpr size_t MAX_NAME_LEN = 8;
    static constexpr Option<Author> from_name(const char * name){
        const auto slen = strlen(name);
        if(slen >= MAX_NAME_LEN) return None;
    }

    constexpr StringView name() const{
        return StringView(name_);
    }
private:
    char name_[MAX_NAME_LEN];
};

struct Version{
    uint8_t major;
    uint8_t minor;

    constexpr auto operator<=>(const Version&) const = default;
};

struct ReleaseInfo{
    Author author;
    Version version;
    Date date;
    Time time;

    // Compile-time creation with validation
    static constexpr Option<ReleaseInfo> create(
        const char* author_name,
        Version ver,
        Date d = Date::from_compiler(),
        Time t = Time::from_compiler()
    ) {
        const auto may_author = Author::from_name(author_name);
        if(may_author.is_none()) return None;
        return Some(ReleaseInfo{
            may_author.unwrap(), 
            ver, 
            d, 
            t
        });
    }
};



// [[maybe_unused]] static void test_eeprom(){
//     hal::I2cSw i2c_sw{hal::portD[1], hal::portD[0]};
//     i2c_sw.init(800_KHz);
//     drivers::AT24CXX at24{drivers::AT24CXX::Config::AT24C02{}, i2c_sw};

//     const auto begin_u = clock::micros();
//     uint8_t rdata[3] = {0};
//     at24.load_bytes(0_addr, std::span(rdata)).examine();
//     while(not at24.is_available()){
//         at24.poll().examine();
//     }

//     DEBUG_PRINTLN(rdata);
//     const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
//     at24.store_bytes(0_addr, std::span(data)).examine();

//     while(not at24.is_available()){
//         at24.poll().examine();
//     }

//     DEBUG_PRINTLN("done", clock::micros() - begin_u);
//     while(true);
// }

class ArchiveSystem{};
}