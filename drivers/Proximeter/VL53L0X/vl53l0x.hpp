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

    Result<void, Error> start_conv();
    Result<void, Error> init();
    Result<void, Error> stop();
    Result<real_t, Error> get_distance(){
        if(const auto res = get_distance_mm(); res.is_ok())
            return Ok(res.unwrap() * real_t(0.001));
        else return Err(Error(res.unwrap_err()));
    };
    Result<uint16_t, Error> get_distance_mm();
    Result<uint16_t, Error> get_ambient_count();
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

    Result<void, Error> flush();
    Result<bool, Error> is_busy();

	Result<void, Error> read_byte_data(const uint8_t reg, uint8_t & data);

    Result<void, Error> read_burst(const uint8_t reg, uint16_t * data, const size_t len);

    Result<void, Error> write_byte_data(const uint8_t reg, const uint8_t byte);

};

};