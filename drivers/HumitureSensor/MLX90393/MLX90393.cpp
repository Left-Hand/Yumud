#include "MLX90393.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;


static constexpr uint8_t MLX90393_AXIS_ALL =  (0x0E);      /**< X+Y+Z axis bits for commands. */
static constexpr uint8_t MLX90393_CONF1 =  (0x00);         /**< Gain */
static constexpr uint8_t MLX90393_CONF2 =  (0x01);         /**< Bur.t_r, comm.mode_r */
static constexpr uint8_t MLX90393_CONF3 =  (0x02);         /**< Oversampli.g_r, filt.r_r, res. */
static constexpr uint8_t MLX90393_CONF4 =  (0x03);         /**< Sensitivty drift. */
static constexpr uint8_t MLX90393_GAIN_SHIFT =  (4);       /**< Left-shift for gain bits. */
static constexpr uint8_t MLX90393_HALL_CONF =  (0x0C);     /**< Hall plate spinning rate adj. */
static constexpr uint8_t MLX90393_STATUS_OK =  (0x00);     /**< OK value for status response. */
static constexpr uint8_t MLX90393_STATUS_SMMODE =  (0x08); /**< SM Mode status response. */
static constexpr uint8_t MLX90393_STATUS_RESET =  (0x01);  /**< Reset value for status response. */
static constexpr uint8_t MLX90393_STATUS_ERROR =  (0xFF);  /**< OK value for status response. */
static constexpr uint8_t MLX90393_STATUS_MASK =  (0xFC);   /**< Mask for status OK checks. */


/** Lookup table to convert raw values to uT based on [HALLCONF][GAIN_SEL][RES].
 */
const real_t mlx90393_lsb_lookup[2][8][4][2] = {

    /* HALLCONF = 0xC (default) */
    {
        /* GAIN_SEL = 0, 5x gain */
        {{0.751_r, 1.210_r}, {1.502_r, 2.420_r}, {3.004_r, 4.840_r}, {6.009_r, 9.680_r}},
        /* GAIN_SEL = 1, 4x gain */
        {{0.601_r, 0.968_r}, {1.202_r, 1.936_r}, {2.403_r, 3.872_r}, {4.840_r, 7.744_r}},
        /* GAIN_SEL = 2, 3x gain */
        {{0.451_r, 0.726_r}, {0.901_r, 1.452_r}, {1.803_r, 2.904_r}, {3.605_r, 5.808_r}},
        /* GAIN_SEL = 3, 2.5x gain */
        {{0.376_r, 0.605_r}, {0.751_r, 1.210_r}, {1.502_r, 2.420_r}, {3.004_r, 4.840_r}},
        /* GAIN_SEL = 4, 2x gain */
        {{0.300_r, 0.484_r}, {0.601_r, 0.968_r}, {1.202_r, 1.936_r}, {2.403_r, 3.872_r}},
        /* GAIN_SEL = 5, 1.667x gain */
        {{0.250_r, 0.403_r}, {0.501_r, 0.807_r}, {1.001_r, 1.613_r}, {2.003_r, 3.227_r}},
        /* GAIN_SEL = 6, 1.333x gain */
        {{0.200_r, 0.323_r}, {0.401_r, 0.645_r}, {0.801_r, 1.291_r}, {1.602_r, 2.581_r}},
        /* GAIN_SEL = 7, 1x gain */
        {{0.150_r, 0.242_r}, {0.300_r, 0.484_r}, {0.601_r, 0.968_r}, {1.202_r, 1.936_r}},
    },

    /* HALLCONF = 0x0 */
    {
        /* GAIN_SEL = 0, 5x gain */
        {{0.787_r, 1.267_r}, {1.573_r, 2.534_r}, {3.146_r, 5.068_r}, {6.292_r, 10.137_r}},
        /* GAIN_SEL = 1, 4x gain */
        {{0.629_r, 1.014_r}, {1.258_r, 2.027_r}, {2.517_r, 4.055_r}, {5.034_r, 8.109_r}},
        /* GAIN_SEL = 2, 3x gain */
        {{0.472_r, 0.760_r}, {0.944_r, 1.521_r}, {1.888_r, 3.041_r}, {3.775_r, 6.082_r}},
        /* GAIN_SEL = 3, 2.5x gain */
        {{0.393_r, 0.634_r}, {0.787_r, 1.267_r}, {1.573_r, 2.534_r}, {3.146_r, 5.068_r}},
        /* GAIN_SEL = 4, 2x gain */
        {{0.315_r, 0.507_r}, {0.629_r, 1.014_r}, {1.258_r, 2.027_r}, {2.517_r, 4.055_r}},
        /* GAIN_SEL = 5, 1.667x gain */
        {{0.262_r, 0.422_r}, {0.524_r, 0.845_r}, {1.049_r, 1.689_r}, {2.097_r, 3.379_r}},
        /* GAIN_SEL = 6, 1.333x gain */
        {{0.210_r, 0.338_r}, {0.419_r, 0.676_r}, {0.839_r, 1.352_r}, {1.678_r, 2.703_r}},
        /* GAIN_SEL = 7, 1x gain */
        {{0.157_r, 0.253_r}, {0.315_r, 0.507_r}, {0.629_r, 1.014_r}, {1.258_r, 2.027_r}},
    }};

/** Lookup table for conversion time based on [DIF_FILT][OSR].
 */
const real_t mlx90393_tconv[8][4] = {
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


using Error = MLX90393::Error;

template<typename T = void>
using IResult = Result<T, Error>;

using Gain = MLX90393::Gain;
using Resolution = MLX90393::Resolution;
using OverSampling = MLX90393::OverSampling;
using Filter = MLX90393::Filter;

IResult<> MLX90393::_init(void) {

    if(const auto res = exit_mode(); res.is_err())
        return Err(res.unwrap_err());

    if(const auto res = reset(); res.is_err())
        return Err(res.unwrap_err());

    /* Set gain and sensor config. */
    if(const auto res = set_gain(MLX90393_GAIN_1X) ; res.is_err())
        return Err(res.unwrap_err());

    /* Set resolution. */
    if(const auto res = set_resolution(MLX90393_X, MLX90393_RES_16); res.is_err())
        return Err(res.unwrap_err());
    if(const auto res = set_resolution(MLX90393_Y, MLX90393_RES_16); res.is_err())
        return Err(res.unwrap_err());
    if(const auto res = set_resolution(MLX90393_Z, MLX90393_RES_16); res.is_err())
        return Err(res.unwrap_err());

    /* Set oversampling. */
    if(const auto res = set_oversampling(MLX90393_OSR_3); res.is_err())
        return Err(res.unwrap_err());

    /* Set digital filtering. */
    if(const auto res = set_filter(MLX90393_FILTER_7); res.is_err())
        return Err(res.unwrap_err());

    /* set INT pin to output interrupt */
    if(const auto res = set_trig_int(false) ; res.is_err())
        return Err(res.unwrap_err());

    return Ok();
}


IResult<> MLX90393::exit_mode(void) {
    const uint8_t tx[1] = {MLX90393_REG_EX};
    uint8_t rx[1] = {0};
    /* Perform the transaction. */
    return transceive(std::span(rx), std::span(tx), 0);
}


IResult<> MLX90393::reset(void) {
    const uint8_t tx[1] = {MLX90393_REG_RT};
    uint8_t rx[1] = {0};

    /* Perform the transaction. */
    return (transceive(std::span(rx), std::span(tx), 5));
}


IResult<> MLX90393::set_gain(Gain gain) {
    _gain = gain;

    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF1, data); 
        res.is_err()) return res;

    // mask off gain bits
    data &= ~0x0070;
    // set gain bits
    data |= gain << MLX90393_GAIN_SHIFT;

    return write_register(MLX90393_CONF1, data);
}

IResult<Gain> MLX90393::get_gain(void) {
    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF1, data);
        res.is_err()) return Err(res.unwrap_err());

    // mask off gain bits
    data &= 0x0070;

    return Ok(Gain(data >> 4));
}

IResult<> MLX90393::set_resolution(Axis axis, Resolution resolution) {

    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF3, data);
        res.is_err()) return Err(res.unwrap_err());

    switch (axis) {
    case MLX90393_X:
        _res_x = resolution;
        data &= ~0x0060;
        data |= resolution << 5;
        break;
    case MLX90393_Y:
        _res_y = resolution;
        data &= ~0x0180;
        data |= resolution << 7;
        break;
    case MLX90393_Z:
        _res_z = resolution;
        data &= ~0x0600;
        data |= resolution << 9;
        break;
    }

    return write_register(MLX90393_CONF3, data);
}

IResult<Resolution> MLX90393::get_resolution(Axis axis) {
    switch (axis) {
    case MLX90393_X:
        return Ok(_res_x);
    case MLX90393_Y:
        return Ok(_res_y);
    case MLX90393_Z:
        return Ok(_res_z);
    }
    // shouldn't get here, but to make compiler happy...
    return Ok(_res_x);
}


IResult<> MLX90393::set_filter(Filter filter) {
    _dig_filt = filter;

    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF3, data);
        res.is_err()) return res;

    data &= ~0x1C;
    data |= filter << 2;

    return write_register(MLX90393_CONF3, data);
}

IResult<Filter> MLX90393::get_filter(void) { 
    return Ok(_dig_filt); 
}


IResult<> MLX90393::set_oversampling(OverSampling oversampling) {
    _osr = oversampling;

    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF3, data);
        res.is_err()) return res;

    data &= ~0x03;
    data |= oversampling;

    return write_register(MLX90393_CONF3, data);
}

IResult<> MLX90393::set_trig_int(bool state) {
    uint16_t data;
    if(const auto res = read_register(MLX90393_CONF2, data);
        res.is_err()) return res;

    // mask off trigint bit
    data &= ~0x8000;

    // set trigint bit if desired
    if (state) {
        /* Set the INT, highest bit */
        data |= 0x8000;
    }

    return write_register(MLX90393_CONF2, data);
}


IResult<> MLX90393::start_single_measurement(void) {
    const uint8_t tx[1] = {MLX90393_REG_SM | MLX90393_AXIS_ALL};
    uint8_t rx[1] = {0};

    /* Set the device to single measurement mode */
    return transceive(std::span(rx), std::span(tx), 0);
}
IResult<Vector3_t<real_t>> MLX90393::read_measurement() {
    const uint8_t tx[1] = {MLX90393_REG_RM | MLX90393_AXIS_ALL};
    uint8_t rx[6] = {0};

    /* Read a single data sample. */
    if(const auto res = transceive(std::span(rx), std::span(tx), 0);
        res.is_err()) return Err(res.unwrap_err());

    int16_t xi, yi, zi;

    /* Convert data to uT and real_t. */
    xi = (rx[0] << 8) | rx[1];
    yi = (rx[2] << 8) | rx[3];
    zi = (rx[4] << 8) | rx[5];

    if (_res_x == MLX90393_RES_18)
        xi -= 0x8000;
    if (_res_x == MLX90393_RES_19)
        xi -= 0x4000;
    if (_res_y == MLX90393_RES_18)
        yi -= 0x8000;
    if (_res_y == MLX90393_RES_19)
        yi -= 0x4000;
    if (_res_z == MLX90393_RES_18)
        zi -= 0x8000;
    if (_res_z == MLX90393_RES_19)
        zi -= 0x4000;


    return Ok(
        Vector3_t{
            real_t(xi) * mlx90393_lsb_lookup[0][_gain][_res_x][0],
            real_t(yi) * mlx90393_lsb_lookup[0][_gain][_res_y][0],
            real_t(zi) * mlx90393_lsb_lookup[0][_gain][_res_z][1]
        }
    );
}

/**
 * Performs a single X/Y/Z conversion and returns the results.
 *
 * @param x     Pointer to where the 'x' value should be stored.
 * @param y     Pointer to where the 'y' value should be stored.
 * @param z     Pointer to where the 'z' value should be stored.
 *
 * @return True if the operation succeeded, otherwise false.
 */
IResult<Vector3_t<real_t>> MLX90393::read_data() {
    if(const auto res = start_single_measurement(); res.is_err())
        return Err(Error::CantReadData);
  // See MLX90393 Getting Started Guide for fancy formula
  // tconv = f(OSR, DIG_FILT, OSR2, ZYXT)
  // For now, using Table 18 from datasheet
  // Without +10ms delay measurement doesn't always seem to work
    delay(int(mlx90393_tconv[_dig_filt][_osr] + 10));
    return read_measurement();
}

IResult<> MLX90393::write_register(uint8_t reg, uint16_t data) {
    const uint8_t tx[4] = {
        MLX90393_REG_WR,
        (uint8_t)(data >> 8),   // high byte
        (uint8_t)(data & 0xFF), // low byte
        (uint8_t)(reg << 2)
    };   // the register itself, shift up by 2 bits!

    uint8_t rx[1] = {0};
    /* Perform the transaction. */
    return transceive(std::span(rx), std::span(tx), 0);
}

IResult<> MLX90393::read_register(uint8_t reg, uint16_t & data) {
    const uint8_t tx[2] = {
        MLX90393_REG_RR,
        (uint8_t)(reg << 2)
    }; // the register itself, shift up by 2 bits!

    uint8_t rx[2] = {0};

    /* Perform the transaction. */
    if (const auto res = transceive(std::span(rx), std::span(tx), 0); 
        res.is_err()) return Err(res.unwrap_err());

    data = ((uint16_t)rx[0] << 8) | rx[1];

    return Ok();
}


IResult<> MLX90393_Phy::transceive(std::span<uint8_t> rx_pbuf, std::span<const uint8_t> tx_pbuf){
    TODO();
    return Ok();
    // uint8_t status = 0;
    // uint8_t i;
    // uint8_t rxbuf2[rxlen + 2];

    // if (i2c_drv_) {
    //     /* Write stage */
    //     if(const auto res = i2c_drv_->write(txbuf, txlen)) ; res.is_err())
    //     return MLX90393_STATUS_ERROR;
    //     }
    //     delay(interdelay);

    //     /* Read status byte plus any others */
    //     if(const auto res = i2c_drv_->read(rxbuf2, rxlen + 1)) ; res.is_err())
    //     return MLX90393_STATUS_ERROR;
    //     }
    //     status = rxbuf2[0];
    //     for (i = 0; i < rxlen; i++) {
    //     rxbuf[i] = rxbuf2[i + 1];
    //     }
    // }

    // if (spi_drv_) {
    //     spi_drv_->write_then_read(txbuf, txlen, rxbuf2, rxlen + 1, 0x00);
    //     status = rxbuf2[0];
    //     for (i = 0; i < rxlen; i++) {
    //     rxbuf[i] = rxbuf2[i + 1];
    //     }
    //     delay(interdelay);
    // }

    // /* Mask out bytes available in the status response. */
    // return (status >> 2);
}