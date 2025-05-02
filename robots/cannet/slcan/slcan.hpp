#pragma once

// https://blog.csdn.net/weifengdq/article/details/128823317

#include "../asciican_utils.hpp"


namespace ymd::robots::asciican{


class Slcan final{
public:
    using Msg = AsciiCanPhy::Msg;
    using Error = AsciiCanPhy::Error;
    using Flags = AsciiCanPhy::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;

    Slcan(AsciiCanPhy & phy): phy_(phy){;}
private:
    //不以\r结尾
    IResult<> handle_recv_string(const StringView str){
        if(str.size() < 1) return Err(Error::NoInput);
        const auto cmd_str = str.substr(1, str.size());
        switch(str[0]){
            default:return Err(Error::InvalidCommand);
            case 'S':return handle_set_baud(cmd_str);

            case 't':return handle_send_std_msg(cmd_str, false);
            case 'r':return handle_send_std_msg(cmd_str, true);

            case 'T':return  handle_send_ext_msg(cmd_str, false);
            case 'R':return  handle_send_ext_msg(cmd_str, true);

            case 'F':response_flag();
            case 'O':return handle_open(cmd_str);
            case 'C':return handle_close(cmd_str);

            case 'V':reponse_version();
            case 'N':reponse_serial_idx();

            case 'Z':return Err(Error::NotSupportedYet);
        }
        return Ok();
    }

    IResult<> reponse_version(){
        return phy_.send_str("V1013\r");
    }
    
    IResult<> reponse_serial_idx(){
        return phy_.send_str("NA123\r");
    }

    Flags get_flag() const {
        return Flags::RxFifoFull;
    }

    IResult<> response_flag(){
        const char str[2] = {
            char(get_flag()), 'r'
        };

        return phy_.send_str(StringView(str, 2));
    }

    IResult<> handle_set_baud(const StringView str){
        if(str.size() == 0) return Err(Error::NoArg);
        if(str.size() > 1) return Err(Error::ArgTooLong);
        const char baud = str[0];
        switch(baud){
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

    IResult<> handle_send_std_msg(const StringView str, const bool is_rmt){
        // riiil\r, 发送标准远程帧, 如 r1000\r 表示 发送 0x100 的远程帧, 返回 z\r 表示 OK, \b表示 ERROR

        if(str.size() == 0) return Err(Error::NoArg);
        if(str.size() < 4) return Err(Error::ArgTooLong);

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

        const StringView data_str = cutter.fetch_remaining();
        
        if(is_rmt){
            const auto data = ({
                const auto res = parse_data(data_str, dlc);
                if(res.is_err()) return Err(res.unwrap_err());
                res.unwrap();
            });
            return phy_.send_can_msg(
                Msg::from_bytes(StdId(id), std::span(data))
            );
        }else{
            if(data_str.size()) return Err(Error::DataExistsInRemote);
            return phy_.send_can_msg(
                Msg::from_remote(StdId(id))
            );
        }
    }

    // Riiiiiiiil\r, 发送扩展远程帧, 如 r1234567F2 表示发送 0x1234567F 且 DLC=2 的 远程帧
    IResult<> handle_send_ext_msg(const StringView str, const bool is_rmt){
        return Ok();
    }
    IResult<> handle_open(const StringView str){ 
        return phy_.open();
    }

    IResult<> handle_close(const StringView str){ 
        return phy_.close();
    }

    static constexpr Msg str2msg(const StringView str) {
        return Msg();
    }

    static constexpr IResult<int> parse_std_id(const StringView str){
        if(str.length() == 0) return Err(Error::NoArg);
        if(str.length() == 3) return Err(Error::InvalidDataLength);

        const int id = int(str);
        if(id > 0x7ff) return Err(Error::InvalidStdId);
        return Ok(id);
    }
    
    static constexpr IResult<int> parse_ext_id(const StringView str){
        if(str.length() == 0) return Err(Error::NoArg);
        if(str.length() == 3) return Err(Error::InvalidDataLength);

        const int id = int(str);
        if(id > 0x7ff) return Err(Error::InvalidExtId);
        return Ok(id);
    }

    static constexpr IResult<std::array<uint8_t, 8>> parse_data(const StringView str, const uint8_t dlc){
        if(str.size() != dlc * 2) return Err(Error::InvalidDataLength);

        std::array<uint8_t, 8> buf;

        for(int i = 0; i < dlc; i++){
            buf[i] = uint8_t(int(str.substr(i * 2, i * 2 + 2)));
        }

        return Ok{buf};
    }

    static constexpr IResult<int> parse_len(const StringView str){
        if(str.length() == 0) return Err(Error::NoArg);
        if(str.length() != 1) return Err(Error::InvalidDataLength);
        const auto len = int(str);
        if(len > 8) return Err(Error::InvalidDataLength);
        return Ok(len);
    }
private:
    AsciiCanPhy & phy_;
};

}