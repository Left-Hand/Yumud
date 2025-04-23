#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"

#include "../DistanceSensor.hpp"

#include "core/math/real.hpp"

namespace ymd::drivers{

class VL53L0X:public DistanceSensor{
protected:
    hal::I2cDrv i2c_drv_;
public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x52);
    VL53L0X(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, addr){;}
    ~VL53L0X(){;}

    void startConv();
    void init();
    void stop();
    real_t get_distance() override {return get_distance_mm() * real_t(0.001);};
    uint16_t get_distance_mm();
    uint16_t get_ambient_count();
    uint16_t get_signal_count();

	void enable_high_precision(const bool _highPrec = true);
    void enable_cont_mode(const bool _continuous = true);
    void update() override;

private:
    bool highPrec = false;
    bool continuous = false;

    struct Result{
        uint16_t ambientCount; /**< Environment quantity */
        uint16_t signalCount;  /**< A semaphore */
        uint16_t distance;
    };

    Result result, last_result;
	void write_byte_data(const uint8_t Reg, const uint8_t byte){
        i2c_drv_.write_reg(Reg, byte).unwrap();
    }

    void flush();
    bool busy();

	uint8_t read_byte_data(const uint8_t Reg){
        uint8_t data;
        i2c_drv_.read_reg(Reg, data).unwrap();
        return data;
    }

    void read_burst(const uint8_t reg, uint16_t * data, const size_t len){
        // sizeof(Result);
        i2c_drv_.read_burst(reg, std::span(data, len), MSB).unwrap();
    }
};

};