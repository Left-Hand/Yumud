#pragma once

#include "core/string/String.hpp"
#include "core/utils/Result.hpp"
#include "hal/bus/can/can_msg.hpp"

#include <variant>

namespace ymd::drivers{

class AsciiCanIntf{
public:
    using Msg = hal::CanMsg;


    enum class Error{
        None,
        InvalidFormat,
        InvalidId,
        InvalidDataLength,
        InvalidData,
        InvalidChecksum,
        InvalidResponse,
        InvalidResponseLength,
        InvalidResponseData,
        InvalidResponseChecksum,
        InvalidResponseId,
        InvalidBaud,
        InvalidStdId,
        InvalidExtId,
        NotSupportedYet,
        NoArg
    };

    template<typename T>
    using MyResult = Result<T, Error>;


    enum class Flags:uint8_t{
        RxFifoFull,
        TxFifoFull,
        ErrorWaring,
    };
protected:
    auto dev_send_msg(const Msg && msg){
            
    }

    auto dev_send_string(const StringView str){

    }

    auto dev_send_msg(const uint32_t id, bool is_remote, const std::span<const uint8_t> buf){
        if(is_remote) dev_send_msg(Msg::empty(hal::CanStdId(id)));
        else dev_send_msg(Msg::from_bytes(hal::CanStdId(id), buf));
    }

    auto dev_set_baud(const uint32_t baud){
        
    }
    void dev_open(){
        
    }

    void dev_close(){
        
    }
};


class Slcan:public AsciiCanIntf{

private:

    
    //不以\r结尾
    Error handle_recv_string(const StringView str){
        if(!str.size()) return Error::NoArg;
        if(str.size() < 1) return Error::InvalidFormat;
        const auto cmd_str = str.substr(1, str.size());
        switch(str[0]){
            default:return Error::InvalidFormat;
            case 'S':return handle_set_baud(cmd_str);

            case 't':return handle_send_std_msg(cmd_str, false);
            case 'r':return handle_send_std_msg(cmd_str, true);

            // case 'T':return handleSendExtMsg(cmd_str, false);
            // case 'R':return handleSendExtMsg(cmd_str, true);

            case 'F':response_flag();
            case 'O':return handle_open(cmd_str);
            case 'C':return handle_close(cmd_str);

            case 'V':reponse_version();
            case 'N':reponse_serial_idx();

            case 'Z':return Error::NotSupportedYet;
        }
        return Error::None;
    }

    void reponse_version(){
        dev_send_string("V1013\r");
    }
    
    void reponse_serial_idx(){
        dev_send_string("NA123\r");
    }

    Flags get_flag() const {
        return Flags::RxFifoFull;
    }

    void response_flag(){
        const char str[2] = {
            char(get_flag()), 'r'
        };

        dev_send_string(StringView(str, 2));
    }

    Error handle_set_baud(const StringView str){
        if(!str.size()) return Error::NoArg;
        const char baud = str[0];
        switch(baud){
            default:return Error::InvalidBaud;
            case '0': dev_set_baud(10_KHz);
            case '1': dev_set_baud(20_KHz);
            case '2': dev_set_baud(50_KHz);
            case '3': dev_set_baud(100_KHz);
            case '4': dev_set_baud(125_KHz);
            case '5': dev_set_baud(250_KHz);
            case '6': dev_set_baud(500_KHz);
            case '7': dev_set_baud(800_KHz);
            case '8': dev_set_baud(1000_KHz);
        }
    }

    Error handle_send_std_msg(const StringView str, const bool is_rmt){
        if(!str.size()) return Error::NoArg;
        if(str.size() < 4) return Error::InvalidDataLength;

        const auto id = UNWRAP(parse_std_id(str.substr(0, 3)));
        const auto dlc = UNWRAP(parse_len(str.substr(3, 4)));
        const StringView data_str = str.substr(4, str.size());
        
        if(is_rmt){
            const auto data = UNWRAP(parse_data(data_str, dlc));
            dev_send_msg(id, false, std::span(data));
        }else{
            if(data_str.size()) return Error::InvalidData;
            dev_send_msg(id, true, {});
        }

    }

    Error handle_open(const StringView str){ 
        dev_open();
        return Error::None;
    }

    Error handle_close(const StringView str){ 
        dev_close();
        return Error::None;
    }


    static constexpr Msg str2msg(const StringView str) {
        return Msg();
    }

    static constexpr MyResult<int> parse_std_id(const StringView str){
        using enum Error;
        return Result<StringView, void>{Ok(str)}
            .validate([](auto&& s){ return s.size() != 0; }, NoArg)
            .validate([](auto&& s){ return s.size() == 3; }, InvalidDataLength)
            .transform([](auto&& s){ 
                const int id = int(s);
                return rescond(id > 0x7FF, id, InvalidExtId);
            });
    }
    
    static constexpr MyResult<int> parse_ext_id(const StringView str){
        using enum Error;

        return Result<StringView, void>{Ok(str)}
            .validate([](auto&& s) -> bool{ return s.size() != 0; }, NoArg)
            .validate([](auto&& s)-> bool{ return s.size() == 3; }, InvalidDataLength)
            .transform([](auto&& s){ 
                const int id = int(s);
                return rescond(id <= 0x7ff, id, InvalidExtId);
            });
    }

    static constexpr MyResult<std::array<uint8_t, 8>> parse_data(const StringView str, const uint8_t dlc){
        using enum Error;
        if(str.size() != dlc * 2) return Err(InvalidDataLength);

        std::array<uint8_t, 8> buf;

        for(int i = 0; i < dlc; i++){
            buf[i] = uint8_t(int(str.substr(i * 2, i * 2 + 2)));
        }

        return Ok{buf};
    }

    static constexpr MyResult<int> parse_len(const StringView str){

        using enum Error;

        return Result<StringView, void>{Ok(str)}
            .validate([](auto&& s){ return s.size() != 0; }, NoArg)
            .validate([](auto&& s){ return s.size() == 1; }, InvalidDataLength)
            .transform([](auto&& s){ 
                const int len = int(s);
                return rescond(len <= 8, len, InvalidDataLength);
            });
    }
};


class EcCan:public AsciiCanIntf{
private:
    void dev_set_swj(const uint8_t swj){

    }

    void dev_set_bs1(const uint8_t bs1){

    }

    void dev_set_bs2(const uint8_t bs2){

    }

    void dev_set_filter(const uint8_t fidx, const std::span<const uint8_t> buf){

    }

    enum class Command{
        SET_FILTER,
        SET_BAUD,
        SET_SWJ,
        SET_BS1,
        SET_BS2
    };

    Error handle_set_baud(const StringView str){
        if(!str.size()) return Error::NoArg;
        const char baud = str[0];
        switch(baud){
            default:return Error::InvalidBaud;
            case '0': dev_set_baud(10_KHz);
            case '1': dev_set_baud(20_KHz);
            case '2': dev_set_baud(50_KHz);
            case '3': dev_set_baud(100_KHz);
            case '4': dev_set_baud(125_KHz);
            case '5': dev_set_baud(250_KHz);
            case '6': dev_set_baud(500_KHz);
            case '7': dev_set_baud(800_KHz);
            case '8': dev_set_baud(1000_KHz);
        }
    }

    template<typename T, typename Ret>
    class Iterator{
        auto next(const char chr){
            return reinterpret_cast<T>(this)->next(chr);
        }

        // auto MyResult() const{
        //     return reinterpret_cast<T>(this)->MyResult();
        // }
    };

    class BufIterator{
    private:
        uint8_t * buf_;
        uint8_t cnt_ = 0;
        const uint8_t dlc_;
    public:
        BufIterator(uint8_t * buf, uint8_t dlc):
            buf_(buf), dlc_(dlc){;}

        bool next(const uint8_t data){
            buf_[cnt_] = data;
            cnt_++;
            return cnt_ == dlc_;
        }
    };

    BufIterator iter_;


    enum class State{
        IDLE = 0,
        GET_HEADER = 0,
        GET_STDID,
        GET_EXTID,
        GET_CRC,
        GET_DATA,
    };

    // struct States{
    // public:


    //     constexpr auto state() const{
    //         return state_;
    //     }
    //     constexpr auto cnt() const{
    //         return cnt_;
    //     }
    // private:
    //     State state_ = State::IDLE;
    //     uint8_t cnt_ = 0;
    // };

    // States states_;
    State state_;

    struct MsgInfo{
        uint8_t dlc;
        bool is_rmt;
        bool is_ext;
        bool is_canfd;
    };

    enum class Method{
        SEND_MSG,
        RECV_MSG,
        SET_FILTER,
        CMD
    };
    
    MsgInfo msginfo_;
    Method method_;
    uint8_t id_[4];
    uint8_t buf_[64];

    Error handle_input(const uint8_t data){
        switch(state_){
            default:break;
            case State::GET_HEADER:
                handle_header(data);
                break;
            case State::GET_CRC:
                break;
            case State::GET_DATA:
                break;
                
        }

        return Error::None;
    }

    Error handle_header(const uint8_t header){
        if(header & 0x80){// CANFD
            msginfo_ = MsgInfo{
                .dlc = uint8_t(header & 0b111111),
                .is_rmt = false,
                .is_ext = bool(header & 0x40),
                .is_canfd = true
            };
        }else{
            switch((header >> 5) & 0b11){
                case 0://SEND MSG
                    method_ = Method::SEND_MSG;

                    msginfo_ = MsgInfo{
                        .dlc = uint8_t(header & 0b111),
                        .is_rmt = bool(header & 0x08),
                        .is_ext = bool(header & 0x10),
                        .is_canfd = false
                    };

                    break;
                case 1://RECV MSG
                    method_ = Method::RECV_MSG;

                    msginfo_ = MsgInfo{
                        .dlc = uint8_t(header & 0b111),
                        .is_rmt = bool(header & 0x08),
                        .is_ext = bool(header & 0x10),
                        .is_canfd = false
                    };

                    break;
                case 2://SET FILTER
                    method_ = Method::SET_FILTER;
                    break;
                case 3://CMD    
                    method_ = Method::CMD;
                    break;
            }
        }

        return Error::None;
    }


    Error handle_open(const StringView str){ 
        dev_open();
        return Error::None;
    }

    Error handle_close(const StringView str){ 
        dev_close();
        return Error::None;
    }

};


}