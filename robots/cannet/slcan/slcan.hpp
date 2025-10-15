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


namespace operations{
    struct SendCanMsg{
        hal::CanMsg msg;

        friend OutputStream & operator<<(OutputStream & os, const SendCanMsg & self){ 
            return os << os.scoped("SendCanMsg")(os 
                << os.field("msg")(os << self.msg)
            );
        }
    };

    struct SendText{
        static constexpr size_t MAX_TEXT_LEN = 16;

        char str[MAX_TEXT_LEN];

        static constexpr SendText from_str(const StringView strv){
            SendText ret;
            if(strv.size() > MAX_TEXT_LEN) 
                __builtin_trap();

            for(size_t i = 0; i < strv.size(); i++){
                ret.str[i] = strv[i];
            }
            return ret;
        }

        friend OutputStream & operator<<(OutputStream & os, const SendText & self){ 
            return os << os.scoped("SendText")(os 
                << os.field("str")(os << StringView(self.str))
            );
        }
    };

    struct SetSerialBaud{
        uint32_t baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetSerialBaud & self){ 
            return os << os.scoped("SetSerialBaud")(os 
                << os.field("baudrate")(os << self.baudrate)
            );
        }
    };

    struct SetCanBaud{
        hal::CanBaudrate baudrate;

        friend OutputStream & operator<<(OutputStream & os, const SetCanBaud & self){ 
            return os << os.scoped("SetCanBaud")(os 
                << os.field("baudrate")(os << self.baudrate)
            );
        }
    };

    struct Open{
        friend OutputStream & operator<<(OutputStream & os, const Open & self){ 
            return os << os.scoped("Open")(os);
        }
    };

    struct Close{
        friend OutputStream & operator<<(OutputStream & os, const Close & self){ 
            return os << os.scoped("Close")(os);
        }
    };

    struct SetTimestamp{
        Enable enabled;
        friend OutputStream & operator<<(OutputStream & os, const SetTimestamp & self){ 
            return os << os.scoped("SetTimestamp")(os 
                << os.field("enabled")(os << self.enabled)
            );
        }
    };
}


struct Operation:public Sumtype<
    operations::SendCanMsg, 
    operations::SendText,
    operations::SetSerialBaud,
    operations::SetCanBaud,
    operations::Open,
    operations::Close
>
{    

};

class SlcanParser final{
public:
    using Msg = asciican::AsciiCanPhy::Msg;
    using Error = asciican::AsciiCanPhy::Error;
    using Flags = asciican::AsciiCanPhy::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;


    [[nodiscard]] IResult<Operation> handle_line(const StringView str) const;
private:

    [[nodiscard]] operations::SendText response_version() const ;
    [[nodiscard]] operations::SendText response_serial_idx() const ;
    [[nodiscard]] Flags get_flag() const;
    [[nodiscard]] operations::SendText response_flag() const ;
};


struct SlcanEncoder{
    using Msg = asciican::AsciiCanPhy::Msg;
    using Error = asciican::AsciiCanPhy::Error;
    using Flags = asciican::AsciiCanPhy::Flags;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using Str = FixedString<32>; 

    struct [[nodiscard]] Response{
        Str str;
        static constexpr Response from_str(const char * str){
            return Response{Str::from_str(str)};
        }

        static constexpr Response from_empty(){
            return Response{Str::from_empty()};
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
        Str str;

        auto get_header = [&]() -> char{
            if(msg.is_remote()){
                return msg.is_extended() ? 'R' : 'r';
            }else{
                return msg.is_extended() ? 'T' : 't';
            }
        };

        auto add_hex = [&str](const uint32_t value, const size_t length) {
            // 从最高位开始处理，填充到指定长度
            for (int32_t i = static_cast<int32_t>(length) - 1; i >= 0; i--) {
                // 每次提取4位（一个十六进制字符）
                uint8_t nibble = (value >> (i * 4)) & 0xF;
                
                // 转换为ASCII字符
                if (nibble < 10) {
                    str.push_back_unchecked('0' + nibble);
                } else {
                    str.push_back_unchecked('A' + (nibble - 10));
                }
            }
        };

        auto add_id = [&](){
            const size_t len = msg.is_extended() ? 8 : 3;
            const auto id_u32 = msg.id_as_u32();
            add_hex(id_u32, len);
        };

        auto add_dlc = [&]() {
            const size_t dlc = msg.dlc();
            add_hex(dlc, 1);  // DLC 是1个十六进制字符
        };

        auto add_data = [&](){ 
            const size_t dlc = msg.dlc();
            const auto payload_bytes = msg.iter_payload();
            for(size_t i = 0; i < dlc; i++){
                add_hex(payload_bytes[i], 2);
            }
        };

        str.push_back_unchecked(get_header());
        add_id();
        add_dlc();
        add_data();

        return Response{str};
    }
};

}