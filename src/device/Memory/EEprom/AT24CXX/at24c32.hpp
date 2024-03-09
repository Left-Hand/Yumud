#ifndef __AT24C32_HPP__

#define __AT24C32_HPP__

#include "device/device_defs.h"

#ifdef AT24C32_DEBUG
#define AT24C32_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define AT24C32_DEBUG(...)
#endif

class AT24C32{
protected:
    I2cDrv & bus_drv;
    const uint32_t max_addr = 256;
    const uint32_t max_page = 8;
public:
    AT24C32(I2cDrv & _bus_drv): bus_drv(_bus_drv){;}

    void writeData(const uint8_t & addr, const uint8_t * data, uint8_t len){
        if(addr + len > max_addr || len > max_page){
            AT24C32_DEBUG("AT24C32: writeData: invalid addr or len");
            len = MIN(max_addr, addr + len) - addr;
        }
        bus_drv.write({(uint8_t)(addr >> 8), (uint8_t)(addr)}, false);
        bus_drv.write(data, len);
    }

    void writeData(const uint8_t & addr, const uint8_t & data){
        AT24C32_DEBUG("AT24C32 write: ", addr, data);
        writeData(addr, &data, 1);
    }

    void readData(const uint8_t & addr, uint8_t * data, uint8_t len){
        if(addr + len > max_addr || len > max_page){
            AT24C32_DEBUG("AT24C32: readData: invalid addr or len");
            len = MIN(max_addr, addr + len) - addr;
        }
        bus_drv.write({(uint8_t)(addr >> 8), (uint8_t)(addr)}, false);
        bus_drv.write(data, len);
    }

    void readData(const uint8_t & addr, uint8_t * data){
        AT24C32_DEBUG("AT24C32 read: ", addr, *data);
        readData(addr, data, 1);
    }
};

#endif
