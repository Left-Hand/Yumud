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

    uint32_t dot_pos = UINT32_MAX;
    for(size_t i = 0; i < str.length(); i++){
        if(str[i] == '.'){
            dot_pos = i;
            break;
        }
    }

    if(dot_pos == UINT32_MAX) return Err(DeMsgErrorKind::NoDot);
    if(dot_pos == 0) return Err(DeMsgErrorKind::NoDigit);
    if(dot_pos == str.length() - 1) return Err(DeMsgErrorKind::NoFrac);

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
static_assert(parse_floating_num("2240.").unwrap_err() == DeMsgErrorKind::NoFrac);
static_assert(parse_floating_num(".2240").unwrap_err() == DeMsgErrorKind::NoDigit);
static_assert(parse_floating_num("22").unwrap_err() == DeMsgErrorKind::NoDot);

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



template<typename T>
struct FieldsTraiter{
    static constexpr size_t NUM_FIELDS = reflect::size<T>();
    template<size_t I>
    static constexpr auto & get_element(T & self){
        return reflect::get<I>(self);
    }
};


template<typename T>
struct [[nodiscard]] FieldExacter{
    using Error = DeMsgErrorKind;

    static constexpr Result<void, Error> exact_field(T & field, const StringView str){
        if(str.length() == 0) return Err(Error::NonnullObject);

        field = ({
            const auto res = T::try_from_str(str);
            if(res.is_err()) return Err(res.unwrap_err());
            (res.unwrap());
        });
        return Ok();
    }
};

template<typename T>
struct [[nodiscard]] FieldExacter<Option<T>>{
    using Error = DeMsgErrorKind;

    static constexpr Result<void, Error> exact_field(Option<T> & field, const StringView str){
        if(str.length() == 0){
            field = None;
            return Ok();
        }

        field = ({
            const auto res = T::try_from_str(str);
            if(res.is_err()) return Err(res.unwrap_err());
            Some(res.unwrap());
        });
        return Ok();
    }
};



template<typename T>
struct ConvertHelper {
    template <size_t I, size_t... Js>
    static constexpr Result<void, DeMsgErrorKind>
    parse_remeaning(T & self, StringSplitIter splitter, std::index_sequence<I, Js...>) {
        if(splitter.has_next() == false) return Err(DeMsgErrorKind::MissingField); 
        const auto str = splitter.next();
        auto & field = FieldsTraiter<T>::template get_element<I>(self);
        using ElementType = std::decay_t<decltype(field)>;
        if(const auto res = FieldExacter<ElementType>::exact_field(field, str);
            res.is_err()) return Err(res.unwrap_err());
        return parse_remeaning(self, std::move(splitter), std::index_sequence<Js...>{});
    }

    template <typename... Args>
    static constexpr Result<void, DeMsgErrorKind>
    parse_remeaning(
        [[maybe_unused]] T & self, 
        StringSplitIter splitter, 
        [[maybe_unused]] std::index_sequence<>
    ) {
        if(splitter.has_next()) return Err(DeMsgErrorKind::TooMuchFields);
        return Ok();
    }

private:
    //空时返回nullptr
    [[nodiscard]] const char * search_comma(const StringView str){
        const auto it = std::find(str.begin(), str.end(), ',');
        if(it == str.end()) return nullptr;
        return static_cast<const char *>(it);
    }
};


template<typename T>
static constexpr Result<T, DeMsgErrorKind> try_deser_msg(const StringView context_str){
    static constexpr size_t NUM_FIELDS = FieldsTraiter<T>::NUM_FIELDS;
    T self = T::from_uninitialized();
    const auto res = ConvertHelper<T>::parse_remeaning(
        self, StringSplitIter(context_str, ','), std::make_index_sequence<NUM_FIELDS>{});
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(self);
}


namespace msgs{
struct GGA{
    using Self = GGA;
    using Error = DeMsgErrorKind;


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
    }

};


}


[[maybe_unused]] static void test_gga(){
    // constexpr StringView line = "023656.00";
    constexpr StringView line = "023656.00,2240.61563,N,11359.86512,E";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "023656.00,2240.61563,N,11359.86512";
    // constexpr StringView line = "$BDGGA,023656.00,2240.61563,N,11359.86512,E,1,23,0.7,96.53,M,-3.52,M,,*5A";


    // constexpr auto err = GGA::try_from_str(line).unwrap_err();
    // static_assert(int(err) == 0);
    // constexpr auto msg = GGA::try_from_str(line).unwrap();
    constexpr auto msg = try_deser_msg<msgs::GGA>(line).unwrap();

    static_assert(msg.utc_time.is_some());
    static_assert(msg.utc_time.unwrap().digit == 23656);

    static_assert(msg.ulat.unwrap().is_north == true);

    static_assert(msg.lon.is_some());
    static_assert(msg.lon.unwrap().digit == 11359);
    static_assert(msg.lon.unwrap().frac == 86512);

    static_assert(msg.ulon.unwrap().is_eastern == true);

}
}