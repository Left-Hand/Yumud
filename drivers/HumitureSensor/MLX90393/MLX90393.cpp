#include "MLX90393.hpp"

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::drivers;


static constexpr uint8_t MLX90393_AXIS_ALL =  (0x0E);      /**< X+Y+Z axis bits for commands. */
static constexpr uint8_t MLX90393_GAIN_SHIFT =  (4);       /**< Left-shift for gain bits. */
static constexpr uint8_t MLX90393_HALL_CONF =  (0x0C);     /**< Hall plate spinning rate adj. */

using Error = MLX90393::Error;

template<typename T = void>
using IResult = Result<T, Error>;

using Gain = MLX90393::Gain;
using Resolution = MLX90393::Resolution;
using OverSampling = MLX90393::OverSampling;
using Filter = MLX90393::Filter;

IResult<> MLX90393::init() {

    if(const auto res = exit_mode(); res.is_err())
        return Err(res.unwrap_err());

    if(const auto res = reset(); res.is_err())
        return Err(res.unwrap_err());

    /* Set gain and sensor config. */
    if(const auto res = set_gain(Gain::_1X) ; res.is_err())
        return Err(res.unwrap_err());

    /* Set resolution. */
    if(const auto res = set_resolution(Axis::X, Resolution::_16); res.is_err())
        return Err(res.unwrap_err());
    if(const auto res = set_resolution(Axis::Y, Resolution::_16); res.is_err())
        return Err(res.unwrap_err());
    if(const auto res = set_resolution(Axis::Z, Resolution::_16); res.is_err())
        return Err(res.unwrap_err());

    /* Set oversampling. */
    if(const auto res = set_oversampling(OverSampling::_3); res.is_err())
        return Err(res.unwrap_err());

    /* Set digital filtering. */
    if(const auto res = set_filter(Filter::_7); res.is_err())
        return Err(res.unwrap_err());

    /* set INT pin to output interrupt */
    if(const auto res = enable_trig_int(DISEN) ; res.is_err())
        return Err(res.unwrap_err());

    return Ok();
}


IResult<> MLX90393::exit_mode() {
    const uint8_t tx[1] = {MLX90393_REG_EX};
    uint8_t rx[1] = {0};
    /* Perform the transaction. */
    return transceive(std::span(rx), std::span(tx), 0);
}


IResult<> MLX90393::reset() {
    const uint8_t tx[1] = {MLX90393_REG_RT};
    uint8_t rx[1] = {0};

    /* Perform the transaction. */
    return (transceive(std::span(rx), std::span(tx), 5));
}


IResult<> MLX90393::set_gain(Gain gain) {
    auto reg = RegCopy(regs_.conf1_reg);
    reg.gain_sel = gain;
    return write_register(reg);
}


IResult<> MLX90393::set_resolution(Axis axis, Resolution resolution) {
    // if(const auto res = read_register(MLX90393_CONF3, data);
    //     res.is_err()) return Err(res.unwrap_err());
    auto reg = RegCopy(regs_.conf3_reg);
    switch (axis) {
    case Axis::X:
        reg.res_x = resolution;
        break;
    case Axis::Y:
        reg.res_y = resolution;
        break;
    case Axis::Z:
        reg.res_z = resolution;
        break;
    }

    return write_register(reg);
}

IResult<> MLX90393::set_filter(Filter filter) {
    auto reg = RegCopy(regs_.conf3_reg);
    reg.digit_filter = filter;
    return write_register(reg);
}

IResult<> MLX90393::set_oversampling(OverSampling oversampling) {
    auto reg = RegCopy(regs_.conf3_reg);
    reg.osr = oversampling;
    return write_register(reg);
}

IResult<> MLX90393::enable_trig_int(Enable en) {
    auto reg = RegCopy(regs_.conf2_reg);
    reg.trig_int = en == EN;
    return write_register(reg);
}


IResult<> MLX90393::start_single_measurement() {
    const uint8_t tx[1] = {MLX90393_REG_SM | MLX90393_AXIS_ALL};
    uint8_t rx[1] = {0};

    /* Set the device to single measurement mode */
    return transceive(std::span(rx), std::span(tx), 0);
}
IResult<Vec3<q24>> MLX90393::read_measurement() {
    const auto gain = regs_.conf1_reg.gain_sel;
    const auto & reg = regs_.conf3_reg;
    const auto res_x = reg.res_x;
    const auto res_y = reg.res_y;
    const auto res_z = reg.res_z;

    const uint8_t tx[1] = {MLX90393_REG_RM | MLX90393_AXIS_ALL};
    uint8_t rx[6] = {0};

    /* Read a single data sample. */
    if(const auto res = transceive(std::span(rx), std::span(tx), 0);
        res.is_err()) return Err(res.unwrap_err());

    int16_t xi, yi, zi;

    /* Convert data to uT and q16. */
    xi = (rx[0] << 8) | rx[1];
    yi = (rx[2] << 8) | rx[3];
    zi = (rx[4] << 8) | rx[5];

    if (res_x == Resolution::_18)
        xi -= 0x8000;
    if (res_x == Resolution::_19)
        xi -= 0x4000;
    if (res_y == Resolution::_18)
        yi -= 0x8000;
    if (res_y == Resolution::_19)
        yi -= 0x4000;
    if (res_z == Resolution::_18)
        zi -= 0x8000;
    if (res_z == Resolution::_19)
        zi -= 0x4000;


    return Ok(
        Vec3{
            q16(xi) * get_lsb(false, gain, res_x, 0),
            q16(yi) * get_lsb(false, gain, res_y, 0),
            q16(zi) * get_lsb(false, gain, res_z, 1)
        }
    );
}
IResult<Vec3<q24>> MLX90393::read_data() {
    if(const auto res = start_single_measurement(); res.is_err())
        return Err(Error::CantReadData);
    // See MLX90393 Getting Started Guide for fancy formula
    // tconv = f(OSR, DIG_FILT, OSR2, ZYXT)
    // For now, using Table 18 from datasheet
    // Without +10ms delay measurement doesn't always seem to work
    auto & reg = regs_.conf3_reg;
    clock::delay(Milliseconds(get_conv_time_ms(reg.digit_filter, reg.osr)));
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
    //     clock::delay(interdelayms);

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
    //     clock::delay(interdelayms);
    // }

    // /* Mask out bytes available in the status response. */
    // return (status >> 2);
}