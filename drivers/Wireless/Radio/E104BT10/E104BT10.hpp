#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/errno.hpp"

#include "hal/conn/uart/hw_singleton.hpp"


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

    struct [[nodiscard]] Packet final{
        DeviceAddr dst_addr;
        std::array<uint8_t, 8> payload;
        uint8_t len;
        static constexpr Packet from_targ_and_pbuf(DeviceAddr dst_addr, std::span<uint8_t> pbuf){
            std::array<uint8_t, 8> buf;
            std::copy(pbuf.begin(), pbuf.end(), buf.begin());
            return Packet{
                dst_addr,
                buf,
                static_cast<uint8_t>(pbuf.size())
            };
        }

        constexpr std::span<const uint8_t> as_bytes() const noexcept {
            return std::span<const uint8_t>(payload.data(), len);
        }

        constexpr size_t size() const noexcept {
            return len;
        }
    };
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
        // const DeviceAddr dst_addr, 
        // const std::span<const uint8_t> pbuf
        const Packet & msg
    ){
        if(msg.size() > 8)
            return Err(Error::PayloadExceed8Bytes);
        // return write_args(Command::Transmit, dst_addr.addr, pbuf);
        return write_args(Command::Transmit, msg.dst_addr.addr, msg.as_bytes());
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
};

}