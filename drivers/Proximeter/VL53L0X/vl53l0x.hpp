//这个驱动已经完成
//这个驱动已经测试

//VL53L0X 是意法半导体的一款基础型TOF距离传感器

#pragma once

#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "../DistanceSensor.hpp"


namespace ymd::drivers{

class VL53L0X final{
public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x52);

    enum class Error_Kind:uint8_t{
        VerifyFailed,
        LightTooWeak
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    VL53L0X(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}
    ~VL53L0X(){;}

    [[nodiscard]]
    Result<void, Error> start_conv();

    [[nodiscard]]
    Result<void, Error> init();

    [[nodiscard]]
    Result<void, Error> stop();

    [[nodiscard]]
    Result<real_t, Error> get_distance(){
        if(const auto res = get_distance_mm(); res.is_ok())
            return Ok(res.unwrap() * real_t(0.001));
        else return Err(Error(res.unwrap_err()));
    };

    [[nodiscard]]
    Result<uint16_t, Error> get_distance_mm();

    [[nodiscard]]
    Result<uint16_t, Error> get_ambient_count();

    [[nodiscard]]
    Result<uint16_t, Error> get_signal_count();

	Result<void, Error> enable_high_precision(const bool _highPrec = true);
    Result<void, Error> enable_cont_mode(const bool _continuous = true);
    Result<void, Error> update();

private:
    hal::I2cDrv i2c_drv_;
    bool highPrec_ = false;
    bool continuous_ = false;

    struct ConvResult{
        uint16_t ambientCount; /**< Environment quantity */
        uint16_t signalCount;  /**< A semaphore */
        uint16_t distance;
    };

    ConvResult result, last_result;

    [[nodiscard]]
    Result<void, Error> flush();

    [[nodiscard]]
    Result<bool, Error> is_busy();

    [[nodiscard]]
	Result<void, Error> read_byte_data(const uint8_t reg, uint8_t & data);

    [[nodiscard]]
    Result<void, Error> read_burst(const uint8_t reg, uint16_t * data, const size_t len);

    [[nodiscard]]
    Result<void, Error> write_byte_data(const uint8_t reg, const uint8_t byte);

};

};