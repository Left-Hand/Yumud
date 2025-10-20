#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct IST8310_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x0E);
    using RegAddr = uint8_t;

    using Error = ImuError;

    template<typename T = void>
    using IResult= Result<T, Error>;

    enum class AverageTimes:uint8_t{
        _1 = 0b000,
        _2 = 0b001,
        _4 = 0b010,
        _8 = 0b011,
        _16 = 0b100,
    };
    
};

struct IST8310_Regs:public IST8310_Prelude{
    struct R8_WhoAmI:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x00;
        static constexpr uint8_t expected_value = 0x10;
        uint8_t data;
    }DEF_R8(whoami_reg)

    struct R8_Status1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x02;

        uint8_t drdy:1;
        uint8_t ovf:1;
        uint8_t :6;
    }DEF_R8(status1_reg)

    struct R16_AxisX:public Reg16i<>{
        static constexpr RegAddr ADDRESS = 0x03;

        int16_t data;
    }DEF_R16(axis_x_reg)

    struct R16_AxisY:public Reg16i<>{
        static constexpr RegAddr ADDRESS = 0x05;

        int16_t data;
    }DEF_R16(axis_y_reg)

    struct R16_AxisZ:public Reg16i<>{
        static constexpr RegAddr ADDRESS = 0x07;

        int16_t data;
    }DEF_R16(axis_z_reg)

    struct R8_Status2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x09;

        uint8_t :3;
        uint8_t on_int:1;
        uint8_t :4;
    }DEF_R8(status2_reg)

    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0A;

        uint8_t awake:1;
        uint8_t cont:1;
        uint8_t :6;
    }DEF_R8(ctrl1_reg)

    struct R8_Ctrl2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0B;

        uint8_t reset:1;
        uint8_t :1;
        uint8_t drdy_level:1;
        uint8_t int_en:1;
        uint8_t :4;
    }DEF_R8(ctrl2_reg)

    struct R8_SelfTest:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0C;

        uint8_t :6;
        uint8_t st_en:1;
        uint8_t :1;
    }DEF_R8(self_test_reg)

    struct R16_Temp:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1C;
        uint16_t data;

        constexpr q16 to_temp() const {
            return (this->as_val() * q16(0.8) - 75);
        }
    }DEF_R16(temp_reg)

    struct R8_Average:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x41;

        AverageTimes x_times:3;
        AverageTimes y_times:3;
        uint8_t :2;
    }DEF_R8(average_reg)

};


}