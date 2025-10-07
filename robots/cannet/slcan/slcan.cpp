#include "slcan.hpp"

using namespace ymd;
using namespace ymd::robots::slcan;
using namespace ymd::robots::asciican;

// #define SLCAN_DEBUG_EN 1
#define SLCAN_DEBUG_EN 0

#define SLCAN_STATIC_TEST_EN 1

#if SLCAN_DEBUG_EN
#define constexpr
#define RET_ERR(e, ...) {DEBUG_PRINTLN(__VA_ARGS__); return Err(e);}
#else
#define RET_ERR(e, ...) {return Err(e);}
#endif

using Error = Slcan::Error;



template<typename T = void>
using IResult = Result<T, Error>;

static constexpr Option<uint8_t> hex2digit(char c){
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

static constexpr char digit2hex(const uint8_t digit){
    switch(digit){
        case 0 ... 9:
            return digit + '0';
        case 10 ... 15:
            return digit - 10 + 'a';
        default:
            return 0;
    }
}


static constexpr IResult<uint32_t> parse_hex(const StringView str){
    if(str.length() == 0)
        RET_ERR(Error::NoArg);

    uint32_t ret = 0;

    for(size_t i = 0; i < str.length(); i++){
        const auto chr = str[i];
        const auto may_digit = hex2digit(chr);

        if(may_digit.is_none())
            RET_ERR(Error::UnsupportedCharInHex, chr);

        ret = ret << 4 | may_digit.unwrap();
    }

    return Ok(ret);
}

static constexpr IResult<uint32_t> parse_hex(const char c1, const char c2){

    const auto d1 = ({
        const auto may_digit = hex2digit(c1);
        if(may_digit.is_none()) return Err(Error::UnsupportedCharInHex);
        may_digit.unwrap();
    });


    const auto d2 = ({
        const auto may_digit = hex2digit(c2);
        if(may_digit.is_none()) return Err(Error::UnsupportedCharInHex);
        may_digit.unwrap();
    });


    return Ok(static_cast<uint32_t>(d1 << 4 | d2));
}

static constexpr IResult<hal::CanStdId> parse_std_id(const StringView str){
    if(str.length() == 0)
        RET_ERR(Error::NoArg);
    if(str.length() > 3)
        RET_ERR(Error::StdIdTooLong);
    if(str.length() < 3)
        RET_ERR(Error::StdIdTooShort);

    const auto either_id = parse_hex(str);
    
    if(either_id.is_err())
        RET_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > 0x7ff)
        RET_ERR(Error::InvalidStdId);

    return Ok(hal::CanStdId(id));
}

#if SLCAN_STATIC_TEST_EN == 1
static_assert(parse_std_id("123").unwrap().to_u11() == 0x123);
static_assert(parse_std_id("923").unwrap_err() == Error::InvalidStdId);
static_assert(parse_std_id("9scd").unwrap_err() == Error::StdIdTooLong);
static_assert(parse_std_id("9sc").unwrap_err() == Error::UnsupportedCharInHex);
#endif

[[maybe_unused]]
static constexpr IResult<hal::CanExtId> parse_ext_id(const StringView str){
    if(str.length() == 0) 
        RET_ERR(Error::NoArg);
    if(str.length() > 8)
        RET_ERR(Error::ExtIdTooLong);
    if(str.length() < 8)
        RET_ERR(Error::ExtIdTooShort);


    const auto either_id = parse_hex(str);
    
    if(either_id.is_err())
        RET_ERR(either_id.unwrap_err());

    const auto id = either_id.unwrap();

    if(id > ((1u << 29) - 1))
        RET_ERR(Error::InvalidExtId);

    return Ok(hal::CanExtId(id));
}

static constexpr IResult<std::array<uint8_t, 8>> parse_payload(
    const StringView str, 
    const size_t dlc
){
    if(str.size() != dlc * 2) 
        RET_ERR(Error::InvalidPayloadLength, str.size(), dlc, str);

    std::array<uint8_t, 8> buf;

    for(size_t i = 0; i < dlc; i++){
        const auto either_data8 = parse_hex(
            str[i * 2], str[i * 2 + 1]
        );
        if(either_data8.is_err())
            RET_ERR(either_data8.unwrap_err());

        buf[i] = either_data8.unwrap();
    }

    return Ok{buf};
}


#if SLCAN_STATIC_TEST_EN == 0
static_assert(parse_payload("11x", 1).unwrap_err() == Error::InvalidPayloadLength);
static_assert(parse_payload("11", 1).unwrap()[0] == 0x11);
static_assert(parse_payload("1122334455667788", 8).unwrap()[7] == 0x88);
#endif

static constexpr IResult<size_t> parse_len(const StringView str){
    if(str.length() == 0) 
        RET_ERR(Error::NoArg);
    if(str.length() != 1) 
        RET_ERR(Error::InvalidPayloadLength, str.length());

    const auto either_len = parse_hex(str);

    if(either_len.is_err()) 
        RET_ERR(either_len.unwrap_err());

    const auto len = either_len.unwrap();
    if(len > 8) RET_ERR(Error::InvalidPayloadLength, len);
    return Ok(len);
}

template<bool is_ext>
static constexpr auto parse_id_as_u32(const StringView str) -> IResult<uint32_t>{
    if constexpr(is_ext){
        return parse_ext_id(str).
            map([](const hal::CanExtId id) -> uint32_t{return id.to_u29();}); 
    } else {
        return parse_std_id(str).
            map([](const hal::CanStdId id) -> uint32_t{return id.to_u11();}); 
    }
};

template<bool is_ext>
static constexpr IResult<hal::CanMsg> parse_msg(const StringView str, bool is_rmt){
    if(str.size() == 0) 
        RET_ERR(Error::NoArg);

    constexpr size_t ID_LEN = is_ext ? 8 : 3;
    constexpr size_t DLC_LEN = 1;

    if(str.length() < 4)
        RET_ERR(Error::ArgTooShort, str.length());

    auto cutter = StringCutter{str};

    const uint32_t id_u32_checked = ({
        const auto res = parse_id_as_u32<is_ext>(cutter.fetch_next(ID_LEN).unwrap());
        if(res.is_err()) RET_ERR(res.unwrap_err());
        res.unwrap();
    });

    const size_t dlc = ({
        const auto res = parse_len(cutter.fetch_next(DLC_LEN).unwrap());
        if(res.is_err()) RET_ERR(res.unwrap_err());
        res.unwrap();
    });

    const StringView payload_str = cutter.fetch_remaining().unwrap();
    
    using ID = std::conditional_t<is_ext, hal::CanExtId, hal::CanStdId>;
    if(not is_rmt){
        const auto payload = ({
            const auto res = parse_payload(payload_str, dlc);
            if(res.is_err()) RET_ERR(res.unwrap_err());
            res.unwrap();
        });

        const auto bytes = std::span(payload.data(), dlc);
        
        return Ok(hal::CanMsg::from_bytes(ID(id_u32_checked), bytes));

    }else{
        if(payload_str.size()) 
            RET_ERR(Error::InvalidFieldInRemoteMsg);
        return Ok(hal::CanMsg::from_remote(ID(id_u32_checked)));
    }
}

// static_assert(int(parse_msg("0C34567F20102", true).unwrap_err()) == 0);
// static constexpr auto l = parse_msg("0A70", true).unwrap().size();


#if 0
[[maybe_unused]] static void static_test(){

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

IResult<> Slcan::on_recv_str(const StringView str){
    static constexpr bool REMOTE = true;

    if(str.size() == 0){
        RET_ERR(Error::NoInput);
    }else if(str.size() == 1){
        switch(str[0]){
            default: RET_ERR(Error::InvalidCommand);

            case 'F': return response_flag();
            case 'V': return response_version();
            case 'N': return response_serial_idx();
        }
    }else{
        const auto cmd_str = str.substr(1).unwrap();

        switch(str[0]){
            default:RET_ERR(Error::InvalidCommand);

            case 'S': return on_recv_set_baud(cmd_str);

            case 't': return on_recv_send_std_msg(cmd_str, not REMOTE);
            case 'r': return on_recv_send_std_msg(cmd_str, REMOTE);

            case 'T': return on_recv_send_ext_msg(cmd_str, not REMOTE);
            case 'R': return on_recv_send_ext_msg(cmd_str, REMOTE);

            case 'O': return on_recv_open(cmd_str);
            case 'C': return on_recv_close(cmd_str);


            case 'Z': RET_ERR(Error::NotImplemented);
        }
    }

    __builtin_unreachable();
}

IResult<> Slcan::response_version(){
    return phy_.send_str("V1013\r");
}

IResult<> Slcan::response_serial_idx(){
    return phy_.send_str("NA123\r");
}

Slcan::Flags Slcan::get_flag() const {
    return Flags::RxFifoFull;
}

IResult<> Slcan::response_flag(){
    const char str[2] = {
        char(get_flag()), 'r'
    };

    return phy_.send_str(StringView(str, 2));
}

IResult<> Slcan::on_recv_set_baud(const StringView str){
    if(str.size() == 0) 
        RET_ERR(Error::NoArg);
    if(str.size() > 1) 
        RET_ERR(Error::ArgTooLong);

    switch(str[0]){
        default:RET_ERR(Error::InvalidBaud);
        case '0': return phy_.set_can_baud(10_KHz);
        case '1': return phy_.set_can_baud(20_KHz);
        case '2': return phy_.set_can_baud(50_KHz);
        case '3': return phy_.set_can_baud(100_KHz);
        case '4': return phy_.set_can_baud(125_KHz);
        case '5': return phy_.set_can_baud(250_KHz);
        case '6': return phy_.set_can_baud(500_KHz);
        case '7': return phy_.set_can_baud(800_KHz);
        case '8': return phy_.set_can_baud(1000_KHz);
    }
}


IResult<> Slcan::on_recv_send_std_msg(const StringView str, const bool is_rmt){
    // riiil\r, 发送标准远程帧, 如 r1000\r 表示 发送 0x100 的远程帧, 返回 z\r 表示 OK, \b表示 ERROR

    const auto res = parse_msg<false>(str, is_rmt);
    if(res.is_err()) RET_ERR(res.unwrap_err());
    return phy_.send_can_msg(res.unwrap());
}

IResult<> Slcan::on_recv_send_ext_msg(const StringView str, const bool is_rmt){
    const auto res = parse_msg<true>(str, is_rmt);
    if(res.is_err()) RET_ERR(res.unwrap_err());
    return phy_.send_can_msg(res.unwrap());
}
IResult<> Slcan::on_recv_open(const StringView str){ 
    return phy_.open();
}

IResult<> Slcan::on_recv_close(const StringView str){ 
    return phy_.close();
}
