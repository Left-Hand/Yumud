#pragma once

// https://github.com/adafruit/MLX90393_Library/blob/master/MLX90393.h
// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/mlx90393-3d-hall-sensor.html

#include <tuple>
#include <optional>

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "types/vectors/vector3.hpp"

#include "core/utils/result.hpp"
#include "core/utils/errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"



namespace ymd::drivers{


struct MLX90393_Prelude{
    enum class Error_Kind{
        CantReadData
    };
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
    
    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x0C >> 1);
    /** Register map. */
    static constexpr uint8_t MLX90393_REG_SB = (0x10);  /**< Start burst mode. */
    static constexpr uint8_t MLX90393_REG_SW = (0x20);  /**< Start wakeup on change mode. */
    static constexpr uint8_t MLX90393_REG_SM = (0x30);  /**> Start single-meas mode. */
    static constexpr uint8_t MLX90393_REG_RM = (0x40);  /**> Read measurement. */
    static constexpr uint8_t MLX90393_REG_RR = (0x50);  /**< Read register. */
    static constexpr uint8_t MLX90393_REG_WR = (0x60);  /**< Write register. */
    static constexpr uint8_t MLX90393_REG_EX = (0x80);  /**> Exit moode. */
    static constexpr uint8_t MLX90393_REG_HR = (0xD0);  /**< Memory recall. */
    static constexpr uint8_t MLX90393_REG_HS = (0x70);  /**< Memory store. */
    static constexpr uint8_t MLX90393_REG_RT = (0xF0);  /**< Reset. */
    static constexpr uint8_t MLX90393_REG_NOP = (0x00); /**< NOP. */

    /** Gain settings for CONF1 register. */
    enum class Gain:uint16_t{
        _5X = (0x00),
        _4X,
        _3X,
        _2_5X,
        _2X,
        _1_67X,
        _1_33X,
        _1X
    };

    /** Resolution settings for CONF3 register. */
    enum class Resolution:uint16_t{
        _16,
        _17,
        _18,
        _19,
    } ;

    /** Axis designator. */
    enum class Axis:size_t{
        X,
        Y,
        Z
    };

    /** Digital filter settings for CONF3 register. */
    enum class Filter:uint16_t{
        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
    };

    /** Oversampling settings for CONF3 register. */
    enum class OverSampling:uint16_t{
        _0,
        _1,
        _2,
        _3
    };

    enum class HallConf:uint16_t{
        _0,
        _1,
        _2,
        _3,
        _4,
        _5,
        _6,
        _7,
        _8,
        _9,
        _10,
        _11,
        _12,
        _13,
        _14,
        _15
    };

    enum class BurstDataRate:uint16_t{
        _0
    };
    

    enum class BurstSel:uint16_t{
        _0
    };

    /** Lookup table to convert raw values to uT based on [HALLCONF][GAIN_SEL][RES].
     */
    static constexpr iq16 LSB_LOOKUP_TABLE[2][8][4][2] = {

        /* HALLCONF = 0xC (default) */
        {
            /* GAIN_SEL = 0, 5x gain */
            {{0.751_iq16, 1.210_iq16}, {1.502_iq16, 2.420_iq16}, {3.004_iq16, 4.840_iq16}, {6.009_iq16, 9.680_iq16}},
            /* GAIN_SEL = 1, 4x gain */
            {{0.601_iq16, 0.968_iq16}, {1.202_iq16, 1.936_iq16}, {2.403_iq16, 3.872_iq16}, {4.840_iq16, 7.744_iq16}},
            /* GAIN_SEL = 2, 3x gain */
            {{0.451_iq16, 0.726_iq16}, {0.901_iq16, 1.452_iq16}, {1.803_iq16, 2.904_iq16}, {3.605_iq16, 5.808_iq16}},
            /* GAIN_SEL = 3, 2.5x gain */
            {{0.376_iq16, 0.605_iq16}, {0.751_iq16, 1.210_iq16}, {1.502_iq16, 2.420_iq16}, {3.004_iq16, 4.840_iq16}},
            /* GAIN_SEL = 4, 2x gain */
            {{0.300_iq16, 0.484_iq16}, {0.601_iq16, 0.968_iq16}, {1.202_iq16, 1.936_iq16}, {2.403_iq16, 3.872_iq16}},
            /* GAIN_SEL = 5, 1.667x gain */
            {{0.250_iq16, 0.403_iq16}, {0.501_iq16, 0.807_iq16}, {1.001_iq16, 1.613_iq16}, {2.003_iq16, 3.227_iq16}},
            /* GAIN_SEL = 6, 1.333x gain */
            {{0.200_iq16, 0.323_iq16}, {0.401_iq16, 0.645_iq16}, {0.801_iq16, 1.291_iq16}, {1.602_iq16, 2.581_iq16}},
            /* GAIN_SEL = 7, 1x gain */
            {{0.150_iq16, 0.242_iq16}, {0.300_iq16, 0.484_iq16}, {0.601_iq16, 0.968_iq16}, {1.202_iq16, 1.936_iq16}},
        },

        /* HALLCONF = 0x0 */
        {
            /* GAIN_SEL = 0, 5x gain */
            {{0.787_iq16, 1.267_iq16}, {1.573_iq16, 2.534_iq16}, {3.146_iq16, 5.068_iq16}, {6.292_iq16, 10.137_iq16}},
            /* GAIN_SEL = 1, 4x gain */
            {{0.629_iq16, 1.014_iq16}, {1.258_iq16, 2.027_iq16}, {2.517_iq16, 4.055_iq16}, {5.034_iq16, 8.109_iq16}},
            /* GAIN_SEL = 2, 3x gain */
            {{0.472_iq16, 0.760_iq16}, {0.944_iq16, 1.521_iq16}, {1.888_iq16, 3.041_iq16}, {3.775_iq16, 6.082_iq16}},
            /* GAIN_SEL = 3, 2.5x gain */
            {{0.393_iq16, 0.634_iq16}, {0.787_iq16, 1.267_iq16}, {1.573_iq16, 2.534_iq16}, {3.146_iq16, 5.068_iq16}},
            /* GAIN_SEL = 4, 2x gain */
            {{0.315_iq16, 0.507_iq16}, {0.629_iq16, 1.014_iq16}, {1.258_iq16, 2.027_iq16}, {2.517_iq16, 4.055_iq16}},
            /* GAIN_SEL = 5, 1.667x gain */
            {{0.262_iq16, 0.422_iq16}, {0.524_iq16, 0.845_iq16}, {1.049_iq16, 1.689_iq16}, {2.097_iq16, 3.379_iq16}},
            /* GAIN_SEL = 6, 1.333x gain */
            {{0.210_iq16, 0.338_iq16}, {0.419_iq16, 0.676_iq16}, {0.839_iq16, 1.352_iq16}, {1.678_iq16, 2.703_iq16}},
            /* GAIN_SEL = 7, 1x gain */
            {{0.157_iq16, 0.253_iq16}, {0.315_iq16, 0.507_iq16}, {0.629_iq16, 1.014_iq16}, {1.258_iq16, 2.027_iq16}},
        }
    };

    static constexpr iq16 get_lsb(bool hallconf_is_0x0, Gain gain_sel, Resolution resolution, size_t i){
        return LSB_LOOKUP_TABLE
            [static_cast<size_t>(hallconf_is_0x0)]
            [static_cast<size_t>(gain_sel)]
            [static_cast<size_t>(resolution)]
            [i];
    }

    /** Lookup table for conversion time based on [DIF_FILT][OSR].
     */
    static constexpr iq16 TCONV_LUT[8][4] = {
        /* DIG_FILT = 0 */
        {1.27_r, 1.84_r, 3.00_r, 5.30_r},
        /* DIG_FILT = 1 */
        {1.46_r, 2.23_r, 3.76_r, 6.84_r},
        /* DIG_FILT = 2 */
        {1.84_r, 3.00_r, 5.30_r, 9.91_r},
        /* DIG_FILT = 3 */
        {2.61_r, 4.53_r, 8.37_r, 16.05_r},
        /* DIG_FILT = 4 */
        {4.15_r, 7.60_r, 14.52_r, 28.34_r},
        /* DIG_FILT = 5 */
        {7.22_r, 13.75_r, 26.80_r, 52.92_r},
        /* DIG_FILT = 6 */
        {13.36_r, 26.04_r, 51.38_r, 102.07_r},
        /* DIG_FILT = 7 */
        {25.65_r, 50.61_r, 100.53_r, 200.37_r},
    };

    static constexpr uint32_t get_conv_time_ms(const Filter _dig_filt, OverSampling _osr){
        return uint32_t(TCONV_LUT[static_cast<size_t>(_dig_filt)][static_cast<size_t>(_osr)] + 10);
    }
};

struct MLX90393_Regset:public MLX90393_Prelude{
    struct R16_Conf1:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x00;

        HallConf hall_conf:4;
        Gain gain_sel:3;
        uint16_t z_series:1;
        uint16_t bist:1;
        uint16_t reserved:7;
    }DEF_R16(conf1_reg)

    struct R16_Conf2:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x01;

        BurstDataRate burst_data_rate:6;
        BurstSel burst_sel:4;
        uint16_t tcmp_en:1;
        uint16_t ext_trg:1;
        uint16_t woc_diff:1;
        uint16_t comm_mode:2;
        uint16_t trig_int:1;
    }DEF_R16(conf2_reg)

    struct R16_Conf3:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x02;
        OverSampling osr:2;
        Filter digit_filter:3;
        Resolution res_x:2;
        Resolution res_y:2;
        Resolution res_z:2;
        OverSampling osr2:2;
        uint16_t reserved:3;
    }DEF_R16(conf3_reg)
};

class MLX90393_Phy final:public MLX90393_Prelude{
private:
    MLX90393_Phy(std::optional<hal::I2cDrv> && i2c_drv, std::optional<hal::SpiDrv> && spi_drv):
        i2c_drv_(std::move(i2c_drv)),
        spi_drv_(std::move(spi_drv)){;}

    std::optional<hal::I2cDrv> i2c_drv_;
    std::optional<hal::SpiDrv> spi_drv_;
public:
    MLX90393_Phy(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MLX90393_Phy(hal::I2cDrv(i2c, addr), std::nullopt){;}

    MLX90393_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveRank slave_index):
        MLX90393_Phy(std::nullopt, hal::SpiDrv(spi, slave_index)){;}

    [[nodiscard]] IResult<> transceive(
        std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf);
};


}