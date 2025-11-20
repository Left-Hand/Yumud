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
using namespace asciican;
using namespace asciican::primitive;
using Error = asciican::primitive::Error;
using Flags = asciican::primitive::Flags;
using Msg = asciican::primitive::Msg;
class [[nodiscard]] SlcanParser final{
public:
    using Msg = hal::CanClassicMsg;


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


    static constexpr Response fmt_canmsg(const hal::CanClassicMsg & msg){
        String str;
        auto filler = CharsFiller{str.mut_chars()};
        const auto header_char = msg_to_header_char(msg);
        filler.push_char(header_char);

        auto push_id = [&](){
            const size_t len = msg.is_extended() ? 8 : 3;
            const auto id_u32 = msg.id_u32();
            filler.push_hex(id_u32, len);
        };

        auto push_dlc = [&]() {
            const size_t length = msg.length();
            filler.push_hex(length, 1);  // DLC 是1个十六进制字符
        };

        auto push_data = [&](){ 
            const size_t length = msg.length();
            const auto payload_bytes = msg.payload_bytes();
            for(size_t i = 0; i < length; i++){
                filler.push_hex(payload_bytes[i], 2);
            }
        };

        push_id();
        push_dlc();
        push_data();

        return Response{str};
    }
private:
    [[nodiscard]] static constexpr char msg_to_header_char(const hal::CanClassicMsg & msg){
        if(msg.is_remote()){
            return msg.is_extended() ? 'R' : 'r';
        }else{
            return msg.is_extended() ? 'T' : 't';
        }
    };
};

}