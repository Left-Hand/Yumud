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

    enum class [[nodiscard]] AverageTimes:uint8_t{
        _1 = 0b000,
        _2 = 0b001,
        _4 = 0b010,
        _8 = 0b011,
        _16 = 0b100,
    };
    
};

struct IST8310_Regset:public IST8310_Prelude{
    // https://zhuanlan.zhihu.com/p/588908861
    // 在大疆的开发手册中找到了寄存器的手册


    struct R8_WhoAmI:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};
        static constexpr uint8_t KEY = 0x10;
        uint8_t bits;
    };

    struct R8_Status1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};

        uint8_t drdy:1;
        uint8_t ovf:1;
        uint8_t :6;
    }DEF_R8(status1_reg)

    //小端
    struct R16_AxisX:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};

        int16_t bits;
    }DEF_R16(axis_x_reg)


    //小端
    struct R16_AxisY:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};

        int16_t bits;
    }DEF_R16(axis_y_reg)

    //小端

    struct R16_AxisZ:public Reg16<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};

        int16_t bits;
    }DEF_R16(axis_z_reg)

    struct R8_Status2:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};

        uint8_t :3;
        uint8_t interrupt_acting:1;
        uint8_t :4;
    }DEF_R8(status2_reg)

    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0A};

        //1:工作 0:休眠
        uint8_t awake:1;
        uint8_t continous:1;
        uint8_t :6;
    }DEF_R8(ctrl1_reg)

    struct R8_Ctrl2:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0B};

        //1:重启 0:不重启
        uint8_t reset:1;
        uint8_t :1;
        uint8_t drdy_level:1;
        uint8_t interrupt_en:1;
        uint8_t :4;
    }DEF_R8(ctrl2_reg)

    struct R8_SelfTest:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0C};

        uint8_t :6;

        //置1自检
        uint8_t st_en:1;
        uint8_t :1;
    }DEF_R8(self_test_reg)


    struct R16_Temp:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1C};
        
        //TODO 全网找不到温度怎么算
        struct TempCode{
            uint16_t bits;

            constexpr iq16 to_celsius() const {
                return (int32_t(uint32_t(bits) * iq16(0.8)) - 75);
            }
        };

        static_assert(sizeof(TempCode) == sizeof(uint16_t));

        TempCode temp_code;

    }DEF_R16(temp_reg)

    struct R8_Average:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x41};

        AverageTimes x_and_z_times:3;
        AverageTimes y_times:3;
        uint8_t :2;
    }DEF_R8(average_reg)

};


}