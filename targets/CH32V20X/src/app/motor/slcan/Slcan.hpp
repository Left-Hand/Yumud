#pragma once

#include "sys/string/String.hpp"
#include "sys/utils/Result.hpp"
#include "hal/bus/can/CanMsg.hpp"

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
    auto devSendMsg(const Msg && msg){
            
    }

    auto devSendString(const StringView str){

    }

    auto devSendMsg(const uint id, bool is_remote, const std::span<const std::byte> buf){
        if(is_remote)devSendMsg(Msg(id));
        else devSendMsg(Msg(id, buf));
    }

    auto devSetBaud(const uint baud){
        
    }
    void devOpen(){
        
    }

    void devClose(){
        
    }
};


class Slcan:public AsciiCanIntf{

private:

    
    //不以\r结尾
    Error handleRecvString(const StringView str){
        if(!str.size()) return Error::NoArg;
        if(str.size() < 1) return Error::InvalidFormat;
        const auto cmd_str = str.substr(1, str.size());
        switch(str[0]){
            default:return Error::InvalidFormat;
            case 'S':return handleSetBaud(cmd_str);

            case 't':return handleSendStdMsg(cmd_str, false);
            case 'r':return handleSendStdMsg(cmd_str, true);

            // case 'T':return handleSendExtMsg(cmd_str, false);
            // case 'R':return handleSendExtMsg(cmd_str, true);

            case 'F':responseFlag();
            case 'O':return handleOpen(cmd_str);
            case 'C':return handleClose(cmd_str);

            case 'V':reponseVersion();
            case 'N':reponseSerialIdx();

            case 'Z':return Error::NotSupportedYet;
        }
        return Error::None;
    }

    void reponseVersion(){
        devSendString("V1013\r");
    }
    
    void reponseSerialIdx(){
        devSendString("NA123\r");
    }

    Flags getFlag() const {
        return Flags::RxFifoFull;
    }

    void responseFlag(){
        const char str[2] = {
            char(getFlag()), 'r'
        };

        devSendString(StringView(str, 2));
    }

    Error handleSetBaud(const StringView str){
        if(!str.size()) return Error::NoArg;
        const char baud = str[0];
        switch(baud){
            default:return Error::InvalidBaud;
            case '0': devSetBaud(10_KHz);
            case '1': devSetBaud(20_KHz);
            case '2': devSetBaud(50_KHz);
            case '3': devSetBaud(100_KHz);
            case '4': devSetBaud(125_KHz);
            case '5': devSetBaud(250_KHz);
            case '6': devSetBaud(500_KHz);
            case '7': devSetBaud(800_KHz);
            case '8': devSetBaud(1000_KHz);
        }
    }

    Error handleSendStdMsg(const StringView str, const bool is_rmt){
        if(!str.size()) return Error::NoArg;
        if(str.size() < 4) return Error::InvalidDataLength;

        const auto id = UNWRAP(parseStdId(str.substr(0, 3)));
        const auto dlc = UNWRAP(parseLen(str.substr(3, 4)));
        const StringView data_str = str.substr(4, str.size());
        
        if(is_rmt){
            const auto data = UNWRAP(parseData(data_str, dlc));
            devSendMsg(id, false, std::span(data));
        }else{
            if(data_str.size()) return Error::InvalidData;
            devSendMsg(id, true, {});
        }

    }

    Error handleOpen(const StringView str){ 
        devOpen();
        return Error::None;
    }

    Error handleClose(const StringView str){ 
        devClose();
        return Error::None;
    }


    static constexpr Msg str2msg(const StringView str) {
        return Msg();
    }

    static constexpr MyResult<int> parseStdId(const StringView str){
        using enum Error;
        return Result<StringView, void>{str}
            .validate([](auto&& s){ return s.size() != 0; }, NoArg)
            .validate([](auto&& s){ return s.size() == 3; }, InvalidDataLength)
            .transform([](auto&& s){ 
                const int id = int(s);
                return rescond(id > 0x7FF, id, InvalidExtId);
            });
    }
    
    static constexpr MyResult<int> parseExtId(const StringView str){
        using enum Error;

        return Result<StringView, void>{str}
            .validate([](auto&& s) -> bool{ return s.size() != 0; }, NoArg)
            .validate([](auto&& s)-> bool{ return s.size() == 3; }, InvalidDataLength)
            .transform([](auto&& s){ 
                const int id = int(s);
                return rescond(id <= 0x7ff, id, InvalidExtId);
            });
    }

    static constexpr MyResult<std::array<std::byte, 8>> parseData(const StringView str, const uint8_t dlc){
        using enum Error;
        if(str.size() != dlc * 2) return Err(InvalidDataLength);

        std::array<std::byte, 8> buf;

        for(int i = 0; i < dlc; i++){
            buf[i] = std::byte(int(str.substr(i * 2, i * 2 + 2)));
        }

        return Ok{buf};
    }

    static constexpr MyResult<int> parseLen(const StringView str){

        using enum Error;

        return Result<StringView, void>{str}
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
    void devSetSwj(const uint8_t swj){

    }

    void devSetBs1(const uint8_t bs1){

    }

    void devSetBs2(const uint8_t bs2){

    }

    void devSetFilter(const uint8_t fidx, const std::span<const std::byte> buf){

    }

    enum class Command{
        SET_FILTER,
        SET_BAUD,
        SET_SWJ,
        SET_BS1,
        SET_BS2
    };

    Error handleSetBaud(const StringView str){
        if(!str.size()) return Error::NoArg;
        const char baud = str[0];
        switch(baud){
            default:return Error::InvalidBaud;
            case '0': devSetBaud(10_KHz);
            case '1': devSetBaud(20_KHz);
            case '2': devSetBaud(50_KHz);
            case '3': devSetBaud(100_KHz);
            case '4': devSetBaud(125_KHz);
            case '5': devSetBaud(250_KHz);
            case '6': devSetBaud(500_KHz);
            case '7': devSetBaud(800_KHz);
            case '8': devSetBaud(1000_KHz);
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

    Error handleInput(const uint8_t data){
        switch(state_){
            default:break;
            case State::GET_HEADER:
                handleHeader(data);
                break;
            case State::GET_CRC:
                break;
            case State::GET_DATA:
                break;
                
        }

        return Error::None;
    }

    Error handleHeader(const uint8_t header){
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


    Error handleOpen(const StringView str){ 
        devOpen();
        return Error::None;
    }

    Error handleClose(const StringView str){ 
        devClose();
        return Error::None;
    }

};


}