//这个驱动已经完成
//这个驱动已经测试

//VL53L0X 是意法半导体的一款基础型TOF距离传感器

#pragma once

#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{


struct VL53L0X_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x52 >> 1);

    enum class Error_Kind:uint8_t{
        VerifyFailed,
        LightTooWeak,
        DataNotReady
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};


class VL53L0X final:public VL53L0X_Prelude{
public:

    explicit VL53L0X(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit VL53L0X(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit VL53L0X(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}

    VL53L0X(const VL53L0X &) = delete;
    VL53L0X(VL53L0X &&) = delete;
    ~VL53L0X(){;}

    [[nodiscard]] IResult<> start_conv();

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> stop();

    [[nodiscard]] IResult<uint16_t> read_distance_mm();

    [[nodiscard]] IResult<uint16_t> read_ambient_count();

    [[nodiscard]] IResult<uint16_t> read_signal_count();

    [[nodiscard]] IResult<> enable_high_precision(const Enable en);
    [[nodiscard]] IResult<> enable_cont_mode(const Enable en);
    [[nodiscard]] IResult<> update();

private:
    hal::I2cDrv i2c_drv_;
    bool high_prec_en_ = false;
    bool continuous_en_ = false;

    struct ConvResult{
        uint16_t ambient_count; /**< Environment quantity */
        uint16_t signal_count;  /**< A semaphore */
        uint16_t distance_mm;
    };

    ConvResult result, last_result;

    [[nodiscard]] IResult<> flush();

    [[nodiscard]] IResult<bool> is_busy();

    [[nodiscard]] IResult<> read_byte_data(const uint8_t reg_addr, uint8_t & data);

    [[nodiscard]] IResult<> read_burst(const uint8_t reg_addr, const std::span<uint16_t> pbuf);

    [[nodiscard]] IResult<> write_byte_data(const uint8_t reg_addr, const uint8_t byte);

};

};