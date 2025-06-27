#pragma once

#include "core/string/String.hpp"
#include "core/utils/Result.hpp"
#include "hal/bus/can/can_msg.hpp"
#include "hal/bus/can/can.hpp"
#include "core/utils/Errno.hpp"

#include <variant>

namespace ymd::robots::asciican{

enum class AsciiCanError:uint8_t{
    NoInput,
    NoArg,
    InvalidPayloadLength,
    InvalidCommand,
    InvalidBaud,
    InvalidStdId,
    InvalidExtId,
    ArgTooLong,
    ArgTooShort,
    UnsupportedCharInHex,
    DataExistsInRemote,
    NotImplemented
};


class StringCutter{
public:
    constexpr StringCutter(const StringView str): 
        str_(str){}

    constexpr StringView fetch_next(const size_t len){
        const auto end = MIN(pos_ + len, str_.length());
        const auto res = str_.substr_by_range(pos_, end);
        pos_ = end;
        return res;
    }

    constexpr StringView fetch_remaining(){
        return str_.substr(pos_);
    }
private:
    size_t pos_ = 0;
    StringView str_;
};



class Oper{
public:
    struct SendCanMsg{
        hal::CanMsg msg;
    };

    struct SendStr{
        static constexpr size_t MAX_STR_LEN = 8;

        char str[MAX_STR_LEN];

        static constexpr SendStr from_str(const StringView strv){
            SendStr ret;
            if(strv.size() > MAX_STR_LEN) sys::abort();

            for(size_t i = 0; i < strv.size(); i++){
                ret.str[i] = strv[i];
            }
            return ret;
        }
    };

    struct SetStreamBaud{
        uint32_t baud;
    };

    struct SetCanBaud{
        uint32_t baud;
    };

    struct Open{

    };

    struct Close{

    };

private:
    using Storage = std::variant<
        SendStr,
        SetStreamBaud,
        SetCanBaud,
        Open,
        Close
    >;
    // Storage oper_;
};

class AsciiCanPhy final{
public:
    using Msg = hal::CanMsg;
    using Error = AsciiCanError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Flags:uint8_t{
        RxFifoFull,
        TxFifoFull,
        ErrorWaring,
    };
public:
    AsciiCanPhy(hal::Can & can):
        can_(can){;}

    [[nodiscard]] IResult<> send_can_msg(const Msg && msg);

    [[nodiscard]] IResult<> send_str(const StringView str);

    [[nodiscard]] IResult<> set_stream_baud(const uint32_t baud);

    [[nodiscard]] IResult<> set_can_baud(const uint32_t baud);

    [[nodiscard]] IResult<> open();

    [[nodiscard]] IResult<> close();

    [[nodiscard]] constexpr auto oper_send_can_msg(const Msg && msg){
        return Oper::SendCanMsg{msg};
    }

    [[nodiscard]] constexpr auto oper_send_str(const StringView str){
        return Oper::SendStr::from_str(str);
    }

    [[nodiscard]] constexpr auto oper_set_stream_baud(const uint32_t baud){
        return Oper::SetStreamBaud{baud};
    }

    [[nodiscard]] constexpr auto oper_set_can_baud(const uint32_t baud){
        return Oper::SetCanBaud{baud};
    }

    [[nodiscard]] constexpr auto oper_open(){
        return Oper::Open{};
    }

    [[nodiscard]] constexpr auto oper_close(){
        return Oper::Close{};
    }

    FRIEND_DERIVE_DEBUG(AsciiCanError)

    hal::Can & can_;
};
}    