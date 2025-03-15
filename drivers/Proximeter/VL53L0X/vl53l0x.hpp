#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"
#include "../DistanceSensor.hpp"


namespace ymd::drivers{

class VL53L0X:public DistanceSensor{
protected:
    hal::I2cDrv i2c_drv_;
public:
    scexpr uint8_t default_i2c_addr = 0x52;
    VL53L0X(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    VL53L0X(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(i2c, i2c_addr){;}
    ~VL53L0X(){;}

    void startConv();
    void init();
    void stop();
    real_t getDistance() override {return getDistanceMM() * real_t(0.001);};
    uint16_t getDistanceMM();
    uint16_t getAmbientCount();
    uint16_t getSignalCount();

	void enableHighPrecision(const bool _highPrec = true);
    void enableContMode(const bool _continuous = true);
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
	void writeByteData(const uint8_t Reg, const uint8_t byte){
        i2c_drv_.writeReg(Reg, byte).unwrap();
    }

    void flush();
    bool busy();

	uint8_t readByteData(const uint8_t Reg){
        uint8_t data;
        i2c_drv_.readReg(Reg, data).unwrap();
        return data;
    }

    void requestData(const uint8_t reg, uint16_t * data, const size_t len){
        // sizeof(Result);
        i2c_drv_.readBurst(reg, std::span(data, len), MSB).unwrap();
    }
};

};