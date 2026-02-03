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



namespace ymd::slcan{


using namespace asciican;
using namespace asciican::primitive;
using Error = asciican::primitive::Error;
using Frame = asciican::primitive::Frame;

struct [[nodiscard]] StatusFlag final{
    using Self = StatusFlag;
    uint8_t rx_queue_full:1;
    uint8_t tx_queue_full:1;
    // Bit 2 Error warning (EI), see SJA1000 datasheet
    // Bit 3 Data Overrun (DOI), see SJA1000 datasheet
    // Bit 4 Not used.
    // Bit 5 Error Passive (EPI), see SJA1000 datasheet
    // Bit 6 Arbitration Lost (ALI), see SJA1000 datasheet *
    // Bit 7 Bus Error (BEI), see SJA1000 datasheet **

    uint8_t ei:1;
    uint8_t doi:1;
    uint8_t __resv__:1;
    uint8_t epi:1;
    uint8_t ali:1;
    uint8_t bei:1;

    static constexpr Self from_bits(uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_nibbles_unchecked(char nib1, char nib2){ 
        uint8_t bits = 0;
        bits |= static_cast<uint8_t>(((nib1 - '0') & 0x0f) << 4);
        bits |= static_cast<uint8_t>(((nib2 - '0') & 0x0f));

        return from_bits(bits);
    }

    static constexpr Option<Self> from_nibbles(char nib1, char nib2){ 
        auto validate_char = [](char ch) -> bool{
            switch(ch){
                case '0' ... '9': return true;
                case 'A' ... 'F': return true;
                case 'a' ... 'f': return true;
            }
            return false;
        };

        if(validate_char(nib1) == false) return None;
        if(validate_char(nib2) == false) return None;
        return Some(from_nibbles_unchecked(nib1, nib2));
    }


    static constexpr Self zero(){
        return from_bits(0);
    }

    constexpr std::tuple<char, char> to_nibbles() const{
        const uint8_t bits = std::bit_cast<uint8_t>(*this);
        // chars[0] = '0' + (bits >> 4);
        // chars[1] = '0' + (bits & 0x0F);
        return std::make_tuple('0' + (bits >> 4), '0' + (bits & 0x0F));
    }
};


class [[nodiscard]] SlcanParser final{
public:
    template<typename T = void>
    using IResult = Result<T, Error>;

    using StdId = hal::CanStdId;
    using ExtId = hal::CanExtId;

    [[nodiscard]] IResult<Operation> process_line(const StringView str) const;
private:

    [[nodiscard]] operations::SendString response_version() const ;
    [[nodiscard]] operations::SendString response_serial_number() const ;
    [[nodiscard]] StatusFlag get_flag() const;
    [[nodiscard]] operations::SendString response_flag() const ;
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
            return Response::from_str("\b");
        }else{
            const Operation & var_op = res.unwrap();
            if(var_op.is<operations::SendCanFrame>()){
                const auto op = var_op.unwrap_as<operations::SendCanFrame>();
                return SlcanResponseFormatter::fmt_canmsg(op.frame);
            }else if(var_op.is<operations::SendString>()){
                return Response::from_str(var_op.unwrap_as<operations::SendString>().str.view());
            }
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