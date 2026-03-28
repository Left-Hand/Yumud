#pragma once


#include "core/io/regs.hpp"
#include "core/utils/result.hpp"
#include "hal/conn/i2c/i2cdrv.hpp"
#include "drivers/imu/imu.hpp"



// https://github.com/tstellanova/l3gd20/blob/master/src/lib.rs
// https://github.com/adafruit/Adafruit_L3GD20_U/blob/master/Adafruit_L3GD20_U.cpp

namespace ymd::drivers{

struct L3GD20_Prelude{

enum class RegAddr:uint8_t{
    WHO_AM_I = 0x0F,
    CTRL_REG1 = 0x20,
    CTRL_REG2 = 0x21,
    CTRL_REG3 = 0x22,
    CTRL_REG4 = 0x23,
    CTRL_REG5 = 0x24,
    REFERENCE = 0x25,
    OUT_TEMP = 0x26,
    STATUS_REG = 0x27,
    OUT_X_L = 0x28,
    OUT_X_H = 0x29,
    OUT_Y_L = 0x2A,
    OUT_Y_H = 0x2B,
    OUT_Z_L = 0x2C,
    OUT_Z_H = 0x2D,
    FIFO_CTRL_REG = 0x2E,
    FIFO_SRC_REG = 0x2F,
    INT1_CFG = 0x30,
    INT1_SRC = 0x31,
    INT1_TSH_XH = 0x32,
    INT1_TSH_XL = 0x33,
    INT1_TSH_YH = 0x34,
    INT1_TSH_YL = 0x35,
    INT1_TSH_ZH = 0x36,
    INT1_TSH_ZL = 0x37,
    INT1_DURATION = 0x38,
};

enum class Scale :uint8_t{
    /// 250 Degrees Per Second
    Dps250 = 0x00,
    /// 500 Degrees Per Second
    Dps500 = 0x01,
    /// 2000 Degrees Per Second
    Dps2000 = 0x03,
};

};

struct L3GD20_Regset:public L3GD20_Prelude{

    /* Set CTRL_REG1 (0x20)
    ====================================================================
    BIT  Symbol    Description                                   Default
    ---  ------    --------------------------------------------- -------
    7-6  DR1/0     Output data rate                                   00
    5-4  BW1/0     Bandwidth selection                                00
        3  PD        0 = Power-down mode, 1 = normal/sleep mode          0
        2  ZEN       Z-axis enable (0 = disabled, 1 = enabled)           1
        1  YEN       Y-axis enable (0 = disabled, 1 = enabled)           1
        0  XEN       X-axis enable (0 = disabled, 1 = enabled)           1 */

    /* Switch to normal mode and enable all three channels */

    struct R8_Ctrl1:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr::CTRL_REG1;
        uint8_t xen:1;
        uint8_t yen:1;
        uint8_t zen:1;
        uint8_t pd:1;
        uint8_t bw1:1;
        uint8_t dr1:1;
    };


    /* ------------------------------------------------------------------ */

    /* Set CTRL_REG2 (0x21)
    ====================================================================
    BIT  Symbol    Description                                   Default
    ---  ------    --------------------------------------------- -------
    5-4  HPM1/0    High-pass filter mode selection                    00
    3-0  HPCF3..0  High-pass filter cutoff frequency selection      0000 */

    /* Nothing to do ... keep default values */
    /* ------------------------------------------------------------------ */

    /* Set CTRL_REG3 (0x22)
    ====================================================================
    BIT  Symbol    Description                                   Default
    ---  ------    --------------------------------------------- -------
        7  I1_Int1   Interrupt enable on INT1 (0=disable,1=enable)       0
        6  I1_Boot   Boot status on INT1 (0=disable,1=enable)            0
        5  H-Lactive Interrupt active config on INT1 (0=high,1=low)      0
        4  PP_OD     Push-Pull/Open-Drain (0=PP, 1=OD)                   0
        3  I2_DRDY   Data ready on DRDY/INT2 (0=disable,1=enable)        0
        2  I2_WTM    FIFO wtrmrk int on DRDY/INT2 (0=dsbl,1=enbl)        0
        1  I2_ORun   FIFO overrun int on DRDY/INT2 (0=dsbl,1=enbl)       0
        0  I2_Empty  FIFI empty int on DRDY/INT2 (0=dsbl,1=enbl)         0 */

    /* Nothing to do ... keep default values */
    /* ------------------------------------------------------------------ */

    /* Set CTRL_REG4 (0x23)
    ====================================================================
    BIT  Symbol    Description                                   Default
    ---  ------    --------------------------------------------- -------
        7  BDU       Block Data Update (0=continuous, 1=LSB/MSB)         0
        6  BLE       Big/Little-Endian (0=Data LSB, 1=Data MSB)          0
    5-4  FS1/0     Full scale selection                               00
                                    00 = 250 dps
                                    01 = 500 dps
                                    10 = 2000 dps
                                    11 = 2000 dps
        0  SIM       SPI Mode (0=4-wire, 1=3-wire)                       0 */

    /* ------------------------------------------------------------------ */

    /* Set CTRL_REG5 (0x24)
    ====================================================================
    BIT  Symbol    Description                                   Default
    ---  ------    --------------------------------------------- -------
        7  BOOT      Reboot memory content (0=normal, 1=reboot)          0
        6  FIFO_EN   FIFO enable (0=FIFO disable, 1=enable)              0
        4  HPen      High-pass filter enable (0=disable,1=enable)        0
    3-2  INT1_SEL  INT1 Selection config                              00
    1-0  OUT_SEL   Out selection config                               00 */

    /* Nothing to do ... keep default values */
    /* ------------------------------------------------------------------ */
};

}