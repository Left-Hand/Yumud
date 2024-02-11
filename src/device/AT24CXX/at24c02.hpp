#ifndef __AT24C02_HPP__

#define __AT24C02_HPP__

#include "defines/device_def.h"

#ifdef AT24C02_DEBUG
#define AT24C02_DEBUG(...) DEBUG_LOG(...)
#else
#define AT24C02_DEBUG(...)
#endif

class AT24C02{
protected:
    I2cBus & bus_drv;
public:
    AT24C02(I2cBus & _bus_drv): bus_drv(_bus_drv){;}

    void writeData(const uint8_t & addr, const uint8_t * data, uint8_t len){
        if(addr + len > 256 || len > 8){
            AT24C02_DEBUG("AT24C02: writeData: invalid addr or len");
            len = MIN(256, addr + len) - arr;
        }
        bus_drv.writePool(addr, data, 1, len);
    }

    void writeData(const uint8_t & addr, const uint8_t & data){
        AT24C02_DEBUG("AT24C02 write: ", addr, data);
        writeData(addr, &data, 1);
    }

    void readData(const uint8_t & addr, uint8_t * data, uint8_t len){
        if(addr + len > 256 || len > 8){
            AT24C02_DEBUG("AT24C02: readData: invalid addr or len");
            len = MIN(256, addr + len) - arr;
        }
        bus_drv.readPool(addr, data, 1, len);
    }

    void readData(const uint8_t & addr, uint8_t * data){
        AT24C02_DEBUG("AT24C02 read: ", addr, data);
        readData(addr, data, 1);
    }
}
#endif
