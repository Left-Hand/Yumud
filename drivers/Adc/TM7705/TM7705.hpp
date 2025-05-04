#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::drivers{


template<typename T>
class Borrow{
public:
    Borrow(T * ptr):
        ptr_(ptr){;}

    Borrow(std::nullptr_t) = delete;

    T & unwrap() const {return *ptr_;}
private:
    T * ptr_;
};

struct TM7705_Collections{
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    using RegAddress = uint8_t;


    

    struct R8_Comm:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        uint8_t ch0:1;
        uint8_t ch1:1;
        uint8_t stby:1;
        uint8_t read_else_write:1;
        RegAddress rs:3;
        uint8_t drdy:1;
    };

    static_assert(sizeof(R8_Comm) == 1);

    struct R8_Setting:public Reg8<>{
        static constexpr RegAddress address = 0x01;
        // uint8_t 
    };

    struct R8_Clock:public Reg8<>{
        static constexpr RegAddress address = 0x02;
        // uint8_t 
    };

    struct R16_Data:public Reg16<>{
        static constexpr RegAddress address = 0x03;
        // uint16_t 
    };

    struct R8_Test:public Reg8<>{
        static constexpr RegAddress address = 0x04;
        // uint8_t 
    };

    struct R8_Nop:public Reg8<>{
        static constexpr RegAddress address = 0x05;
    };

    struct R24_Offset:public Reg24<>{
        static constexpr RegAddress address = 0x06;
    };

    struct R24_Gain:public Reg24<>{
        static constexpr RegAddress address = 0x07;
    };

    template<typename T = void>
    using IResult = Result<T, Error>;
};

class TM7705_Phy final: public TM7705_Collections{
public:
    struct Config{
        Borrow<hal::Gpio> sclk_gpio;
        Borrow<hal::Gpio> dout_gpio;
        Borrow<hal::Gpio> din_gpio;
        Borrow<hal::Gpio> cs_gpio;
        Borrow<hal::Gpio> drdy_gpio;
    };

    TM7705_Phy(const Config & cfg) :
        sclk_gpio_{cfg.sclk_gpio.unwrap()},
        dout_gpio_{cfg.dout_gpio.unwrap()},
        din_gpio_{cfg.din_gpio.unwrap()},
        cs_gpio_{cfg.cs_gpio.unwrap()},
        drdy_gpio_{cfg.drdy_gpio.unwrap()}
        {;}
private:
    hal::Gpio & sclk_gpio_;
    hal::Gpio & dout_gpio_;
    hal::Gpio & din_gpio_;
    hal::Gpio & cs_gpio_;
    hal::Gpio & drdy_gpio_;

    IResult<> write_byte(uint8_t dat1);
    IResult<> read_byte(uint8_t & ret);
    IResult<> init();

};


class TM7705 final: public TM7705_Collections{
private:
    TM7705_Phy phy_;
};


}