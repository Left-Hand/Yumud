#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "../bus/bus_inc.h"

#define VL53L0X_DEF_I2C_ADDR 0x29

class VL53L0X{
public:
    I2cDrv & busdrv;

  VL53L0X(I2cDrv & _busdrv):busdrv(_busdrv){;}
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
    void reflash();
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
        busdrv.writeReg(Reg, byte);
    }

	uint8_t readByteData(unsigned char Reg){
        uint8_t data;
        busdrv.readReg(Reg, data);
        return data;
    }

	void writeData(unsigned char Reg ,unsigned char *buf, unsigned char Num){
        busdrv.writePool(Reg, buf, Num, Num, false);
    }

    void requestData(uint8_t reg, uint8_t * data, const size_t len){
        busdrv.readPool(reg, data, 2, len);
    }
};

#endif


