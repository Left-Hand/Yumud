#pragma once

// https://github.com/adafruit/MLX90393_Library/blob/master/MLX90393.h

#include <tuple>
#include <optional>

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "types/vectors/vector3/Vector3.hpp"

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

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x0C);
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
    enum Gain{
        MLX90393_GAIN_5X = (0x00),
        MLX90393_GAIN_4X,
        MLX90393_GAIN_3X,
        MLX90393_GAIN_2_5X,
        MLX90393_GAIN_2X,
        MLX90393_GAIN_1_67X,
        MLX90393_GAIN_1_33X,
        MLX90393_GAIN_1X
    };

    /** Resolution settings for CONF3 register. */
    enum Resolution {
        MLX90393_RES_16,
        MLX90393_RES_17,
        MLX90393_RES_18,
        MLX90393_RES_19,
    } ;

    /** Axis designator. */
    enum Axis{
        MLX90393_X,
        MLX90393_Y,
        MLX90393_Z
    };

    /** Digital filter settings for CONF3 register. */
    enum Filter {
        MLX90393_FILTER_0,
        MLX90393_FILTER_1,
        MLX90393_FILTER_2,
        MLX90393_FILTER_3,
        MLX90393_FILTER_4,
        MLX90393_FILTER_5,
        MLX90393_FILTER_6,
        MLX90393_FILTER_7,
    };

    /** Oversampling settings for CONF3 register. */
    enum OverSampling{
        MLX90393_OSR_0,
        MLX90393_OSR_1,
        MLX90393_OSR_2,
        MLX90393_OSR_3,
    };
};

struct MLX90393_Regs:public MLX90393_Prelude{

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

    MLX90393_Phy(Some<hal::Spi *> spi, const hal::SpiSlaveIndex slave_index):
        MLX90393_Phy(std::nullopt, hal::SpiDrv(spi, slave_index)){;}

    [[nodiscard]] IResult<> transceive(
        std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf);
};

class MLX90393 final: public MLX90393_Regs{
public:
    MLX90393(MLX90393_Phy && phy):
        phy_(phy){;}

    [[nodiscard]] IResult<> reset(void);
    [[nodiscard]] IResult<> exit_mode(void);
    
    [[nodiscard]] IResult<Vector3<q24>> read_measurement();
    [[nodiscard]] IResult<> start_single_measurement(void);
    
    [[nodiscard]] IResult<> set_gain(Gain gain);
    [[nodiscard]] IResult<Gain> get_gain(void);
    
    [[nodiscard]] IResult<> set_resolution(Axis, Resolution resolution);
    [[nodiscard]] IResult<Resolution> get_resolution(Axis);
    
    [[nodiscard]] IResult<> set_filter(Filter filter);
    [[nodiscard]] IResult<Filter> get_filter(void);
    
    [[nodiscard]] IResult<> set_oversampling(OverSampling oversampling);
    [[nodiscard]] IResult<OverSampling> get_oversampling(void);
    
    [[nodiscard]] IResult<> set_trig_int(bool state);
    [[nodiscard]] IResult<Vector3<q24>> read_data();

    
private:
    MLX90393_Phy phy_;
    [[nodiscard]] IResult<> read_register(uint8_t reg, uint16_t & data);
    [[nodiscard]] IResult<> write_register(uint8_t reg, uint16_t data);
    [[nodiscard]] IResult<> _init(void);
    [[nodiscard]] IResult<> transceive(
        std::span<uint8_t> rx_pbuf, 
        std::span<const uint8_t> tx_pbuf, 
        const uint8_t interdelay
    ){
        return phy_.transceive(rx_pbuf, tx_pbuf);
    }
    
    Gain _gain;
    Resolution _res_x, _res_y, _res_z;
    Filter _dig_filt;
    OverSampling _osr;
};

}