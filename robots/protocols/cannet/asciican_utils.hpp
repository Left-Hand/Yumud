#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"


#include "primitive/can/can_msg.hpp"
#include "hal/bus/can/can.hpp"

#include <variant>

namespace ymd::robots::asciican{

enum class AsciiCanError:uint8_t{
    NoInput,
    NoArg,
    PayloadLengthMismatch,
    PayloadLengthOverflow,
    InvalidCommand,
    UnknownCommand,
    InvalidCanBaudrate,
    InvalidSerialBaudrate,
    StdIdOverflow,
    ExtIdOverflow,
    ArgTooLong,
    ArgTooShort,
    
    StdIdTooLong,
    StdIdTooShort,

    ExtIdTooLong,
    ExtIdTooShort,

    UnsupportedHexChar,
    InvalidFieldInRemoteMsg,
    NotImplemented
};

::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const AsciiCanError & value);

class [[nodiscard]] StrProvider{
public:
    explicit constexpr StrProvider(const StringView str): 
        str_(str){}

    constexpr Option<StringView> fetch_leading(const size_t len){
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



    hal::Can & can_;
};
}    