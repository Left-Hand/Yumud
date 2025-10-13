#pragma once

#include "../asciican_utils.hpp"

namespace ymd::robots::asciican{

class EcCan final{
public:
    using Msg = AsciiCanPhy::Msg;
    using Error = AsciiCanPhy::Error;
    using Flags = AsciiCanPhy::Flags;

    template<typename T = void>
    using IResult = Result<T, Error>;

    EcCan(AsciiCanPhy & phy): phy_(phy){;}
private:

    enum class Command{
        SET_FILTER,
        SET_BAUD,
        SET_SWJ,
        SET_BS1,
        SET_BS2
    };

    IResult<> handle_set_baud(const StringView str){
        if(!str.size()) return Err(Error::NoArg);
        const char baud = str[0];
        switch(baud){
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
        return Err(Error::InvalidBaudrate);
    }

    template<typename T, typename Ret>
    class Iterator{
        auto next(const char chr){
            return reinterpret_cast<T>(this)->next(chr);
        }
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

    // BufIterator iter_;


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

    IResult<> handle_input(const uint8_t data){
        switch(state_){
            default:break;
            case State::GET_HEADER:
                return handle_header(data);
            case State::GET_CRC:
                break;
            case State::GET_DATA:
                break;
        }

        return Ok();
    }

    IResult<> handle_header(const uint8_t header){
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

        return Ok();
    }


    IResult<> handle_open(const StringView str){ 
        return phy_.open();
    }

    IResult<> handle_close(const StringView str){ 
        return phy_.close();
    }
private:
    AsciiCanPhy & phy_;
};


}