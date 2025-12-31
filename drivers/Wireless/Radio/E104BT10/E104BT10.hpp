#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "hal/bus/uart/uart.hpp"


namespace ymd::drivers{

struct E104BT10_Prelude{
    enum class Error_Kind:uint8_t{
        PayloadExceed8Bytes
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Command:uint8_t{
        Transmit = 0xc2,
        Receive = 0x42
    };

    struct DeviceAddr{
        uint16_t addr;
    };

    struct Msg{
        DeviceAddr targ;
        std::array<uint8_t, 8> payload;
        uint8_t len;

        // constexpr Msg(DeviceAddr targ, std::span<const uint8_t> pbuf):
        //     targ(targ), 
        //     len(pbuf.size()){
        //         std::copy(pbuf.begin(), pbuf.end(), payload.begin());
        //     }

        static constexpr Msg from_targ_and_pbuf(DeviceAddr targ, std::span<uint8_t> pbuf){
            std::array<uint8_t, 8> buf;
            std::copy(pbuf.begin(), pbuf.end(), buf.begin());
            return Msg{
                targ,
                buf,
                pbuf.size()
            };
        }

        static constexpr std::span<const uint8_t> as_bytes() const {
            return std::span<const uint8_t>(payload.data(), len);
        }


        constexpr size()const{
            return len;
        }
    }
};

class E104BT10_Transport final:public E104BT10_Prelude{
public:
    IResult<> write_bytes(const std::span<const uint8_t> pbuf);
    IResult<> read_bytes(const std::span<uint8_t> pbuf);

    template<typename... Args>
    IResult<> write_args(Args  &&... args){
        return Ok();
    };


    IResult<> write_msg(
        // const DeviceAddr targ, 
        // const std::span<const uint8_t> pbuf
        const Msg & msg
    ){
        if(msg.size() > 8)
            return Err(Error::PayloadExceed8Bytes);
        // return write_args(Command::Transmit, targ.addr, pbuf);
        return write_args(Command::Transmit, msg.targ.addr, msg.as_bytes());
    }
};

class E104BT10 final:public E104BT10_Prelude{
public:

private:
    using Phy = E104BT10_Transport;
    Phy transport_;
    
    void refactory(){
        transport_.write_args(0x02, 0xc0, 0x15);
    }
}
}