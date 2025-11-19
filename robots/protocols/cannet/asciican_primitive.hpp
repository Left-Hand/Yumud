#pragma once

#include "core/string/string_view.hpp"
#include "core/string/fixed_string.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"

#include "primitive/can/can_msg.hpp"

#include <variant>

namespace ymd::asciican::primitive{


namespace operations{
    struct [[nodiscard]] SendCanMsg{
        hal::CanClassicMsg msg;

        friend OutputStream & operator<<(OutputStream & os, const SendCanMsg & self){ 
            return os << os.field("msg")(self.msg);
        }
    };

    struct [[nodiscard]] SendText{
        static constexpr size_t MAX_TEXT_LEN = 16;

        using String = FixedString<MAX_TEXT_LEN> ;
        String str;

        static constexpr SendText from_str(const StringView str){
            if(str.size() > MAX_TEXT_LEN) 
                __builtin_trap();

            return SendText{
                .str = String::from_str(str)
            };
        }

        friend OutputStream & operator<<(OutputStream & os, const SendText & self){ 
            return os << os.field("str")(self.str.view());
        }
    };

    struct [[nodiscard]] SetSerialBaud{
        uint32_t baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetSerialBaud & self){ 
            return os << os.field("baudrate")(self.baudrate);
        }
    };

    struct [[nodiscard]] SetCanBaud{
        hal::CanBaudrate baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetCanBaud & self){ 
            return os << os.field("baudrate")(self.baudrate);
        }
    };

    struct [[nodiscard]] Open{
        friend OutputStream & operator<<(OutputStream & os, const Open & self){ 
            return os;
        }
    };

    struct [[nodiscard]] Close{
        friend OutputStream & operator<<(OutputStream & os, const Close & self){ 
            return os;
        }
    };

    struct [[nodiscard]] SetTimestamp{
        Enable enabled;
        friend OutputStream & operator<<(OutputStream & os, const SetTimestamp & self){ 
            return os << os.field("enabled")(self.enabled);
        }
    };
}


struct [[nodiscard]] Operation:public Sumtype<
    operations::SendCanMsg, 
    operations::SendText,
    operations::SetSerialBaud,
    operations::SetCanBaud,
    operations::Open,
    operations::Close
>
{    

};

using Msg = hal::CanClassicMsg;

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