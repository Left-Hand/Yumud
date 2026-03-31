#pragma once

#include "core/string/view/string_view.hpp"
#include "core/string/owned/char_array.hpp"
#include "core/string/owned/heapless_string.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/utils/Result.hpp"

namespace ymd::drivers::sr2631z3{

enum class DeMsgError:uint8_t{
    EmptyString,
    StringTooLong,
    InvalidHexChar,
    InvalidDecChar,
    NoDot,
    NoDigit,
    NoFrac,
    InvalidLineBegin,
    CrcSplitterNotFounded,
    CommaNotFounded,
    CrcMismatch,
    InvalidMsgKind,
    InvalidChar,
    MissingParaments,
    TooMuchParaments,
};


static constexpr Result<uint8_t, DeMsgError> 
parse_dec_nibble(const char c){
    switch(c){
        case '0' ... '9':
            return Ok(c - '0');
        default:
            return Err(DeMsgError::InvalidDecChar);
    }
}

static constexpr Result<uint8_t, DeMsgError> 
parse_hex_nibble(const char c){
    switch(c){
        case '0' ... '9':
            return Ok(c - '0');
        case 'a' ... 'f':
            return Ok((c - 'a') + 10);
        case 'A' ... 'F':
            return Ok((c - 'A') + 10);
        default:
            return Err(DeMsgError::InvalidHexChar);
    }
}

static constexpr Result<uint8_t, DeMsgError> 
parse_hex_str(const StringView str){
    if(str.length() == 0) return Err(DeMsgError::EmptyString);
    if(str.length() > 2) return Err(DeMsgError::StringTooLong);

    uint8_t sum = 0;
    for(size_t i = 0; i < 2; i++){
        const auto digit = ({
            const auto res = parse_hex_nibble(str[i]);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        sum = (sum << 4) + digit;
    }

    return Ok(sum);
}

static constexpr Result<uint32_t, DeMsgError> 
parse_dec_str(const StringView str){
    if(str.length() == 0) return Err(DeMsgError::EmptyString);

    uint32_t sum = 0;
    for(size_t i = 0; i < str.length(); i++){
        const uint32_t digit = ({
            const auto res = parse_hex_nibble(str[i]);
            if(res.is_err()) return Err(res.unwrap_err());
            static_cast<uint32_t>(res.unwrap());
        });

        sum = (sum * 10) + digit;
    }

    return Ok(sum);
}

static constexpr Result<std::tuple<uint32_t, uint32_t>, DeMsgError> 
parse_floating_num(const StringView str){
    if(str.length() == 0) return Err(DeMsgError::EmptyString);

    uint32_t dot_pos = UINT32_MAX;
    for(size_t i = 0; i < str.length(); i++){
        if(str[i] == '.'){
            dot_pos = i;
            break;
        }
    }

    if(dot_pos == UINT32_MAX) return Err(DeMsgError::NoDot);
    if(dot_pos == 0) return Err(DeMsgError::NoDigit);
    if(dot_pos == str.length() - 1) return Err(DeMsgError::NoFrac);

    const uint32_t arg1 = ({
        const auto res = parse_dec_str(StringView(str.begin(), dot_pos));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const uint32_t arg2 = ({
        const auto res = parse_dec_str(StringView(str.begin() + (dot_pos + 1), str.end()));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    return Ok(std::make_tuple(arg1, arg2));
}



static_assert(parse_hex_str("5b").unwrap() == 0x5b);
static_assert(parse_dec_str("023656").unwrap() == 23656);
static_assert(std::get<0>(parse_floating_num("2240.61563").unwrap()) == 2240);
static_assert(std::get<1>(parse_floating_num("2240.61563").unwrap()) == 61563);
static_assert(parse_floating_num("2240.").unwrap_err() == DeMsgError::NoFrac);
static_assert(parse_floating_num(".2240").unwrap_err() == DeMsgError::NoDigit);
static_assert(parse_floating_num("22").unwrap_err() == DeMsgError::NoDot);

struct [[nodiscard]] SysIdentifier{
    using Self = SysIdentifier;
    StringView str;


    static constexpr Self from_str(const StringView str){
        return Self{.str = str};
    }
};

struct [[nodiscard]] UtcTime{
    using Self = UtcTime;
    uint32_t digit;
    uint32_t frac;

    static constexpr Result<Self, DeMsgError> try_from_str(const StringView str){
        const auto [digit, frac] = ({
            const auto res = parse_floating_num(str);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        return Ok(Self{
            .digit = digit,
            .frac = frac
        });
    }
};

struct [[nodiscard]] Lat{
    using Self = Lat;

    uint32_t digit;
    uint32_t frac;

    static constexpr Result<Self, DeMsgError> try_from_str(const StringView str){
        const auto [digit, frac] = ({
            const auto res = parse_floating_num(str);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        return Ok(Self{
            .digit = digit,
            .frac = frac
        });
    }
};

struct [[nodiscard]] uLat{
    using Self = uLat;


    bool is_north;

    static constexpr Result<Self, DeMsgError> try_from_str(const StringView str){
        if(str.length() == 0) return Err(DeMsgError::EmptyString);
        switch(str[0]){
            case 'N': return Ok(Self{.is_north = true});
            case 'S': return Ok(Self{.is_north = false});
        }

        return Err(DeMsgError::InvalidChar);
    }
};

struct [[nodiscard]] Lon{
    using Self = Lon;

    uint32_t digit;
    uint32_t frac;

    static constexpr Result<Self, DeMsgError> try_from_str(const StringView str){
        const auto [digit, frac] = ({
            const auto res = parse_floating_num(str);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        return Ok(Self{
            .digit = digit,
            .frac = frac
        });
    }
};

struct [[nodiscard]] uLon{
    using Self = uLon;

    bool is_eastern;

    static constexpr Result<Self, DeMsgError> try_from_str(const StringView str){
        if(str.length() == 0) return Err(DeMsgError::EmptyString);
        switch(str[0]){
            case 'E': return Ok(Self{.is_eastern = true});
            case 'W': return Ok(Self{.is_eastern = false});
        }

        return Err(DeMsgError::InvalidChar);
    }
};

struct [[nodiscard]] Fs{

};

struct [[nodiscard]] NumSv{
    uint8_t count;
};

struct [[nodiscard]] Sep{

};

struct [[nodiscard]] uSep{

};

struct [[nodiscard]] MsgKind{
    using Self = MsgKind;
    StringView str;

    static constexpr Self from_str(const StringView str){
        return Self{.str = str};
    } 
    // static constexpr Result<void, DeMsgError> validate(const StringView str){

    // };
};

struct [[nodiscard]] LineParts {
    // struct InvalidCrcError{
    //     uint8_t actual;
    //     uint8_t expected;
    // };

    // struct Error{
    //     enum class Kind{
    //         NoHeader
    //     };

    //     Kind kind;
    //     union{
    //         InvalidCrcError invalid_crc;
    //     };
    // };

    using Self = LineParts;
    using Error = DeMsgError;

    SysIdentifier sys_id;
    MsgKind msg_kind;
    StringView context_str;

    static constexpr Result<LineParts, Error> try_from_str(StringView line){
        line = line.trim();
        if(line[0] != '$') return Err(Error::InvalidLineBegin);
        const auto begin_it = std::next(line.begin());

        const auto star_it = std::find(line.begin(), line.end(), '*');
        if(star_it >= line.end()) return Err(Error::CrcSplitterNotFounded);

        const auto context_str = StringView(begin_it, star_it);
        const auto context_crc = calc_str_crc(context_str);

        const auto expected_crc = ({
            const auto crc_str = StringView(std::next(star_it), line.end());
            const auto res = parse_hex_str(crc_str);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        if(context_crc != expected_crc) return Err(Error::CrcMismatch);

        const auto comma_it = std::find(line.begin(), line.end(), ',');
        if(comma_it >= line.end()) return Err(Error::CommaNotFounded);

        const auto self = Self{
            .sys_id = SysIdentifier::from_str(StringView(line.begin() + 1, 2)),
            .msg_kind = MsgKind::from_str(StringView(line.begin() + 3, comma_it)),
            .context_str = StringView(comma_it + 1, star_it)
        };

        return Ok(self);
    }

private:
    static constexpr uint8_t calc_str_crc(const StringView str){
        uint8_t sum = 0;
        for(char chr : str){
            sum = sum ^ static_cast<uint8_t>(chr);
        }
        return sum;
    }
};



[[maybe_unused]] static void test_parts(){
    constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5B";
    // constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5A";
    constexpr auto parts = LineParts::try_from_str(line).unwrap();

    static_assert(parts.sys_id.str.hash() == "BD"_ha);
    static_assert(parts.msg_kind.str.hash() == "GGA"_ha);
    static_assert(parts.context_str.hash() == "023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,"_ha);
}



namespace msgs{
struct GGA{
    using Self = GGA;
    using Error = DeMsgError;

    static constexpr size_t NUM_PARAMENTS = 5;


    Option<UtcTime> utc_time;
    Option<Lat> lat;
    Option<uLat> ulat;
    Option<Lon> lon;
    Option<uLon> ulon;

    static constexpr Self from_uninitialized(){
        return Self{
            .utc_time = None,
            .lat = None,
            .ulat = None,
            .lon = None,
            .ulon = None
        };

        // return Self{};
    }

    static constexpr Result<Self, Error> try_from_str(const StringView context_str){
        Self self = Self::from_uninitialized();
        StringSplitIter splitter(context_str, ',');

        size_t arg_rank = 0;

        while(arg_rank < NUM_PARAMENTS){
            if(not splitter.has_next()) return Err(Error::MissingParaments);
            const auto str = splitter.next();
            switch(arg_rank){
                case 0:{

                    if(str.length() == 0){
                        self.utc_time = None; 
                        break;
                    }

                    self.utc_time = ({
                        const auto res = UtcTime::try_from_str(str);
                        if(res.is_err()) return Err(res.unwrap_err());
                        Some(res.unwrap());
                    });
                }
                    break;
                case 1:{
                    if(str.length() == 0){
                        self.lat = None; 
                        break;
                    }

                    self.lat = ({
                        const auto res = Lat::try_from_str(str);
                        if(res.is_err()) return Err(res.unwrap_err());
                        Some(res.unwrap());
                    });
                }
                    break;
                case 2:{
                    if(str.length() == 0){
                        self.ulat = None; 
                        break;
                    }

                    self.ulat = ({
                        const auto res = uLat::try_from_str(str);
                        if(res.is_err()) return Err(res.unwrap_err());
                        Some(res.unwrap());
                    });
                }
                    break;
                case 3:{
                    if(str.length() == 0){
                        self.lon = None; 
                        break;
                    }

                    self.lon = ({
                        const auto res = Lon::try_from_str(str);
                        if(res.is_err()) return Err(res.unwrap_err());
                        Some(res.unwrap());
                    });
                }
                    break;
                case 4:{
                    if(str.length() == 0){
                        self.ulon = None; 
                        break;
                    }

                    self.ulon = ({
                        const auto res = uLon::try_from_str(str);
                        if(res.is_err()) return Err(res.unwrap_err());
                        Some(res.unwrap());
                    });
                }
                    break;

            }

            arg_rank ++;
        }

        if(splitter.has_next()) return Err(Error::TooMuchParaments);

        return Ok(self);
    } 
};


[[maybe_unused]] static void test_gga(){
    // constexpr StringView line = "023656.00";
    constexpr StringView line = "023656.00,2240.61563,N,11359.86512,E";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5A";


    // constexpr auto err = GGA::try_from_str(line).unwrap_err();
    // static_assert(int(err) == 0);
    constexpr auto msg = GGA::try_from_str(line).unwrap();

    static_assert(msg.utc_time.is_some());
    static_assert(msg.utc_time.unwrap().digit == 23656);

    static_assert(msg.ulat.unwrap().is_north == true);

    static_assert(msg.lon.is_some());
    static_assert(msg.lon.unwrap().digit == 11359);
    static_assert(msg.lon.unwrap().frac == 86512);

    static_assert(msg.ulon.unwrap().is_eastern == true);

}

}
}