#pragma once

#include "core/string/String.hpp"
#include "core/utils/Result.hpp"
#include "hal/bus/can/can_msg.hpp"

#include <variant>

namespace ymd::robots::asciican{

enum class AsciiCanError{
    NoInput,

    InvalidId,
    InvalidDataLength,
    InvalidData,
    InvalidChecksum,

    InvalidResponse,
    InvalidResponseLength,
    InvalidResponseData,
    InvalidResponseChecksum,
    InvalidResponseId,

    InvalidCommand,
    InvalidBaud,
    InvalidStdId,
    InvalidExtId,
    NotSupportedYet,
    NoArg,
    DataExistsInRemote,
    ArgTooLong
};

template<typename T = void>
using IResult = Result<T, AsciiCanError>;




class StringCutter{
public:
    StringCutter(const StringView str): str_(str){}

    StringView fetch_next(const size_t len){
        const auto res = str_.substr(pos_, len);
        pos_ += len;
        return res;
    }

    StringView fetch_remaining(){
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
            if(strv.size() > MAX_STR_LEN) __builtin_abort();
            memcpy(ret.str, strv.data(), strv.size());
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

    enum class Flags:uint8_t{
        RxFifoFull,
        TxFifoFull,
        ErrorWaring,
    };
public:
    [[nodiscard]] IResult<> send_can_msg(const Msg && msg){
        return Ok();
    }

    [[nodiscard]] IResult<> send_str(const StringView str){
        return Ok();
    }

    [[nodiscard]] IResult<> set_stream_baud(const uint32_t baud){
        return Ok();
    }

    [[nodiscard]] IResult<> set_can_baud(const uint32_t baud){
        return Ok();
    }

    [[nodiscard]] IResult<> open(){
        return Ok();
    }

    [[nodiscard]] IResult<> close(){
        return Ok();
    }

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
};
}    