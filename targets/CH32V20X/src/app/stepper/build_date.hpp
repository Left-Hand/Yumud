#pragma once

#include "core/utils/Option.hpp"
#include "core/string/StringView.hpp"

namespace ymd{

using Year = uint8_t;
// using Month = uint8_t;

struct Month final{
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

        return None;
    }


    constexpr bool operator == (const Month & rhs) const{
        return kind == rhs.kind;
    }

    constexpr bool operator < (const Month & rhs) const{
        return std::bit_cast<uint8_t>(kind) < std::bit_cast<uint8_t>(rhs.kind);
    }


    constexpr bool is_valid() const {
        switch(kind){
            case Kind::Jan ... Kind::Dec: return true;
            default: return false;
        }
    }


    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const Month & self){
        return hs << uint8_t(self.kind);
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

using Hour = uint8_t;
using Minute = uint8_t;
using Seconds = uint8_t;

struct Date final{

    
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
        
        const auto may_month = Month::from_str(str.substr_by_range(0,3));
        if(may_month.is_none()) return None;
        return Some(Date{y, may_month.unwrap(), d});
    }


    constexpr bool is_valid() const {
        return IN_RANGE(year, 24, 30) //no one will use this shit after 2030
            and month.is_valid()
            and IN_RANGE(day, 1, 31);
    }

    constexpr bool operator == (const Date & rhs){
        return year == rhs.year and month == rhs.month and day == rhs.day;
    }

    constexpr bool operator < (const Date & rhs){
        if(year != rhs.year){
            return year < rhs.year;
        }

        if(month != rhs.month){ 
            return month < rhs.month;
        }

        return day < rhs.day;
    }

    constexpr bool is_latest() const {
        return *this == Date::from_compiler();
    }


    friend OutputStream & operator <<(OutputStream & os, const Date & self){
        return os << os.brackets<'{'>() 
            << self.month << '/'
            << self.year << '/' 
            << self.day
            << os.brackets<'}'>()
            ;
    }

    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const Date & self){
        return hs << self.year << self.month << self.day;
    }
};

struct Time final{

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

    constexpr bool is_valid() const {
        return (hour < 24) && (minute < 60) && (seconds < 60);
    }

    constexpr bool operator == (const Time & rhs) const{
        return (hour == rhs.hour) && (minute == rhs.minute) && (seconds == rhs.seconds);
    }

    constexpr bool operator < (const Time & rhs) const {
        // 层级比较：小时 → 分钟 → 秒
        if (hour != rhs.hour) {
            return hour < rhs.hour;
        }
        if (minute != rhs.minute) {
            return minute < rhs.minute;
        }
        return seconds < rhs.seconds;
    }

    constexpr bool is_latest() const{
        return *this == Time::from_compiler();
    }

    friend OutputStream & operator <<(OutputStream & os, const Time & self){
        return os << os.brackets<'{'>() 
                << self.hour << os.brackets<':'>()
                << self.minute << os.brackets<':'>()
                << self.seconds
                << os.brackets<'}'>()
            ;
    }

    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const Time & self){
        return hs << self.hour << self.minute << self.seconds;
    }

};


struct Author final{
    static constexpr size_t MAX_NAME_LEN = 8;
    static constexpr Option<Author> from(const char * name){
        const auto slen = strlen(name);
        if(slen >= MAX_NAME_LEN) return None;
        return Some(Author{name});
    }

    constexpr StringView name() const{
        return StringView(name_.data(), 8);
    }

    friend OutputStream & operator<<(OutputStream & os, const Author & self){ 
        return os << "name: " << self.name();
    }

    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const Author & self){
        return hs << self.name();
    }

private:
    constexpr Author(const char * name){
        name_.fill(0);
        for(size_t i = 0; i < MAX_NAME_LEN && name[i]; ++i){
            name_[i] = name[i];
        }
    }
    std::array<char, MAX_NAME_LEN> name_;
};

struct Version{
    uint8_t major;
    uint8_t minor;

    constexpr auto operator<=>(const Version&) const = default;

    friend OutputStream & operator <<(OutputStream & os, const Version & self){
        return os << os.brackets<'{'>() 
                << self.major << os.brackets<':'>()
                << self.minor
                << os.brackets<'}'>()
            ;
    }

    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const Version & self){
        return hs << self.major << self.minor;
    }
};

struct ReleaseInfo{
    Author author;
    Version version;
    Date date;
    Time time;

    // Compile-time creation with validation
    static constexpr Option<ReleaseInfo> from(
        const char* author_name,
        Version ver,
        Date d = Date::from_compiler(),
        Time t = Time::from_compiler()
    ) {
        const auto may_author = Author::from(author_name);
        if(may_author.is_none()) return None;
        return Some(ReleaseInfo{
            may_author.unwrap(), 
            ver, 
            d, 
            t
        });
    }

    friend OutputStream & operator <<(OutputStream & os, const ReleaseInfo & self){
        os << os.scoped("release")(
            os 
            << os.field("author")(os << self.author)
            << os.field("version")(os << self.version)
            << os.field("date")(os << self.date)
            << os.field("time")(os << self.time)
        );
        return os;
    }

    template<HashAlgo S>
    constexpr friend Hasher<S> & operator << (Hasher<S> & hs, const ReleaseInfo & self){
        return hs << self.author << self.version << self.date << self.time;
    }
};


}