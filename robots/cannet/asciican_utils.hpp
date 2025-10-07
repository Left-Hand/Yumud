#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"


#include "hal/bus/can/can_msg.hpp"
#include "hal/bus/can/can.hpp"

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
    
    StdIdTooLong,
    StdIdTooShort,

    ExtIdTooLong,
    ExtIdTooShort,

    UnsupportedCharInHex,
    InvalidFieldInRemoteMsg,
    NotImplemented
};


class StringCutter{
public:
    constexpr StringCutter(const StringView str): 
        str_(str){}

    constexpr Option<StringView> fetch_next(const size_t len){
        const auto next_pos = MIN(pos_ + len, str_.length());
        const auto guard = make_scope_guard([&]{pos_ = next_pos;});
        return str_.substr_by_range(pos_, next_pos);
    }

    constexpr Option<StringView> fetch_remaining(){
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

    struct SendSerialStr{
        static constexpr size_t MAX_STR_LEN = 16;

        char str[MAX_STR_LEN];

        static constexpr SendSerialStr from_str(const StringView strv){
            SendSerialStr ret;
            if(strv.size() > MAX_STR_LEN) sys::abort();

            for(size_t i = 0; i < strv.size(); i++){
                ret.str[i] = strv[i];
            }
            return ret;
        }
    };

    struct SetSerialBaud{
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
        SendSerialStr,
        SetSerialBaud,
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
        return Oper::SendSerialStr::from_str(str);
    }

    [[nodiscard]] constexpr auto oper_set_stream_baud(const uint32_t baud){
        return Oper::SetSerialBaud{baud};
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

    DEF_FRIEND_DERIVE_DEBUG(AsciiCanError)

    hal::Can & can_;
};
}    