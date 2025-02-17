#pragma once

#include "sys/string/String.hpp"
#include "hal/bus/can/CanMsg.hpp"

#include <variant>

namespace ymd::drivers{


class AsciiCanIntf{
public:
    using Msg = hal::CanMsg;

    template<typename Ret, typename Error>
    class Result_t {
    private:
        std::variant<Ret, Error> result_;
    
    public:
        // 构造函数，用于成功情况
        constexpr Result_t(Ret value) : result_(std::move(value)) {}
    
        // 构造函数，用于错误情况
        constexpr Result_t(Error error) : result_(std::move(error)) {}
    
        // 检查是否成功
        constexpr bool is_ok() const {
            return std::holds_alternative<Ret>(result_);
        }
    
        // 检查是否出错
        constexpr bool is_err() const {
            return std::holds_alternative<Error>(result_);
        }
    
        // 获取成功值，如果当前是错误状态则抛出异常
        constexpr Ret unwrap() const {
            if (is_ok()) {
                return std::get<Ret>(result_);
            } else {
                HALT
            }
        }
    
        // 获取错误值，如果当前是成功状态则抛出异常
        constexpr Error unwrap_err() const {
            if (is_err()) {
                return std::get<Error>(result_);
            } else {
                HALT
            }
        }
    };


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
    using Result = Result_t<T, Error>;


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

    auto devSendMsg(const uint id, bool is_remote, const std::span<const uint8_t> buf){
        if(is_remote)devSendMsg(Msg(id));
        else devSendMsg(Msg(id, buf.data(), buf.size()));
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
        const auto cmd_str = str.substring(1, str.size());
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
        const char buad = str[0];
        switch(buad){
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

        #define PARSE(name, func, from, to) \
            const auto name##_result = func(str.substring(from, to)); \
            if(name##_result.is_err()) return name##_result.unwrap_err(); \
            const auto name = name##_result.unwrap();\
        
        
        // const auto id_result = parseStdId(str.substr(0, 3));
        // if(id_result.is_err()) return id_result.unwrap_err();
        // const auto id = id_result.unwrap();

        PARSE(id, parseStdId, 0, 3)
        PARSE(dlc, parseLen, 3, 4)

        const StringView data_str = str.substring(4, str.size());
        
        if(is_rmt){
            const auto data_res = parseData(data_str, dlc);
            if(data_res.is_err()) return data_res.unwrap_err();
            const auto data = data_res.unwrap();
            devSendMsg(id, false, data);
        }else{
            if(data_str.size()) return Error::InvalidData;
            devSendMsg(id, true, std::span<const uint8_t>());
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

    static constexpr Result<int> parseStdId(const StringView str){
        if(!str.size()) return Error::NoArg;
        if(str.size() != 3) return Error::InvalidDataLength;
        const auto id = int(str);
        if(id > 0x7FF) return Error::InvalidStdId;
        return id;
    }

    static constexpr Result<int> parseExtId(const StringView str){
        if(!str.size()) return Error::NoArg;
        if(str.size() != 3) return Error::InvalidDataLength;
        const auto id = int(str);
        if(id > 0x7FF) return Error::InvalidExtId;
        return id;
    }

    static constexpr Result<std::array<uint8_t, 8>> parseData(const StringView str, const uint8_t dlc){
        if(str.size() != dlc * 2) return Error::InvalidDataLength;

        std::array<uint8_t, 8> buf;

        for(int i = 0; i < dlc; i++){
            buf[i] = int(str.substring(i * 2, i * 2 + 2));
        }

        return buf;
    }

    static constexpr Result<int> parseLen(const StringView str){
        if(!str.size()) return Error::NoArg;
        if(str.size() != 1) return Error::InvalidDataLength;
        const auto len = int(str);
        if(len > 8) return Error::InvalidDataLength;
        return len;
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

    void devSetFilter(const uint8_t fidx, const std::span<const uint8_t> buf){

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
        const char buad = str[0];
        switch(buad){
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

        // auto result() const{
        //     return reinterpret_cast<T>(this)->result();
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
    uint8_t id[4];
    uint8_t buf[64];

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