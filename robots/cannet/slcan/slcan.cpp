#include "slcan.hpp"

using namespace ymd;
using namespace ymd::robots::slcan;
using namespace ymd::robots::asciican;
using namespace operations;

// #define SLCAN_DEBUG_EN 1
#define SLCAN_DEBUG_EN 0

#define SLCAN_STATIC_TEST_EN 1

#if SLCAN_DEBUG_EN
#define constexpr
#define RETURN_ERR(e, ...) {DEBUG_PRINTLN(__VA_ARGS__); return Err(e);}
#else
#define RETURN_ERR(e, ...) {return Err(e);}
#endif

using Error = Slcan::Error;



template<typename T = void>
using IResult = Result<T, Error>;

[[nodiscard]] static constexpr Option<uint8_t> 
hex2digit(char c){
    switch(c){
        case '0' ... '9':
            return Some(c - '0');
        case 'a' ... 'f':
            return Some(c - 'a' + 10);
        case 'A' ... 'F':
            return Some(c - 'A' + 10);
        default:
            return None;
    }
}

[[nodiscard]] static constexpr char 
digit2hex(const uint8_t digit){
    switch(digit){
        case 0 ... 9:
            return digit + '0';
        case 10 ... 15:
            return digit - 10 + 'a';
        default:
            return 0;
    }
}


[[nodiscard]] static constexpr IResult<uint32_t> 
parse_hex(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);

    uint32_t ret = 0;

    for(size_t i = 0; i < str.length(); i++){
        const auto chr = str[i];
        const auto may_digit = hex2digit(chr);

        if(may_digit.is_none())
            RETURN_ERR(Error::UnsupportedHexChar, chr);

        ret = static_cast<uint32_t>(ret << 4) | static_cast<uint32_t>(may_digit.unwrap());
    }

    return Ok(ret);
}

[[nodiscard]] static constexpr IResult<uint32_t> 
parse_hex(const char c1, const char c2){

    const auto d1 = ({
        const auto may_digit = hex2digit(c1);
        if(may_digit.is_none()) return Err(Error::UnsupportedHexChar);
        may_digit.unwrap();
    });


    const auto d2 = ({
        const auto may_digit = hex2digit(c2);
        if(may_digit.is_none()) return Err(Error::UnsupportedHexChar);
        may_digit.unwrap();
    });


    return Ok(static_cast<uint32_t>(d1 << 4 | d2));
}

[[nodiscard]] static constexpr IResult<hal::CanStdId> 
parse_std_id(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);
    if(str.length() > 3)
        RETURN_ERR(Error::StdIdTooLong);
    if(str.length() < 3)
        RETURN_ERR(Error::StdIdTooShort);

    const auto either_id = parse_hex(str);
    
    if(either_id.is_err())
        RETURN_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > 0x7ff)
        RETURN_ERR(Error::StdIdOverflow);

    return Ok(hal::CanStdId(id));
}

#if SLCAN_STATIC_TEST_EN == 1
static_assert(parse_std_id("123").unwrap().to_u11() == 0x123);
static_assert(parse_std_id("923").unwrap_err() == Error::StdIdOverflow);
static_assert(parse_std_id("9scd").unwrap_err() == Error::StdIdTooLong);
static_assert(parse_std_id("9sc").unwrap_err() == Error::UnsupportedHexChar);
#endif

[[nodiscard]] static constexpr IResult<hal::CanExtId> 
parse_ext_id(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);
    if(str.length() > 8)
        RETURN_ERR(Error::ExtIdTooLong);
    if(str.length() < 8)
        RETURN_ERR(Error::ExtIdTooShort);


    const auto either_id = parse_hex(str);
    
    if(either_id.is_err())
        RETURN_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > ((1u << 29) - 1))
        RETURN_ERR(Error::ExtIdOverflow);

    return Ok(hal::CanExtId(id));
}

[[nodiscard]] static constexpr IResult<std::array<uint8_t, 8>> parse_payload(
    const StringView str, 
    const size_t dlc
){
    if(str.size() != dlc * 2) 
        RETURN_ERR(Error::PayloadLengthMismatch, str.size(), dlc, str);

    std::array<uint8_t, 8> buf;

    for(size_t i = 0; i < dlc; i++){
        const auto either_data8 = parse_hex(
            str[i * 2], str[i * 2 + 1]
        );
        if(either_data8.is_err())
            RETURN_ERR(either_data8.unwrap_err());

        buf[i] = either_data8.unwrap();
    }

    return Ok{buf};
}


#if SLCAN_STATIC_TEST_EN == 0
static_assert(parse_payload("11x", 1).unwrap_err() == Error::PayloadLengthMismatch);
static_assert(parse_payload("11", 1).unwrap()[0] == 0x11);
static_assert(parse_payload("1122334455667788", 8).unwrap()[7] == 0x88);
#endif

[[nodiscard]] static constexpr 
IResult<size_t> parse_len(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);
    if(str.length() != 1) 
        RETURN_ERR(Error::PayloadLengthMismatch, str.length());

    const auto len = ({
        const auto either_len = parse_hex(str);
        
        if(either_len.is_err()) 
            RETURN_ERR(either_len.unwrap_err());
        either_len.unwrap();
    });

    if(len > 8) 
        RETURN_ERR(Error::PayloadLengthOverflow, len);

    return Ok(len);
}

template<bool is_ext>
[[nodiscard]] static constexpr auto 
parse_id_as_u32(const StringView str) -> IResult<uint32_t>{
    if constexpr(is_ext){
        return parse_ext_id(str).
            map([](const hal::CanExtId id) -> uint32_t{return id.to_u29();}); 
    } else {
        return parse_std_id(str).
            map([](const hal::CanStdId id) -> uint32_t{return id.to_u11();}); 
    }
};

template<bool is_ext>
[[nodiscard]] static constexpr IResult<hal::CanMsg> 
parse_msg(const StringView str, bool is_rmt){
    if(str.size() == 0) 
        RETURN_ERR(Error::NoArg);

    constexpr size_t ID_LEN = is_ext ? 8 : 3;
    constexpr size_t DLC_LEN = 1;

    if(str.length() < 4)
        RETURN_ERR(Error::ArgTooShort, str.length());

    auto cutter = StringCutter{str};

    const uint32_t id_u32_checked = ({
        const auto res = parse_id_as_u32<is_ext>(cutter.fetch_next(ID_LEN).unwrap());
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    const size_t dlc = ({
        const auto res = parse_len(cutter.fetch_next(DLC_LEN).unwrap());
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    const StringView payload_str = cutter.fetch_remaining().unwrap();
    
    using ID = std::conditional_t<is_ext, hal::CanExtId, hal::CanStdId>;
    if(not is_rmt){
        const auto payload = ({
            const auto res = parse_payload(payload_str, dlc);
            if(res.is_err()) RETURN_ERR(res.unwrap_err());
            res.unwrap();
        });

        const auto bytes = std::span(payload.data(), dlc);
        
        return Ok(hal::CanMsg::from_bytes(ID(id_u32_checked), bytes));

    }else{
        if(payload_str.size()) 
            RETURN_ERR(Error::InvalidFieldInRemoteMsg);
        return Ok(hal::CanMsg::from_remote(ID(id_u32_checked)));
    }
}

// static_assert(int(parse_msg("0C34567F20102", true).unwrap_err()) == 0);
// static constexpr auto l = parse_msg("0A70", true).unwrap().size();


#if 0
[[nodiscard]] static void static_test(){

    // static constexpr auto msg = parse_msg("0A7111", true).unwrap();
    // static constexpr auto msg = parse_msg("0A70", true).unwrap();
    static_assert(msg.size() == 0);
    static_assert(msg.id() == 0x0a7);
    // static_assert(parse_std_id("0A7111", true).unwrap();
    // static_assert(parse_std_id("0A7").unwrap() == 0x0a7);
    // static_assert(parse_std_id("123").unwrap() == 0x123);
    // static_assert(parse_len("7").unwrap() == 7);
    // static_assert(parse_msg("0A70", true).unwrap().size() == 0);
}
#endif


auto map_chr_to_buad = [](char chr) -> hal::CanBaudrate{ 
    switch(chr){
        case '0': return hal::CanBaudrate::_10K;
        case '1': return hal::CanBaudrate::_20K;
        case '2': return hal::CanBaudrate::_50K;
        case '3': return hal::CanBaudrate::_100K;
        case '4': return hal::CanBaudrate::_125K;
        case '5': return hal::CanBaudrate::_250K;
        case '6': return hal::CanBaudrate::_500K;
        case '7': return hal::CanBaudrate::_800K;
        case '8': return hal::CanBaudrate::_1M;
    }
    __builtin_unreachable();
};

auto map_msg_to_operation = [](const hal::CanMsg & msg) { return Operation(SendCanMsg{msg}); };

IResult<Operation> Slcan::handle_line(const StringView str) const {
    static constexpr bool REMOTE = true;

    const StringView line = str.trim();
    // DEBUG_PRINTLN(line, line.length());
    if(line.size() == 0){
        RETURN_ERR(Error::NoInput);
    }else if(line.size() == 1){
        switch(line[0]){
            case 'F': return Ok(Operation(response_flag()));
            case 'V': return Ok(Operation(response_version()));
            case 'N': return Ok(Operation(response_serial_idx()));
            
            case 'O': return Ok(Operation(Open{}));
            case 'C': return Ok(Operation(Close{}));

            case 'Z': RETURN_ERR(Error::NotImplemented);
        }
        RETURN_ERR(Error::UnknownCommand);
    }else{
        const auto cmd_line = line.substr(1).unwrap();

        switch(line[0]){
            case 'S': {
                if(cmd_line.size() == 0) 
                    RETURN_ERR(Error::NoArg);
                if(cmd_line.size() > 1) 
                    RETURN_ERR(Error::ArgTooLong);
                const auto chr = cmd_line[0];
                if(chr < '0' || chr > '8') 
                    return Err(Error::InvalidBaudrate);

                return Ok(Operation(SetCanBaud{.baudrate = map_chr_to_buad(chr)}));
            }

            case 't': return parse_msg<false>(cmd_line, not REMOTE)
                .map(map_msg_to_operation);
            case 'r': return parse_msg<false>(cmd_line, REMOTE)
                .map(map_msg_to_operation);

            case 'T': return parse_msg<true>(cmd_line, not REMOTE)
                .map(map_msg_to_operation);
            case 'R': return parse_msg<true>(cmd_line, REMOTE)
                .map(map_msg_to_operation);

        }
        RETURN_ERR(Error::UnknownCommand);
    }

    __builtin_unreachable();
}

SendText Slcan::response_version() const{
    return SendText::from_str("V1013\r");
}

SendText Slcan::response_serial_idx() const{
    return SendText::from_str("NA123\r");
}

Slcan::Flags Slcan::get_flag() const {
    return Flags::RxFifoFull;
}

SendText Slcan::response_flag() const{
    const char str[2] = {
        char(get_flag()), 'r'
    };

    return SendText::from_str(StringView(str, 2));
}