#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"

#include "primitive/can/can_msg.hpp"

#include <variant>

namespace ymd::robots::asciican{
using Msg = hal::CanMsg;

enum class Error:uint8_t{
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

enum class Flags:uint8_t{
    RxFifoFull,
    TxFifoFull,
    ErrorWaring,
};

::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const Error & value);

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



}    