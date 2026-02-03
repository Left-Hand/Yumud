#pragma once

#include "core/string/view/string_view.hpp"
#include "core/string/owned/heapless_string.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/utils/scope_guard.hpp"

#include "primitive/can/bxcan_frame.hpp"

#include <variant>

namespace ymd::asciican::primitive{


namespace operations{
    struct [[nodiscard]] SendCanFrame{
        hal::BxCanFrame frame;

        friend OutputStream & operator<<(OutputStream & os, const SendCanFrame & self){ 
            return os << os.field("frame")(self.frame);
        }
    };

    struct [[nodiscard]] SendString{
        static constexpr size_t MAX_TEXT_LEN = 16;

        using IString = HeaplessString<MAX_TEXT_LEN> ;
        IString str;

        static constexpr SendString from_str(const StringView str){
            if(str.size() > MAX_TEXT_LEN) 
                __builtin_trap();

            return SendString{
                .str = IString::from_str(str)
            };
        }

        friend OutputStream & operator<<(OutputStream & os, const SendString & self){ 
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
        uint32_t baudrate;

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
    operations::SendCanFrame, 
    operations::SendString,
    operations::SetSerialBaud,
    operations::SetCanBaud,
    operations::Open,
    operations::Close
>
{    

};

using Frame = hal::BxCanFrame;

enum class Error:uint8_t{
    NoInput,
    NoArg,
    OddPayloadLength,
    PayloadLengthMismatch,
    InvalidDlcFormat,
    PayloadLengthOverflow,
    InvalidCommand,
    InvalidCanBaudrate,
    InvalidSerialBaudrate,

    ArgTooLong,
    ArgTooShort,

    StdIdOverflow,
    ExtIdOverflow,
    
    StdIdTooLong,
    StdIdTooShort,

    ExtIdTooLong,
    ExtIdTooShort,

    InvalidCharInHex,
    PayloadFoundedInRemote,
    NotImplemented,
    WillNeverSupport
};

enum class Flags:uint8_t{
    RxFifoFull,
    TxFifoFull,
    ErrorWaring,
};

::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const Error & value);




}    