#include "slcan.hpp"

using namespace ymd;
using namespace ymd::robots::slcan;
using namespace ymd::asciican;
using namespace asciican::primitive::operations;

// #define SLCAN_DEBUG_EN 1
#define SLCAN_DEBUG_EN 0


#if SLCAN_DEBUG_EN
#define constexpr
#define RETURN_ERR(e, ...) {DEBUG_PRINTLN(__VA_ARGS__); return Err(e);}
#else
#define RETURN_ERR(e, ...) ({return Err(e);})
#endif

template<typename T = void>
using IResult = Result<T, Error>;

[[nodiscard]] static constexpr 
Option<uint8_t> char2digit(char c){
    switch(c){
        case '0' ... '9':
            return Some(c - '0');
        case 'a' ... 'f':
            return Some((c - 'a') + 10);
        case 'A' ... 'F':
            return Some((c - 'A') + 10);
        default:
            return None;
    }
}

[[nodiscard]] static constexpr 
char digit2char(const uint8_t digit){
    switch(digit){
        case 0 ... 9:
            return digit + '0';
        case 10 ... 15:
            return (digit - 10) + 'a';
        default:
            return 0;
    }
}


[[nodiscard]] static constexpr 
IResult<uint32_t> parse_hex_str(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);

    uint32_t ret = 0;

    #pragma GCC unroll 8
    for(size_t i = 0; i < str.length(); i++){
        const auto chr = str[i];
        const auto may_digit = char2digit(chr);

        if(may_digit.is_none())
            RETURN_ERR(Error::UnsupportedHexChar, chr);

        ret = static_cast<uint32_t>(ret << 4) | static_cast<uint32_t>(may_digit.unwrap());
    }

    return Ok(ret);
}

[[nodiscard]] static constexpr 
IResult<uint32_t> parse_dual_char(const char c1, const char c2){

    const auto d1 = ({
        const auto may_digit = char2digit(c1);
        if(may_digit.is_none()) return Err(Error::UnsupportedHexChar);
        may_digit.unwrap();
    });


    const auto d2 = ({
        const auto may_digit = char2digit(c2);
        if(may_digit.is_none()) return Err(Error::UnsupportedHexChar);
        may_digit.unwrap();
    });


    return Ok(static_cast<uint32_t>(d1 << 4 | d2));
}

static constexpr size_t STD_ID_STR_LEN = 3;
static constexpr size_t EXT_ID_STR_LEN = 8;
static constexpr size_t NUM_MAX_CAN_DLC = 8;

[[nodiscard]] static constexpr 
IResult<hal::CanStdId> parse_std_id(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);
    if(str.length() > STD_ID_STR_LEN)
        RETURN_ERR(Error::StdIdTooLong);
    if(str.length() < STD_ID_STR_LEN)
        RETURN_ERR(Error::StdIdTooShort);

    const auto either_id = parse_hex_str(str);
    
    if(either_id.is_err())
        RETURN_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > 0x7ff)
        RETURN_ERR(Error::StdIdOverflow);

    return Ok(hal::CanStdId::from_bits(id));
}


[[nodiscard]] static constexpr 
IResult<hal::CanExtId> parse_ext_id(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);
    if(str.length() > EXT_ID_STR_LEN)
        RETURN_ERR(Error::ExtIdTooLong);
    if(str.length() < EXT_ID_STR_LEN)
        RETURN_ERR(Error::ExtIdTooShort);


    const auto either_id = parse_hex_str(str);
    
    if(either_id.is_err())
        RETURN_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > ((1u << 29) - 1))
        RETURN_ERR(Error::ExtIdOverflow);

    return Ok(hal::CanExtId::from_bits(id));
}

[[nodiscard]] static constexpr 
IResult<std::array<uint8_t, NUM_MAX_CAN_DLC>> parse_payload(
    const StringView str, 
    const size_t dlc
){
    if(str.length() != dlc * 2) 
        RETURN_ERR(Error::PayloadLengthMismatch, str.length(), dlc, str);

    alignas(8) std::array<uint8_t, NUM_MAX_CAN_DLC> buf;

    #pragma GCC unroll 8
    for(size_t i = 0; i < dlc; i++){
        buf[i] = ({
            const auto res = parse_dual_char(
                str[i * 2], str[i * 2 + 1]
            );
            if(res.is_err()) RETURN_ERR(res.unwrap_err());
            res.unwrap();
        });
    }
    return Ok{buf};
}


[[nodiscard]] static constexpr 
IResult<size_t> parse_dlc(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);
    if(str.length() != 1) 
        RETURN_ERR(Error::PayloadLengthMismatch, str.length());

    const auto dlc = ({
        const auto res = parse_hex_str(str);
        
        if(res.is_err()) 
            RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    if(dlc > NUM_MAX_CAN_DLC)
        RETURN_ERR(Error::PayloadLengthOverflow, dlc);

    return Ok(dlc);
}

template<bool IS_EXTENDED>
[[nodiscard]] static constexpr 
IResult<uint32_t> parse_id_u32(const StringView str){
    if constexpr(IS_EXTENDED){
        return parse_ext_id(str).
            map([](const hal::CanExtId id) -> uint32_t{return id.to_u29();}); 
    } else {
        return parse_std_id(str).
            map([](const hal::CanStdId id) -> uint32_t{return id.to_u11();}); 
    }
};

template<bool IS_EXTENDED>
[[nodiscard]] static constexpr
IResult<hal::BxCanFrame> parse_msg(const StringView str, hal::CanRtr can_rtr){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);

    constexpr size_t ID_LEN = IS_EXTENDED ? 8 : 3;
    constexpr size_t DLC_LEN = 1;
    using ID = std::conditional_t<IS_EXTENDED, hal::CanExtId, hal::CanStdId>;

    if(str.length() < 4)
        RETURN_ERR(Error::ArgTooShort, str.length());

    auto provider = StrProvider{str};

    const uint32_t id_u32_checked = ({
        const auto res = parse_id_u32<IS_EXTENDED>(provider.fetch_leading(ID_LEN).unwrap());
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    const size_t dlc = ({
        const auto res = parse_dlc(provider.fetch_leading(DLC_LEN).unwrap());
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    const StringView payload_str = provider.fetch_remaining().unwrap();
    

    switch(can_rtr){
        case hal::CanRtr::Data:{
            const auto payload = ({
                const auto res = parse_payload(payload_str, dlc);
                if(res.is_err()) RETURN_ERR(res.unwrap_err());
                res.unwrap();
            });

            const auto bytes = std::span(payload.data(), dlc);
            
            return Ok(hal::BxCanFrame(
                ID::from_bits(id_u32_checked), 
                hal::BxCanPayload::from_bytes(bytes))
            );
        }
        case hal::CanRtr::Remote:{
            if(payload_str.length() != 0) 
                RETURN_ERR(Error::InvalidFieldInRemoteMsg);
            return Ok(hal::BxCanFrame::from_remote(ID::from_bits(id_u32_checked)));
        }
    }
    __builtin_unreachable();
}




[[nodiscard]] static constexpr 
hal::CanBaudrate map_chr_to_baud(char chr){ 
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

auto map_msg_to_operation = [](const hal::BxCanFrame & frame) { 
    return Operation(SendCanFrame{frame}); };

IResult<Operation> SlcanParser::process_line(const StringView str) const {
    static constexpr bool IS_EXTENDED = true;
    const StringView line = str.trim();
    // DEBUG_PRINTLN(line, line.length());
    if(line.size() == 0){
        RETURN_ERR(Error::NoInput);
    }
    
    if(line.size() == 1){
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
        if(cmd_line.size() == 0) [[unlikely]]
            RETURN_ERR(Error::NoArg);
        switch(line[0]){
            case 'S': {
                if(cmd_line.size() == 0) 
                    RETURN_ERR(Error::NoArg);
                if(cmd_line.size() > 1) 
                    RETURN_ERR(Error::ArgTooLong);
                const auto chr = cmd_line[0];
                if(chr < '0' || chr > '8') 
                    return Err(Error::InvalidCanBaudrate);

                return Ok(Operation(SetCanBaud{.baudrate = map_chr_to_baud(chr)}));
            }
            case 's':{
                //set serial baudrate
                TODO();
                return Ok(Operation(SetSerialBaud{.baudrate = 0}));
            }

            case 't': return parse_msg<not IS_EXTENDED>(cmd_line, hal::CanRtr::Data)
                .map(map_msg_to_operation);
            case 'r': return parse_msg<not IS_EXTENDED>(cmd_line, hal::CanRtr::Remote)
                .map(map_msg_to_operation);

            case 'T': return parse_msg<IS_EXTENDED>(cmd_line, hal::CanRtr::Data)
                .map(map_msg_to_operation);
            case 'R': return parse_msg<IS_EXTENDED>(cmd_line, hal::CanRtr::Remote)
                .map(map_msg_to_operation);

        }
        RETURN_ERR(Error::UnknownCommand);
    }

    __builtin_unreachable();
}

SendText SlcanParser::response_version() const{
    return SendText::from_str("V1013\r");
}

SendText SlcanParser::response_serial_idx() const{
    return SendText::from_str("NA123\r");
}

Flags SlcanParser::get_flag() const {
    return Flags::RxFifoFull;
}

SendText SlcanParser::response_flag() const{
    const auto flags = get_flag();
    const char str[2] = {
        static_cast<char>(std::bit_cast<char>(flags) + '0'), 'r'
    };

    return SendText::from_str(StringView(str, 2));
}

namespace unit_test{
static_assert(parse_payload("11x", 1).unwrap_err() == Error::PayloadLengthMismatch);
static_assert(parse_payload("11", 1).unwrap()[0] == 0x11);
static_assert(parse_payload("1122334455667788", 8).unwrap()[7] == 0x88);
static_assert(parse_std_id("123").unwrap().to_u11() == 0x123);
static_assert(parse_std_id("923").unwrap_err() == Error::StdIdOverflow);
static_assert(parse_std_id("9scd").unwrap_err() == Error::StdIdTooLong);
static_assert(parse_std_id("9sc").unwrap_err() == Error::UnsupportedHexChar);

// Additional tests for hex conversion functions
static_assert(char2digit('0').unwrap() == 0);
static_assert(char2digit('9').unwrap() == 9);
static_assert(char2digit('a').unwrap() == 10);
static_assert(char2digit('f').unwrap() == 15);
static_assert(char2digit('A').unwrap() == 10);
static_assert(char2digit('F').unwrap() == 15);
static_assert(char2digit('g').is_none());
static_assert(char2digit('G').is_none());
static_assert(char2digit('-').is_none());

static_assert(digit2char(0) == '0');
static_assert(digit2char(9) == '9');
static_assert(digit2char(10) == 'a');
static_assert(digit2char(15) == 'f');
static_assert(digit2char(16) == 0); // Invalid input

// Additional tests for parse_hex_str
static_assert(parse_hex_str("").unwrap_err() == Error::NoArg);
static_assert(parse_hex_str("12").unwrap() == 0x12);
static_assert(parse_hex_str("AB").unwrap() == 0xAB);
static_assert(parse_hex_str("ff").unwrap() == 0xFF);
static_assert(parse_hex_str("xyz").unwrap_err() == Error::UnsupportedHexChar);
static_assert(parse_dual_char('1', '0').unwrap() == 0x10);
static_assert(parse_dual_char('f', 'f').unwrap() == 0xFF);


// Additional tests for extended ID parsing
static_assert(parse_ext_id("00000000").unwrap().to_u29() == 0);
static_assert(parse_ext_id("1FFFFFFF").unwrap().to_u29() == 0x1FFFFFFF);
static_assert(parse_ext_id("20000000").unwrap_err() == Error::ExtIdOverflow);
static_assert(parse_ext_id("1234567").unwrap_err() == Error::ExtIdTooShort);
static_assert(parse_ext_id("123456789").unwrap_err() == Error::ExtIdTooLong);
static_assert(parse_ext_id("1234567G").unwrap_err() == Error::UnsupportedHexChar);

// Additional tests for length parsing
static_assert(parse_dlc("0").unwrap() == 0);
static_assert(parse_dlc("8").unwrap() == 8);
static_assert(parse_dlc("9").unwrap_err() == Error::PayloadLengthOverflow);
static_assert(parse_dlc("").unwrap_err() == Error::NoArg);
static_assert(parse_dlc("12").unwrap_err() == Error::PayloadLengthMismatch);

// Tests for parse_id_u32
static_assert(parse_id_u32<false>("000").unwrap() == 0);
static_assert(parse_id_u32<false>("7FF").unwrap() == 0x7FF);
static_assert(parse_id_u32<true>("00000000").unwrap() == 0);
static_assert(parse_id_u32<true>("1FFFFFFF").unwrap() == 0x1FFFFFFF);

// Tests for baudrate mapping
static_assert(map_chr_to_baud('0').has_same_freq(hal::CanBaudrate::_10K));
static_assert(map_chr_to_baud('8').has_same_freq(hal::CanBaudrate::_1M));

// Test edge cases for payload parsing
static_assert(parse_payload("", 0).unwrap()[0] == 0); // Empty payload
static_assert(parse_payload("00", 1).unwrap()[0] == 0x00); // Zero byte
static_assert(parse_payload("ff", 1).unwrap()[0] == 0xff); // Max byte
}