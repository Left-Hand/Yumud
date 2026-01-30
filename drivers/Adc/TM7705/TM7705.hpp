#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::drivers{

struct TM7705_Prelude{
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    using RegAddr = uint8_t;

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct TM7705_Regset:public TM7705_Prelude{
    struct R8_Comm:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00;
        uint8_t ch0:1;
        uint8_t ch1:1;
        uint8_t stby:1;
        uint8_t read_else_write:1;
        RegAddr rs:3;
        uint8_t drdy:1;
    };

    static_assert(sizeof(R8_Comm) == 1);

    struct R8_Setting:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x01;
        // uint8_t 
    };

    struct R8_Clock:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x02;
        // uint8_t 
    };

    struct R16_Data:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x03;
        // uint16_t 
    };

    struct R8_Test:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x04;
        // uint8_t 
    };

    struct R8_Nop:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x05;
    };

    struct R24_Offset:public Reg24<>{
        static constexpr RegAddr ADDRESS = 0x06;
    };

    struct R24_Gain:public Reg24<>{
        static constexpr RegAddr ADDRESS = 0x07;
    };
};

class TM7705_Transport final: public TM7705_Prelude{
public:
    struct Config{
        Some<hal::Gpio *> sclk_pin;
        Some<hal::Gpio *> dout_pin;
        Some<hal::Gpio *> din_pin;
        Some<hal::Gpio *> cs_pin;
        Some<hal::Gpio *> drdy_pin;
    };

    TM7705_Transport(const Config & cfg) :
        sclk_pin_{cfg.sclk_pin.deref()},
        dout_pin_{cfg.dout_pin.deref()},
        din_pin_{cfg.din_pin.deref()},
        cs_pin_{cfg.cs_pin.deref()},
        drdy_pin_{cfg.drdy_pin.deref()}
        {;}
private:
    hal::Gpio & sclk_pin_;
    hal::Gpio & dout_pin_;
    hal::Gpio & din_pin_;
    hal::Gpio & cs_pin_;
    hal::Gpio & drdy_pin_;

    IResult<> write_byte(uint8_t dat1);
    IResult<> read_byte(uint8_t & ret);
    IResult<> init();

};


class TM7705 final: public TM7705_Prelude{
private:
    TM7705_Transport transport_;
};


}