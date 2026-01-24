#pragma once

// https://blog.csdn.net/weifengdq/article/details/128823317

#include "core/string/owned/heapless_string.hpp"

#include "asciican_utils.hpp"


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
using Frame = asciican::primitive::Frame;


class [[nodiscard]] SlcanParser final{
public:
    template<typename T = void>
    using IResult = Result<T, Error>;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    [[nodiscard]] IResult<Operation> process_line(const StringView str) const;
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

    using IString = HeaplessString<32>; 

    struct [[nodiscard]] Response{
        IString str;
        static constexpr Response from_str(const StringView str){
            return Response{IString::from_str(str)};
        }

        static constexpr Response from_empty(){
            return Response{IString::from_empty()};
        }

        friend OutputStream& operator<<(OutputStream & os, const Response & self){ 
            return os << self.str;
        }
    };

    static constexpr Response fmt_operation(const IResult<Operation> & res){
        if(res.is_err()){
            return Response::from_str("Z");
        }else{
            return Response::from_empty();
        }
    }


    static constexpr Response fmt_canmsg(const Frame & frame){
        IString str;
        auto filler = CharsFiller{str.mut_chars()};
        auto push_id = [&](){
            const size_t num_chars = frame.is_extended() ? 8 : 3;
            const auto id_u32 = frame.id_u32();
            filler.push_hex(id_u32, num_chars);
        };

        auto push_dlc = [&]() {
            const auto dlc = frame.dlc();
            filler.push_hex(dlc.length(), 1);  // DLC 是1个十六进制字符
        };

        auto push_payload = [&](){ 
            const auto payload_bytes = frame.payload_bytes();
            for(size_t i = 0; i < payload_bytes.size(); i++){
                filler.push_hex(payload_bytes[i], 2);
            }
        };

        const auto header_char = frame_to_header_char(frame);
        filler.push_char(header_char);
        push_id();
        push_dlc();
        push_payload();

        return Response{str};
    }
private:
    [[nodiscard]] static constexpr char frame_to_header_char(const Frame & frame){
        if(frame.is_remote()){
            return frame.is_extended() ? 'R' : 'r';
        }else{
            return frame.is_extended() ? 'T' : 't';
        }
    };
};

}