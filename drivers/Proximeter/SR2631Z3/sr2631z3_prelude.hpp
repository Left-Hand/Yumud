#pragma once

#include "core/string/view/string_view.hpp"
#include "core/string/owned/char_array.hpp"
#include "core/string/owned/heapless_string.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/utils/Result.hpp"
#include "core/polymorphism/reflect.hpp"

namespace ymd::drivers::sr2631z3{

enum class [[nodiscard]] DeMsgErrorKind:uint8_t{
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
    MissingField,
    TooMuchFields,
    NonnullObject
};


static constexpr Result<uint8_t, DeMsgErrorKind> 
parse_dec_nibble(const char c){
    switch(c){
        case '0' ... '9':
            return Ok(c - '0');
        default:
            return Err(DeMsgErrorKind::InvalidDecChar);
    }
}

static constexpr Result<uint8_t, DeMsgErrorKind> 
parse_hex_nibble(const char c){
    switch(c){
        case '0' ... '9':
            return Ok(c - '0');
        case 'a' ... 'f':
            return Ok((c - 'a') + 10);
        case 'A' ... 'F':
            return Ok((c - 'A') + 10);
        default:
            return Err(DeMsgErrorKind::InvalidHexChar);
    }
}

static constexpr Result<uint8_t, DeMsgErrorKind> 
parse_hex_str(const StringView str){
    if(str.length() == 0) return Err(DeMsgErrorKind::EmptyString);
    if(str.length() > 2) return Err(DeMsgErrorKind::StringTooLong);

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

static constexpr Result<uint32_t, DeMsgErrorKind> 
parse_dec_str(const StringView str){
    if(str.length() == 0) return Err(DeMsgErrorKind::EmptyString);

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

static constexpr Result<std::tuple<uint32_t, uint32_t>, DeMsgErrorKind> 
parse_floating_num(const StringView str){
    if(str.length() == 0) return Err(DeMsgErrorKind::EmptyString);

    const auto dot_it = std::find(str.begin(), str.end(), '.');

    if(dot_it >= str.end()) return Err(DeMsgErrorKind::NoDot);
    if(dot_it == str.begin()) return Err(DeMsgErrorKind::NoDigit);
    if(dot_it == std::prev(str.end())) return Err(DeMsgErrorKind::NoFrac);

    const uint32_t arg1 = ({
        const auto res = parse_dec_str(StringView(str.begin(), dot_it));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const uint32_t arg2 = ({
        const auto res = parse_dec_str(StringView(std::next(dot_it), str.end()));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    return Ok(std::make_tuple(arg1, arg2));
}



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

    static constexpr Result<Self, DeMsgErrorKind> try_from_str(const StringView str){
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

    static constexpr Result<Self, DeMsgErrorKind> try_from_str(const StringView str){
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

    static constexpr Result<Self, DeMsgErrorKind> try_from_str(const StringView str){
        if(str.length() == 0) return Err(DeMsgErrorKind::EmptyString);
        switch(str[0]){
            case 'N': return Ok(Self{.is_north = true});
            case 'S': return Ok(Self{.is_north = false});
        }

        return Err(DeMsgErrorKind::InvalidChar);
    }
};

struct [[nodiscard]] Lon{
    using Self = Lon;

    uint32_t digit;
    uint32_t frac;

    static constexpr Result<Self, DeMsgErrorKind> try_from_str(const StringView str){
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

    static constexpr Result<Self, DeMsgErrorKind> try_from_str(const StringView str){
        if(str.length() == 0) return Err(DeMsgErrorKind::EmptyString);
        switch(str[0]){
            case 'E': return Ok(Self{.is_eastern = true});
            case 'W': return Ok(Self{.is_eastern = false});
        }

        return Err(DeMsgErrorKind::InvalidChar);
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
};

struct [[nodiscard]] LineParts {

    using Self = LineParts;
    using Error = DeMsgErrorKind;

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
    [[nodiscard]] static constexpr uint8_t calc_str_crc(const StringView str){
        uint8_t sum = 0;
        for(char chr : str){
            sum = sum ^ static_cast<uint8_t>(chr);
        }
        return sum;
    }
};




}