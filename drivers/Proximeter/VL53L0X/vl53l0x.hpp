#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"


class VL53L0X{
protected:
    I2cDrv bus_drv;
public:
    static constexpr uint8_t default_id = 0x52;
    VL53L0X(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    VL53L0X(I2cDrv && _bus_drv):bus_drv(_bus_drv){;}
    VL53L0X(I2c & bus):bus_drv(bus, default_id){;}
    ~VL53L0X(){;}

    void startConv();
    void init();
    void stop();
    uint16_t getDistance();
    uint16_t getAmbientCount();
    uint16_t getSignalCount();

	void enableHighPrecision(const bool _highPrec = true);
    void enableContMode(const bool _continuous = true);
    bool update();

private:
    bool highPrec = false;
    bool continuous = false;

    #pragma pack(push, 1)

    struct Result{
        uint16_t ambientCount; /**< Environment quantity */
        uint16_t signalCount;  /**< A semaphore */
        uint16_t distance;
    };

    #pragma pack(pop)

    Result result, last_result;
	void writeByteData(const uint8_t Reg, const uint8_t byte){
        bus_drv.writeReg(Reg, byte);
    }

    void flush();
    bool isIdle();

	uint8_t readByteData(const uint8_t Reg){
        uint8_t data;
        bus_drv.readReg(Reg, data);
        return data;
    }

    void requestData(const uint8_t reg, uint16_t * data, const size_t len){
        bus_drv.readPool(reg, data, len);
    }
};
