#pragma once

#include "core/io/regs.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/STMicroIMU.hpp"

namespace ymd::drivers{

struct LIS3DH_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1101001);

    enum class FilteringStrategy:uint8_t{ FIR, IIR};
    enum class Bandwidth:uint8_t{_440Hz, _235Hz};
    
    enum class SelfTestMode{
        Normal,
        PositiveSign,
        NegtiveSign
    };

    enum class FifoMode:uint8_t{
        Bypass      =   0b000,
        Fifo        =   0b001,
        ContinuousUntilTrigger  =   0b011,
        BypassUntilTrigger = 0b100,
        Continuous = 0b110
    };

    using Transport       = StmicroImu_Transport;

    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>; 
    using RegAddr = uint8_t;
};

struct _LIS3DH_Regs:public LIS3DH_Prelude{

    struct R8_WhoAmI:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0F; 
        static constexpr uint8_t key = 0x11;
        uint8_t data;
    }DEF_R8(whoami_reg)


    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x20;
        uint8_t bdu:1;
        uint8_t drdy_pulse:1;
        uint8_t sw_reset:1;
        uint8_t boot:1;
        uint8_t __resv__:2;
        uint8_t if_add_inc:1;
        uint8_t norm_mod_en:1;
    }DEF_R8(ctrl1_reg)

    struct _R8_Int1Ctrl:public Reg8<>{

        uint8_t __resv__:2;
        uint8_t int_ext:1;
        uint8_t int_fth:1;
        uint8_t int_fss5:1;
        uint8_t int_ovr:1;
        uint8_t int_boot:1;
        uint8_t int_drdy:1;
    };
    struct R8_Int1Ctrl:public _R8_Int1Ctrl{
        static constexpr RegAddr ADDRESS = 0x21;
    }DEF_R8(int1_ctrl_reg)

    struct R8_Int2Ctrl:public _R8_Int1Ctrl{
        static constexpr RegAddr ADDRESS = 0x22;
    }DEF_R8(int2_ctrl_reg)

    struct R8_Ctrl4:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x23;
        
        uint8_t __resv__:1;
        uint8_t fifo_en:1;
        uint8_t pp_od_int1:1;
        uint8_t pp_od_int2:1;
        uint8_t selftest_mode:2;
        uint8_t dsp_bw_sel:1;
        uint8_t dsp_lp_type:1;
    }DEF_R8(ctrl4_reg)

    struct R8_Ctrl5:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x24;
        uint8_t fifo_spi_hs_on:1;
        uint8_t __resv__:7;
    }DEF_R8(ctrl5_reg)

    REG16_QUICK_DEF(0x25, R16_OutTemp, out_temp_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x27;
        uint8_t x_drdy:1;
        uint8_t y_drdy:1;
        uint8_t z_drdy:1;
        uint8_t zyx_drdy:1;
        uint8_t x_overrun:1;
        uint8_t y_overrun:1;
        uint8_t z_overrun:1;
        uint8_t zyx_overrun:1;
    }DEF_R8(status_reg)

    REG16_QUICK_DEF(0x28, R16_OutX, out_x_reg)
    REG16_QUICK_DEF(0x2A, R16_OutY, out_y_reg)
    REG16_QUICK_DEF(0x2C, R16_OutZ, out_z_reg)

    struct R8_FifoCtrl:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2E;
        uint8_t fifo_threshold:5;
        uint8_t fifo_mode:3;
    }DEF_R8(fifo_ctrl_reg)

    struct R8_FifoSrc:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x27;
        uint8_t fss:6;
        uint8_t ovrn:1;
        uint8_t fth:1;
    }DEF_R8(fifo_src_reg)
};


}