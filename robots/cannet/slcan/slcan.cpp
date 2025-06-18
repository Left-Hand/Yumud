#include "slcan.hpp"

using namespace ymd;
using namespace ymd::robots::asciican;

using Error = Slcan::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr Option<uint8_t> hex2digit(char c){
    switch(c){
        case '0' ... '9':
            return Some(c - '0');
        case 'a' ... 'f':
            return Some(c - 'a');
        case 'A' ... 'F':
            return Some(c - 'A');
        default:
            return None;
    }
}


static constexpr IResult<uint32_t> parse_hex(const StringView str){
    if(str.length() == 0)
        return Err(Error::NoArg);

    uint32_t ret = 0;

    for(size_t i = 0; i < str.length(); i++){
        const auto chr = str[i];
        const auto may_digit = hex2digit(chr);

        if(may_digit.is_none())
            return Err(Error::UnsupportedCharInHex);

        ret = ret << 4 | may_digit.unwrap();
    }

    return Ok(ret);
}

static_assert(parse_hex("1234").unwrap() == 0x1234);

static constexpr IResult<hal::CanStdId> parse_std_id(const StringView str){
    if(str.length() == 0)
        return Err(Error::NoArg);
    if(str.length() > 3)
        return Err(Error::ArgTooLong);
    if(str.length() < 3)
        return Err(Error::ArgTooShort);

    const auto either_id = parse_hex(str);
    
    if(either_id.is_err())
        return Err(either_id.unwrap_err());

    const auto id = either_id.unwrap();
    if(id > 0x7ff)
        return Err(Error::InvalidStdId);
    return Ok(hal::CanStdId(id));
}

static constexpr IResult<hal::CanExtId> parse_ext_id(const StringView str){
    //TODO NOT IMPLEMENTED
    if(str.length() == 0) 
        return Err(Error::NoArg);
    if(str.length() > 8)
        return Err(Error::ArgTooLong);
    if(str.length() < 8)
        return Err(Error::ArgTooShort);


    const size_t id = size_t(str);
    if(id > ((1 << 29) - 1)) return Err(Error::InvalidExtId);
    return Ok(hal::CanExtId(id));
}

static constexpr IResult<std::array<uint8_t, 8>> parse_payload(
    const StringView str, 
    const uint8_t dlc
){
    if(str.size() != dlc * 2) 
        return Err(Error::InvalidPayloadLength);

    std::array<uint8_t, 8> buf;

    for(size_t i = 0; i < dlc; i++){
        const auto either_data8 = parse_hex(str.substr_by_len(i * 2, 2));
        if(either_data8.is_err())
            return Err(either_data8.unwrap_err());

        buf[i] = either_data8.unwrap();
    }

    return Ok{buf};
}

static constexpr IResult<size_t> parse_len(const StringView str){
    if(str.length() == 0) 
        return Err(Error::NoArg);
    if(str.length() != 1) 
        return Err(Error::InvalidPayloadLength);

    const auto either_len = parse_hex(str);

    if(either_len.is_err()) 
        return Err(either_len.unwrap_err());

    const auto len = either_len.unwrap();
    if(len > 8) return Err(Error::InvalidPayloadLength);
    return Ok(len);
}

IResult<> Slcan::on_recv_string(const StringView str){
    if(str.size() == 0) 
        return Err(Error::NoInput);

    const auto cmd_str = str.substr(1);

    switch(str[0]){
        default:return Err(Error::InvalidCommand);

        case 'S': return on_recv_set_baud(cmd_str);

        case 't': return on_recv_send_std_msg(cmd_str, false);
        case 'r': return on_recv_send_std_msg(cmd_str, true);

        case 'T': return on_recv_send_ext_msg(cmd_str, false);
        case 'R': return on_recv_send_ext_msg(cmd_str, true);

        case 'F': return response_flag();
        case 'O': return on_recv_open(cmd_str);
        case 'C': return on_recv_close(cmd_str);

        case 'V': return reponse_version();
        case 'N': return reponse_serial_idx();

        case 'Z': return Err(Error::NotImplemented);
    }
    return Ok();
}

IResult<> Slcan::reponse_version(){
    return phy_.send_str("V1013\r");
}

IResult<> Slcan::reponse_serial_idx(){
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
        return Err(Error::NoArg);
    if(str.size() > 1) 
        return Err(Error::ArgTooLong);

    switch(str[0]){
        default:return Err(Error::InvalidBaud);
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

    if(str.size() == 0) 
        return Err(Error::NoArg);
    if(str.size() < 4) 
        return Err(Error::ArgTooShort);
    if(str.size() > 4)
        return Err(Error::ArgTooLong);

    static constexpr size_t ID_LEN = 3;
    static constexpr size_t DLC_LEN = 1;

    auto cutter = StringCutter{str};

    const auto id = ({
        const auto res = parse_std_id(cutter.fetch_next(ID_LEN));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const auto dlc = ({
        const auto res = parse_len(cutter.fetch_next(DLC_LEN));
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    const StringView payload_str = cutter.fetch_remaining();
    
    if(is_rmt){
        const auto payload = ({
            const auto res = parse_payload(payload_str, dlc);
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });
        return phy_.send_can_msg(
            Msg::from_bytes(StdId(id), std::span(payload))
        );
    }else{
        if(payload_str.size()) 
            return Err(Error::DataExistsInRemote);
        return phy_.send_can_msg(
            Msg::from_remote(StdId(id))
        );
    }
}

// Riiiiiiiil\r, 发送扩展远程帧, 如 r1234567F2 表示发送 0x1234567F 且 DLC=2 的 远程帧
IResult<> Slcan::on_recv_send_ext_msg(const StringView str, const bool is_rmt){
    return Ok();
}
IResult<> Slcan::on_recv_open(const StringView str){ 
    return phy_.open();
}

IResult<> Slcan::on_recv_close(const StringView str){ 
    return phy_.close();
}

