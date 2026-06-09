#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

struct BMI055_Prelude{
public:

    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] AccRegAddr:uint8_t{
        CHIP_ID                    = (0x00),
        X_L                        = (0x02),
        X_H                        = (0x03),
        Y_L                        = (0x04),
        Y_H                        = (0x05),
        Z_L                        = (0x06),
        Z_H                        = (0x07),
        TEMP                       = (0x08),
        INT_STATUS_0               = (0x09),
        INT_STATUS_1               = (0x0A),
        INT_STATUS_2               = (0x0B),
        INT_STATUS_3               = (0x0C),
        FIFO_STATUS                = (0x0E),
        RANGE                      = (0x0F),
        BW                         = (0x10),
        PMU_LPW                    = (0x11),
        PMU_LOW_POWER              = (0x12),
        DATA_CTRL                  = (0x13),
        SOFTRESET                  = (0x14),
        INT_EN_0                   = (0x16),
        INT_EN_1                   = (0x17),
        INT_EN_2                   = (0x18),
        INT_MAP_0                  = (0x19),
        INT_MAP_1                  = (0x1A),
        INT_MAP_2                  = (0x1B),
        INT_SRC                    = (0x1E),
        INT_OUT_CTRL               = (0x20),
        INT_LATCH                  = (0x21),
        INT_LH_0                   = (0x22),
        INT_LH_1                   = (0x23),
        INT_LH_2                   = (0x24),
        INT_LH_3                   = (0x25),
        INT_LH_4                   = (0x26),
        INT_MOT_0                  = (0x27),
        INT_MOT_1                  = (0x28),
        INT_MOT_2                  = (0x29),
        INT_TAP_0                  = (0x2A),
        INT_TAP_1                  = (0x2B),
        INT_ORIENT_0               = (0x2C),
        INT_ORIENT_1               = (0x2D),
        INT_FLAT_0                 = (0x2E),
        INT_FLAT_1                 = (0x2F),
        FIFO_CONFIG_0              = (0x30),
        SELFTEST                   = (0x32),
        EEPROM_CTRL                = (0x33),
        SERIAL_CTRL                = (0x34),
        OFFSET_CTRL                = (0x36),
        OFC_SETTING                = (0x37),
        OFFSET_X                   = (0x38),
        OFFSET_Y                   = (0x39),
        OFFSET_Z                   = (0x3A),
        TRIM_GPO                   = (0x3B),
        TRIM_GP1                   = (0x3C),
        FIFO_CONFIG_1              = (0x3E),
        FIFO_DATA                  = (0x3F),
    };


    enum class [[nodiscard]] GyrRegAddr:uint8_t{
        CHIP_ID                    = (0x00),
        X_L                        = (0x02),
        X_H                        = (0x03),
        Y_L                        = (0x04),
        Y_H                        = (0x05),
        Z_L                        = (0x06),
        Z_H                        = (0x07),
        INT_STATUS_0               = (0x09),
        INT_STATUS_1               = (0x0A),
        INT_STATUS_2               = (0x0B),
        INT_STATUS_3               = (0x0C),
        FIFO_STATUS                = (0x0E),
        RANGE                      = (0x0F),
        BW                         = (0x10),
        LPM1                       = (0x11),
        LPM2                       = (0x12),
        RATE_HBW                   = (0x13),
        SOFTRESET                  = (0x14),
        INT_EN_0                   = (0x15),
        INT_EN_1                   = (0x16),
        INT_MAP_0                  = (0x17),
        INT_MAP_1                  = (0x18),
        INT_MAP_2                  = (0x19),
        _0_REG                     = (0x1A),
        _1_REG                     = (0x1B),
        _2_REG                     = (0x1C),
        _3_REG                     = (0x1E),
        INT_LATCH                  = (0x21),
        INT_HR_0                   = (0x22),
        INT_HR_1                   = (0x23),
        INT_HR_2                   = (0x24),
        INT_HR_3                   = (0x25),
        INT_HR_4                   = (0x26),
        INT_HR_5                   = (0x27),
        SOC                        = (0x31),
        A_FOC                      = (0x32),
        TRIM_NVM_CTRL              = (0x33),
        SPI3_WDT                   = (0x34),
        OFFSET_COMP                = (0x36),
        OFFSET_COMP_X              = (0x37),
        OFFSET_COMP_Y              = (0x38),
        OFFSET_COMP_Z              = (0x39),
        TRIM_GPO                   = (0x3A),
        TRIM_GP1                   = (0x3B),
        SELFTEST                   = (0x3C),
        FIFO_CONFIG_0              = (0x3D),
        FIFO_CONFIG_1              = (0x3E),
        FIFO_DATA                  = (0x3F),
    };
};

}


