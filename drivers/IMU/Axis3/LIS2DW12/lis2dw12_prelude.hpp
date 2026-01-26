#pragma once

//这个驱动还在开发中

#include "core/io/regs.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/STMicroIMU.hpp"

namespace ymd::drivers{


struct LIS2DW12_Prelude{
    enum class [[nodiscard]] DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class [[nodiscard]] G:uint8_t{
        _2, _4, _8, _16
    };

    enum class [[nodiscard]] AccOdr:uint8_t{
        _25_32 = 0b0001,
        _25_16,
        _25_8,
        _25_4,
        _25_2,
        _25,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600
    };

    enum class [[nodiscard]] GyrOdr:uint8_t{
        _25 = 0b0110,
        
        _50,
        _100,
        _200,
        _400,
        _800,
        _1600,
        _3200
    };
    
    enum class [[nodiscard]] AccFs:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class [[nodiscard]] GyrFs:uint8_t{
        _2000deg = 0b0000,
        _1000deg,
        _500deg,
        _250deg,
        _125deg
    };

    enum class [[nodiscard]] Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYR_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class [[nodiscard]] PmuType{
        Acc,
        Gyr,
        Mag
    };

    enum class [[nodiscard]] PmuMode{
        Suspend = 0b00,
        Normal = 0b01,
        LowPower = 0b10,
        FastSetup =  0b11
    };

    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;
    

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1101001);
};


struct LIS2DW12_Regs:public LIS2DW12_Prelude{
    using RegAddr = uint8_t;


    struct R8_TempOut:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0D; 
        uint8_t bits;
    };

    struct R8_WhoAmI:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0F; 
        uint8_t bits;
    };

    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x20;
        uint8_t lp_mode:2;
        uint8_t mode:2;
        uint8_t odr:4;    
    };

    struct R8_Ctrl2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x21;
        uint8_t sim:1;
        uint8_t i2c_disable:1;
        uint8_t if_add_inc:1;
        uint8_t bdu:1;
        uint8_t cs_pu_disc:1;
        uint8_t :1;
        uint8_t soft_reset:1;
        uint8_t boot:1;
    };

    struct R8_Ctrl3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x22;
        uint8_t slp_mode_1:1;
        uint8_t slp_mode_sel:1;
        uint8_t :1;
        uint8_t h_active:1;
        uint8_t lir:1;
        uint8_t pp_od:1;
        uint8_t selftest_mode:2;
    };

    struct R8_Ctrl4:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x23;
        uint8_t int1_drdy:1;
        uint8_t int1_fth:1;
        uint8_t int1_diff5:1;
        uint8_t int1_tap:1;
        uint8_t int1_ff:1;
        uint8_t int1_wu:1;
        uint8_t int1_single_tap:1;
        uint8_t int1_6d:1;
    };

    struct R8_Ctrl5:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x24;
        uint8_t int2_drdy:1;
        uint8_t int2_fth:1;
        uint8_t int2_diff5:1;
        uint8_t int2_ovr:1;
        uint8_t int2_drdy_t:1;
        uint8_t int2_boot:1;
        uint8_t int2_sleep_chg:1;
        uint8_t int2_sleep_state:1;
    };

    struct R8_Ctrl6:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x25;
        uint8_t :2;
        uint8_t low_noise:1;
        uint8_t fds:1;
        uint8_t fs:2;
        uint8_t bw_filt:2;
    };

    struct R8_TempOut8:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x26; 
        uint8_t bits;
    };

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x27;
        uint8_t drdy:1;
        uint8_t ff_ia:1;
        uint8_t _6d_ia:1;
        uint8_t single_tap:1;
        uint8_t double_tap:1;
        uint8_t sleep_state:1;
        uint8_t wu_ia:1;
        uint8_t fifo_ths:1;
    };

    struct R16_OutputX:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x28; 
        int16_t bits;
    };
    struct R16_OutputY:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x2A; 
        int16_t bits;
    };
    struct R16_OutputZ:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x2C; 
        int16_t bits;
    };

    struct R8_FifoCtrl:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2E;
        uint8_t fth:5;
        uint8_t fmode:3;
    };

    struct R8_FifoSamples:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2F;
        uint8_t diff0:6;
        uint8_t fifo_ovr:1;
        uint8_t fifo_fth:1;
    };
};


}
