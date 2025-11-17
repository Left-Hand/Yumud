#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"

#include "primitive/can/can_msg.hpp"

#include <variant>

namespace ymd{
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

struct [[nodiscard]] CharsFiller{
    constexpr explicit CharsFiller(std::span<char> chars):
        chars_(chars){;}

    constexpr void push_char(const char chr){
        if(pos_ >= chars_.size()) [[unlikely]]
            on_overflow();

        chars_[pos_++] = chr;
    }

    constexpr void push_str(const StringView str){
        const auto len = str.size();
        if(pos_ + len >= chars_.size()) [[unlikely]]
            on_overflow();

        std::copy_n(str.begin(), len, chars_.begin() + pos_);
    }

    constexpr void push_hex(const uint32_t int_val, const size_t length){
        for (int32_t i = static_cast<int32_t>(length) - 1; i >= 0; i--) {
            // 每次提取4位（一个十六进制字符）
            uint8_t nibble = (int_val >> (i * 4)) & 0xF;
            
            // 转换为ASCII字符
            if (nibble < 10) {
                push_char('0' + nibble);
            } else {
                push_char('A' + (nibble - 10));
            }
        }
    }
private:
    std::span<char> chars_;
    size_t pos_ = 0;

    __always_inline void on_overflow(){
        __builtin_abort();
    }
};

}

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




}    