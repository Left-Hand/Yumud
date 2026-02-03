#include "slcan.hpp"
#include "core/utils/scope_guard.hpp"

using namespace ymd;
using namespace ymd::slcan;
using namespace ymd::asciican;
using namespace asciican::primitive::operations;

// #define SLCAN_DEBUG_EN 1
#define SLCAN_DEBUG_EN 0


#if(SLCAN_DEBUG_EN == 1)
#define constexpr
#define RETURN_ERR(e, ...) {DEBUG_PRINTLN(__VA_ARGS__); return Err(e);}
#else
#define RETURN_ERR(e, ...) ({return Err(e);})
#endif

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr size_t STD_ID_STR_LEN = 3;
static constexpr size_t EXT_ID_STR_LEN = 8;
static constexpr size_t NUM_MAX_CAN_DLC = 8;


[[nodiscard]] static constexpr 
Option<uint8_t> _char2digit(char c){
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
IResult<uint32_t> _parse_hex_str(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);

    uint32_t ret = 0;

    #pragma GCC unroll 8
    for(size_t i = 0; i < str.length(); i++){
        const auto chr = str[i];
        const auto may_digit = _char2digit(chr);

        if(may_digit.is_none())
            RETURN_ERR(Error::InvalidCharInHex, chr);

        ret = static_cast<uint32_t>(ret << 4) | static_cast<uint32_t>(may_digit.unwrap());
    }

    return Ok(ret);
}

[[nodiscard]] static constexpr 
IResult<uint32_t> _parse_dual_char(const char c1, const char c2){

    const auto nibble1 = ({
        const auto may_digit = _char2digit(c1);
        if(may_digit.is_none()) return Err(Error::InvalidCharInHex);
        may_digit.unwrap();
    });


    const auto nibble2 = ({
        const auto may_digit = _char2digit(c2);
        if(may_digit.is_none()) return Err(Error::InvalidCharInHex);
        may_digit.unwrap();
    });


    return Ok(static_cast<uint32_t>(nibble1 << 4 | nibble2));
}


[[nodiscard]] static constexpr 
IResult<hal::CanStdId> parse_std_id(const StringView str){
    if(str.length() == 0)
        RETURN_ERR(Error::NoArg);

    if(str.length() > STD_ID_STR_LEN)
        RETURN_ERR(Error::StdIdTooLong);
    if(str.length() < STD_ID_STR_LEN)
        RETURN_ERR(Error::StdIdTooShort);

    const auto either_id = _parse_hex_str(str);
    
    if(either_id.is_err())
        RETURN_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > 0x7ff)
        RETURN_ERR(Error::StdIdOverflow);

    return Ok(hal::CanStdId::from_bits(id));
}


[[nodiscard]] static constexpr 
IResult<hal::CanExtId> _parse_ext_id(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);

    if(str.length() > EXT_ID_STR_LEN)
        RETURN_ERR(Error::ExtIdTooLong);
    if(str.length() < EXT_ID_STR_LEN)
        RETURN_ERR(Error::ExtIdTooShort);


    const auto either_id = _parse_hex_str(str);
    
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
    if((str.length() & 1) != 0) 
        RETURN_ERR(Error::OddPayloadLength, str.length(), dlc, str);
    if(str.length() != dlc * 2) 
        RETURN_ERR(Error::PayloadLengthMismatch, str.length(), dlc, str);

    alignas(8) std::array<uint8_t, NUM_MAX_CAN_DLC> buf;

    #pragma GCC unroll 8
    for(size_t i = 0; i < dlc; i++){
        buf[i] = ({
            const auto res = _parse_dual_char(
                str[i * 2], str[i * 2 + 1]
            );
            if(res.is_err()) RETURN_ERR(res.unwrap_err());
            res.unwrap();
        });
    }

    #pragma GCC unroll 8
    for(size_t i = dlc; i < NUM_MAX_CAN_DLC; i++){
        buf[i] = 0;
    }

    return Ok{buf};
}


[[nodiscard]] static constexpr 
IResult<size_t> _parse_dlc(const StringView str){
    if(str.length() == 0) 
        RETURN_ERR(Error::NoArg);
    if(str.length() != 1) 
        RETURN_ERR(Error::InvalidDlcFormat, str.length());

    const auto dlc = ({
        const auto res = _parse_hex_str(str);
        
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
IResult<uint32_t> _parse_id_u32(const StringView str){
    if constexpr(IS_EXTENDED){
        return _parse_ext_id(str).
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

    // auto provider = StrProvider{str};
    size_t offset = 0;


    const uint32_t id_u32_checked = ({
        const auto res = _parse_id_u32<IS_EXTENDED>(StringView(str.data() + offset, ID_LEN));
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });
    offset += ID_LEN;

    const size_t dlc = ({
        const auto res = _parse_dlc(StringView(str.data() + offset, DLC_LEN));
        if(res.is_err()) RETURN_ERR(res.unwrap_err());
        res.unwrap();
    });

    offset += DLC_LEN;
    const StringView payload_str = StringView(str.data() + offset, str.length() - offset);

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
                RETURN_ERR(Error::PayloadFoundedInRemote);
            return Ok(hal::BxCanFrame::from_remote(ID::from_bits(id_u32_checked)));
        }
    }
    //unreachable
    __builtin_trap();
}


[[nodiscard]] static constexpr 
uint32_t _chr_to_baud_unchecked(char chr){
    switch(chr){
        case '0': return 10000;
        case '1': return 20000;
        case '2': return 50000;
        case '3': return 100000;
        case '4': return 125000;
        case '5': return 250000;
        case '6': return 500000;
        case '7': return 800000;
        case '8': return 1000000;
    }
    __builtin_trap();
}

auto _msg_to_operation = [](const hal::BxCanFrame & frame) { 
    return Operation(SendCanFrame{frame}); };


IResult<Operation> SlcanParser::process_line(const StringView input_line) const {
    static constexpr bool IS_EXTENDED = true;
    const StringView line = input_line.trim();
    if(line.size() == 0) RETURN_ERR(Error::NoInput);
    if(line.size() == 1){
        switch(line[0]){
            case 'F': return Ok(Operation(response_flag()));
            case 'V': return Ok(Operation(response_version()));
            case 'N': return Ok(Operation(response_serial_number()));
            
            case 'O': return Ok(Operation(Open{}));
            case 'C': return Ok(Operation(Close{}));

            case 'S':
            case 's': 
            case 't': 
            case 'r': 
            case 'T': 
            case 'R': 
            case 'M': 
            case 'm': 
            case 'Z': 
                RETURN_ERR(Error::NoArg);
        }
        RETURN_ERR(Error::InvalidCommand);
    }else{
        const auto cmd_line = line.substr(1).unwrap();
        if(cmd_line.size() == 0) [[unlikely]]
            RETURN_ERR(Error::NoArg);
        switch(line[0]){
            // Setup with standard CAN bit-rates where n is 0-8.
            // This command is only active if the CAN channel is closed.
            // S0 Setup 10Kbit
            // S1 Setup 20Kbit
            // S2 Setup 50Kbit
            // S3 Setup 100Kbit
            // S4 Setup 125Kbit
            // S5 Setup 250Kbit
            // S6 Setup 500Kbit
            // S7 Setup 800Kbit
            // S8 Setup 1Mbit
            case 'S': {
                if(cmd_line.size() == 0) 
                    RETURN_ERR(Error::NoArg);
                if(cmd_line.size() > 1) 
                    RETURN_ERR(Error::ArgTooLong);
                const auto chr = cmd_line[0];
                if(chr < '0' || chr > '8') 
                    return Err(Error::InvalidCanBaudrate);

                return Ok(Operation(SetCanBaud{.baudrate = _chr_to_baud_unchecked(chr)}));
            }

            // Setup with BTR0/BTR1 CAN bit-rates where xx and yy is a hex
            // value. This command is only active if the CAN channel is closed.
            case 's':{
                RETURN_ERR(Error::WillNeverSupport);
            }

            case 't': return parse_msg<not IS_EXTENDED>(cmd_line, hal::CanRtr::Data)
                .map(_msg_to_operation);

            case 'T': return parse_msg<IS_EXTENDED>(cmd_line, hal::CanRtr::Data)
                .map(_msg_to_operation);
                
            case 'r': return parse_msg<not IS_EXTENDED>(cmd_line, hal::CanRtr::Remote)
                .map(_msg_to_operation);


            case 'R': return parse_msg<IS_EXTENDED>(cmd_line, hal::CanRtr::Remote)
                .map(_msg_to_operation);

            // Sets Acceptance Code Register (ACn Register of SJA1000).
            // This command is only active if the CAN channel is initiated and
            // not opened.
            case 'M': RETURN_ERR(Error::WillNeverSupport);

            // Sets Acceptance Mask Register (AMn Register of SJA1000).
            // This command is only active if the CAN channel is initiated and
            // not opened.
            case 'm': RETURN_ERR(Error::WillNeverSupport);

            // Sets Time Stamp ON/OFF for received frames only.
            // This command is only active if the CAN channel is closed.
            // The value will be saved in EEPROM and remembered next time
            // the CANUSB is powered up. This command shouldn’t be used more
            // than when you want to change this behaviour. It is set to OFF by
            // default, to be compatible with old programs written for CANUSB.
            // Setting it to ON, will add 4 bytes sent out from CANUSB with the A
            // and P command or when the Auto Poll/Send feature is enabled.
            // When using Time Stamp each message gets a time in milliseconds
            // when it was received into the CANUSB, this can be used for real
            // time applications for e.g. knowing time inbetween messages etc.
            // Note however by using this feature you maybe will decrease
            // bandwith on the CANUSB, since it adds 4 bytes to each message
            // being sent (specially on VCP drivers).
            // If the Time Stamp is OFF, the incomming frames looks like this:
            // t10021133[CR] (a standard frame with ID=0x100 & 2 bytes)
            // If the Time Stamp is ON, the incomming frames looks like this:
            // t100211334D67[CR] (a standard frame with ID=0x100 & 2 bytes)
            // Note the last 4 bytes 0x4D67, which is a Time Stamp for this
            // specific message in milliseconds (and of course in hex). The timer
            // in the CANUSB starts at zero 0x0000 and goes up to 0xEA5F before
            // it loop arround and get’s back to 0x0000. This corresponds to exact
            // 60,000mS (i.e. 1 minute which will be more than enough in most
            // systems)
            case 'Z': RETURN_ERR(Error::WillNeverSupport);

        }
        RETURN_ERR(Error::InvalidCommand);
    }

    //unreachable
    __builtin_trap();
}

SendString SlcanParser::response_version() const{
    return SendString::from_str("V1013\r");
}

SendString SlcanParser::response_serial_number() const{
    return SendString::from_str("NA123\r");
}

StatusFlag SlcanParser::get_flag() const {
    return StatusFlag::zero();
}

SendString SlcanParser::response_flag() const{
    static constexpr size_t LEN = 4;
    std::array<char, LEN> chars;
    const auto flag = get_flag();
    chars[0] = 'F';
    std::tie(chars[1], chars[2]) = flag.to_nibbles();
    chars[3] = '\r';

    return SendString::from_str(StringView(chars.data(), LEN));
}

namespace{
static_assert(parse_payload("11x", 1).unwrap_err() == Error::OddPayloadLength);
static_assert(parse_payload("11", 1).unwrap()[0] == 0x11);
static_assert(parse_payload("1122334455667788", 8).unwrap()[7] == 0x88);
static_assert(parse_std_id("123").unwrap().to_u11() == 0x123);
static_assert(parse_std_id("923").unwrap_err() == Error::StdIdOverflow);
static_assert(parse_std_id("9scd").unwrap_err() == Error::StdIdTooLong);
static_assert(parse_std_id("9sc").unwrap_err() == Error::InvalidCharInHex);

// Additional tests for hex conversion functions
static_assert(_char2digit('0').unwrap() == 0);
static_assert(_char2digit('9').unwrap() == 9);
static_assert(_char2digit('a').unwrap() == 10);
static_assert(_char2digit('f').unwrap() == 15);
static_assert(_char2digit('A').unwrap() == 10);
static_assert(_char2digit('F').unwrap() == 15);
static_assert(_char2digit('g').is_none());
static_assert(_char2digit('G').is_none());
static_assert(_char2digit('-').is_none());

// Additional tests for parse_hex_str
static_assert(_parse_hex_str("").unwrap_err() == Error::NoArg);
static_assert(_parse_hex_str("12").unwrap() == 0x12);
static_assert(_parse_hex_str("AB").unwrap() == 0xAB);
static_assert(_parse_hex_str("ff").unwrap() == 0xFF);
static_assert(_parse_hex_str("xyz").unwrap_err() == Error::InvalidCharInHex);
static_assert(_parse_dual_char('1', '0').unwrap() == 0x10);
static_assert(_parse_dual_char('f', 'f').unwrap() == 0xFF);


// Additional tests for extended ID parsing
static_assert(_parse_ext_id("00000000").unwrap().to_u29() == 0);
static_assert(_parse_ext_id("1FFFFFFF").unwrap().to_u29() == 0x1FFFFFFF);
static_assert(_parse_ext_id("20000000").unwrap_err() == Error::ExtIdOverflow);
static_assert(_parse_ext_id("1234567").unwrap_err() == Error::ExtIdTooShort);
static_assert(_parse_ext_id("123456789").unwrap_err() == Error::ExtIdTooLong);
static_assert(_parse_ext_id("1234567G").unwrap_err() == Error::InvalidCharInHex);

// Additional tests for length parsing
static_assert(_parse_dlc("0").unwrap() == 0);
static_assert(_parse_dlc("8").unwrap() == 8);
static_assert(_parse_dlc("9").unwrap_err() == Error::PayloadLengthOverflow);
static_assert(_parse_dlc("").unwrap_err() == Error::NoArg);
static_assert(_parse_dlc("12").unwrap_err() == Error::InvalidDlcFormat);

// Tests for parse_id_u32
static_assert(_parse_id_u32<false>("000").unwrap() == 0);
static_assert(_parse_id_u32<false>("7FF").unwrap() == 0x7FF);
static_assert(_parse_id_u32<true>("00000000").unwrap() == 0);
static_assert(_parse_id_u32<true>("1FFFFFFF").unwrap() == 0x1FFFFFFF);


// Test edge cases for payload parsing
static_assert(parse_payload("", 0).unwrap()[0] == 0); // Empty payload
static_assert(parse_payload("00", 1).unwrap()[0] == 0x00); // Zero byte
static_assert(parse_payload("ff", 1).unwrap()[0] == 0xff); // Max byte
}