#pragma once

#include "drivers/IMU/details/InvensenseIMU.hpp"


// 参考来源：

// 无许可证
// https://github.com/NOKOLat/STM32_ICM45686/blob/master/ICM45686.h

//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。


namespace ymd::drivers{

struct ICM45686_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);

    enum class RegAddr: uint8_t{ 
        AccelDataX1Ui = 0x00,
        PwrMgmt0        = 0x10,
        AccConfig     = 0x1B,
        GyrConfig 	 = 0x1C,
        WhoAmI   		 = 0x72
    };

    enum class [[nodiscard]] Mode: uint8_t{

        Off = 0x00,
        Standby,
        LowPower,
        LowNoise
    };

    enum class [[nodiscard]] AccelFs: uint8_t{

        _32G = 0x00,
        _16G,
        _8G,
        _4G,
        _2G
    };

    enum class [[nodiscard]] GyrFs: uint8_t{

        _4000deg = 0x00,
        _2000deg,
        _1000deg,
        _500deg,
        _250deg,
        _125deg,
        _62deg,
        _31deg,
        _15deg,
        _6deg

    };

    enum class [[nodiscard]] Odr: uint8_t{

        _6400_Hz = 3,
        _3200_Hz,
        _1600_Hz,
        _800_Hz,
        _400_Hz,
        _200_Hz,
        _100_Hz,
        _50_Hz,
        _25_Hz,
        _12_Hz,
        _6_Hz,
        _3_Hz,
        _1_Hz
    };
};

struct ICM45686_Regs:public ICM45686_Prelude{

    struct R8_Int1Config0:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr{0x16};

        uint8_t en_fifo_full:1;
        uint8_t en_fifo_ths:1;
        uint8_t en_drdy:1;
        uint8_t en_aux1_drdy:1;
        uint8_t en_ap_fsync:1;
        uint8_t en_ap_agc_rdy:1;
        uint8_t en_aux1_agc_rdy:1;
        uint8_t en_reset_done:1;
    };

    struct R8_Int1Config1:public Reg8<>{
        static constexpr auto ADDRESS = RegAddr{0x17};

        uint8_t en_pll_rdy:1;
        uint8_t en_wom_x:1;
        uint8_t en_wom_y:1;
        uint8_t en_wom_z:1;
        uint8_t en_i3c_protocol_err:1;
        uint8_t en_i2cm_done:1;
        uint8_t en_apex_event:1;
    };

    math::Vec3<int16_t> acc_bits_ = math::Vec3<int16_t>::ZERO;
    math::Vec3<int16_t> gyr_bits_ = math::Vec3<int16_t>::ZERO;
};

}