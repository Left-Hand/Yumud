#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"


#include "hal/conn/i2c/i2cdrv.hpp"
#include "hal/conn/spi/spidrv.hpp"

namespace ymd::drivers{

struct MMC5603_Prelude{

    using Error = ImuError;
    template<typename T = void>
    using IResult = Result<T, Error>;
    
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b01100000 >> 1);

    enum class [[nodiscard]] DataRate:uint8_t{
        _75,
        _150,
        _255,
        _1000
    };

    enum class [[nodiscard]] BandWidth:uint8_t{
        _6_6ms,
        _3_5ms,
        _2_0ms,
        _1_2ms
    };

    using RegAddr = uint8_t;

    static constexpr RegAddr REG_ADDR_X = 0x00;
    static constexpr RegAddr REG_ADDR_Y = 0x02;
    static constexpr RegAddr REG_ADDR_Z = 0x04;
};


struct MMC5603_Regset:public MMC5603_Prelude { 



    struct R16_Axis:public Reg16<>{

        uint8_t data_h;
        uint8_t data_l;
    };

    struct R8_ExtAxis:public Reg8<>{
        static constexpr RegAddr REG_ADDR_x = 0x06;
        static constexpr RegAddr REG_ADDR_y = 0x07;
        static constexpr RegAddr REG_ADDR_z = 0x08;

        uint8_t __resv__:4;
        uint8_t data:4;
    };

    struct R8_Temp:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x09};
        uint8_t bits;
        operator int() const {
            return ((uint8_t(*this) * int(0.8 * 65536) >> 16) - 75);
        }
    };

    struct R8_Status1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x18};

        uint8_t __resv__:4;

        uint8_t otp_read_done:1;
        uint8_t selftest_sig:1;
        uint8_t mag_measure_done:1;
        uint8_t temp_measure_done:1;
    };

    struct R8_Odr:public Reg8<>{


        static constexpr RegAddr REG_ADDR = RegAddr{0x1a};
        DataRate datarate;
    };

    struct R8_Ctrl0:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1B};

        uint8_t do_mag_measure:1;
        uint8_t do_temp_measure:1;
        uint8_t __resv__:1;
        uint8_t do_set:1;

        uint8_t do_reset:1;
        uint8_t auto_sr_en:1;
        uint8_t auto_self_test_en:1;
        uint8_t cmm_freq_en:1;
    };

    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1C};

        uint8_t bandwidth:2;
        uint8_t x_inhibit:1;
        uint8_t y_inhibit:1;
        uint8_t z_inhibit:1;

        uint8_t st_enp:1;
        uint8_t st_enm:1;
        uint8_t sw_reset:1;
    };

    struct R8_Ctrl2:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1D};

        //These bits determine how many measurements are done before a set is executed, when the 
        // part is in continuous mode and the automatic set/reset is enabled. From 000 to 111, the sensor 
        // will do one set for every 1, 25, 75, 100, 250, 500, 1000, and 2000 samples. In order to enable 
        // this feature, both En_prd_set and Auto_SR must be set to 1, and the part should work in 
        // continuous mode. Please note that during this operation, the sensor will not be reset.

        uint8_t prd_set:3;
        uint8_t en_prd_set:1;
        uint8_t cont_en:1;
        uint8_t __resv__:2;
        uint8_t high_pwr:1;
    };

    struct R8_AxisSelfTestX:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1e};
        uint8_t bits;
    };

    struct R8_AxisSelfTestY:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x1f};
        uint8_t bits;
    };

    struct R8_AxisSelfTestZ:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x20};
        uint8_t bits;
    };


    struct R8_ProductId:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x39};
        static constexpr uint8_t correct_id = 0b00010000;
        
        uint8_t id;
    };


    VALIDATE_R16(R16_Axis)
    VALIDATE_R8(R8_ExtAxis)
    VALIDATE_R8(R8_Temp)
    VALIDATE_R8(R8_Status1)
    VALIDATE_R8(R8_Odr)
    VALIDATE_R8(R8_Ctrl0)
    VALIDATE_R8(R8_Ctrl1)
    VALIDATE_R8(R8_Ctrl2)
    VALIDATE_R8(R8_AxisSelfTestX)
    VALIDATE_R8(R8_AxisSelfTestY)
    VALIDATE_R8(R8_AxisSelfTestZ)
    VALIDATE_R8(R8_ProductId)

    R16_Axis x_reg;
    R16_Axis y_reg;
    R16_Axis z_reg;
    R8_ExtAxis x_ext_reg;
    R8_ExtAxis y_ext_reg;
    R8_ExtAxis z_ext_reg;
    R8_Temp temp_reg;
    R8_Status1 stat_reg;
    R8_Odr odr_reg;
    R8_Ctrl0 ctrl0_reg;
    R8_Ctrl1 ctrl1_reg;
    R8_Ctrl2 ctrl2_reg;
    R8_AxisSelfTestX x_st_reg;
    R8_AxisSelfTestY y_st_reg;
    R8_AxisSelfTestZ z_st_reg;
    R8_ProductId product_id_reg;

};


}