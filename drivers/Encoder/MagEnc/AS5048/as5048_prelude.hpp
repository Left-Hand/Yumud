#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

//AS5048是一款18年的14位磁编码器 已经不推荐用于新设计

//AS5048A的接口为SPI
//AS5048B的接口为I2C
//两者的读写时序和寄存器布局都有所差异


namespace ymd::drivers::as5048{


using Error = EncoderError;



struct AS5048A_Regs{
    using RegAddr = uint16_t;
    struct [[nodiscard]] R16_Nop:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x0000;
        uint16_t bits;
    };

    // The CLEAR ERROR FLAG command is implemented as READ 
    // command. This command clears the ERROR FLAG which is 
    // contained in every READ frame. Before the ERROR FLAG is 
    // cleared the error register content comes back with the 
    // information which error type was occurred. On the next new 
    // READ register the ERROR FLAG is cleared
    struct [[nodiscard]] R16_ClearError:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x0001;
        uint16_t framing_error:1;
        uint16_t command_invalid:1;
        uint16_t parity_error:1;
        uint16_t :13;
    };

    struct [[nodiscard]] R16_ProgrammingControl:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x0003;
        uint16_t programming_enable:1;
        uint16_t :2;
        uint16_t burn:1;
        uint16_t :2;
        uint16_t verify:1;
        uint16_t :9;
    };

    struct [[nodiscard]] R16_OtpZeroPositionHi:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x0016;
        uint16_t zero_position_hi_8b:8;
        uint16_t :8;
    };

    struct [[nodiscard]] R16_OtpZeroPositionLo:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x00017;
        uint16_t zero_position_lo_6b:6;
        uint16_t :10;
    };

    struct [[nodiscard]] R16_Agc:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x3ffd;
        uint16_t agc_value:8;
        uint16_t ocf:1;
        uint16_t cof:1;
        uint16_t comp_low:1;
        uint16_t comp_high:1;
        uint16_t :4;
    };


    struct [[nodiscard]] R16_Magnitude:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x3ffe;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_Angle:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x3fff;
        uint16_t bits;
    };
};


struct AS5048B_Regs{
    using RegAddr = uint8_t;

    struct [[nodiscard]] R8_ProgrammingControl:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0003;
        uint8_t programming_enable:1;
        uint8_t :2;
        uint8_t burn:1;
        uint8_t :2;
        uint8_t verify:1;
        uint8_t :1;
    };

    struct [[nodiscard]] R8_I2cSlaveAddress:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0015;
        // I²C slave address slave 
        // address consist of 5 bits 
        // (MSBs) and the hardware 
        // setting of Pins A1 andA2 
        // I²C address <4> is the 
        // inversion defined as '1'
        uint8_t i2c_slave_address:5;
        uint8_t :3;
    };

    struct [[nodiscard]] R8_OtpZeroPositionHi:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0016;
        uint8_t zero_position_hi_8b:8;
    };

    struct [[nodiscard]] R8_OtpZeroPositionLo:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00017;
        uint8_t zero_position_lo_6b:6;
        uint8_t :2;
    };

    struct [[nodiscard]] R8_Agc:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0xfa;
        uint8_t agc_value;
    };

    struct [[nodiscard]] R8_Diagnostics:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0xfb;
        uint8_t ocf:1;
        uint8_t cof:1;
        uint8_t comp_low:1;
        uint8_t comp_high:1;
        uint8_t :4;
    };


    struct [[nodiscard]] R16_Magnitude:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0xfc;
        uint16_t bits;
    };

    struct [[nodiscard]] R16_Angle:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0xff;
        uint16_t bits;
    };
};

}