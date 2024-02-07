#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "../bus/bus_inc.h"

#define VL53L0X_DEF_I2C_ADDR 0x29

class VL53L0X{
public:
    I2cDrv & bus_drv;

  VL53L0X(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
  ~VL53L0X(){;}

    void startConv();
    void init();
    void stop();
    uint16_t getDistanceMm();
    uint16_t getAmbientCount();
    uint16_t getSignalCount();

	void setHighPrecision(const bool _highPrec);
    void setContinuous(const bool _continuous);
    bool isIdle();	
    void flush();
private:
    bool highPrec = false;
    bool continuous = false;

    struct {
        uint16_t ambientCount; /**< Environment quantity */
        uint16_t signalCount;  /**< A semaphore */
        uint16_t distance; 
    };

    uint16_t last_distance;
	void writeByteData(unsigned char Reg, unsigned char byte){
        bus_drv.writeReg(Reg, byte);
    }

	uint8_t readByteData(unsigned char Reg){
        uint8_t data;
        bus_drv.readReg(Reg, data);
        return data;
    }

	void writeData(unsigned char Reg ,unsigned char *buf, unsigned char Num){
        bus_drv.writePool(Reg, buf, Num, Num, false);
    }

    void requestData(uint8_t reg, uint8_t * data, const size_t len){
        bus_drv.readPool(reg, data, 2, len);
    }
};

#endif


