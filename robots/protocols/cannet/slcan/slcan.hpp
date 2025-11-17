#pragma once

// https://blog.csdn.net/weifengdq/article/details/128823317

#include "../asciican_utils.hpp"
#include "core/utils/Match.hpp"
#include "core/string/fixed_string.hpp"
// Options:
//          -o          (send open command 'O\r')
//          -c          (send close command 'C\r')
//          -f          (read status flags with 'F\r' to reset error states)
//          -l          (send listen only command 'L\r', overrides -o)
//          -s <speed>  (set CAN speed 0..8)
//          -S <speed>  (set UART speed in baud)
//          -t <type>   (set UART flow control type 'hw' or 'sw')
//          -b <btr>    (set bit time register value)
//          -F          (stay in foreground; no daemonize)

namespace ymd::robots::slcan{
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


namespace operations{
    struct [[nodiscard]] SendCanMsg{
        hal::CanMsg msg;

        friend OutputStream & operator<<(OutputStream & os, const SendCanMsg & self){ 
            return os << os.field("msg")(os << self.msg);
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
            return os << os.field("str")(os << self.str.view());
        }
    };

    struct [[nodiscard]] SetSerialBaud{
        uint32_t baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetSerialBaud & self){ 
            return os << os.field("baudrate")(os << self.baudrate);
        }
    };

    struct [[nodiscard]] SetCanBaud{
        hal::CanBaudrate baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetCanBaud & self){ 
            return os << os.field("baudrate")(os << self.baudrate);
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
            return os << os.field("enabled")(os << self.enabled);
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

class [[nodiscard]] SlcanParser final{
public:
    using Msg = hal::CanMsg;
    using Error = asciican::Error;
    using Flags = asciican::Flags;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;



    [[nodiscard]] IResult<Operation> handle_line(const StringView str) const;
private:

    [[nodiscard]] operations::SendText response_version() const ;
    [[nodiscard]] operations::SendText response_serial_idx() const ;
    [[nodiscard]] Flags get_flag() const;
    [[nodiscard]] operations::SendText response_flag() const ;
};


struct SlcanResponseFormatter{
    using Msg = asciican::Msg;
    using Error = asciican::Error;
    using Flags = asciican::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using String = FixedString<32>; 

    struct [[nodiscard]] Response{
        String str;
        static constexpr Response from_str(const StringView str){
            return Response{String::from_str(str)};
        }

        static constexpr Response from_empty(){
            return Response{String::from_empty()};
        }

        friend OutputStream& operator<<(OutputStream & os, const Response & self){ 
            return os << self.str << '\r';
        }
    };

    static constexpr Response fmt_operation(const IResult<Operation> & res){
        if(res.is_err()){
            return Response::from_str("Z");
        }else{
            return Response::from_empty();
        }
    }


    static constexpr Response fmt_canmsg(const hal::CanMsg & msg){
        String str;
        auto filler = CharsFiller{str.mut_chars()};
        const auto header_char = msg_to_header_char(msg);
        filler.push_char(header_char);

        auto push_id = [&](){
            const size_t len = msg.is_extended() ? 8 : 3;
            const auto id_u32 = msg.id_as_u32();
            filler.push_hex(id_u32, len);
        };

        auto push_dlc = [&]() {
            const size_t dlc = msg.dlc();
            filler.push_hex(dlc, 1);  // DLC 是1个十六进制字符
        };

        auto push_data = [&](){ 
            const size_t dlc = msg.dlc();
            const auto payload_bytes = msg.payload_bytes();
            for(size_t i = 0; i < dlc; i++){
                filler.push_hex(payload_bytes[i], 2);
            }
        };

        push_id();
        push_dlc();
        push_data();

        return Response{str};
    }
private:
    [[nodiscard]] static constexpr char msg_to_header_char(const hal::CanMsg & msg){
        if(msg.is_remote()){
            return msg.is_extended() ? 'R' : 'r';
        }else{
            return msg.is_extended() ? 'T' : 't';
        }
    };
};

}