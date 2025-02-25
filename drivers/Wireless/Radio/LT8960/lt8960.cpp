#include "lt8960.hpp"
#include "sys/debug/debug.hpp"

#ifdef LT8960_DEBUG
#define LT8960_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define LT8960_DEBUG(...)
#endif

using namespace ymd;
using namespace ymd::drivers;


void LT8960::writeReg(const RegAddress address, const uint16_t reg){
    if(i2c_drv){
        i2c_drv->writeReg((uint8_t)address, reg, MSB);
    }else if(spi_drv){
        TODO("not implemented yet");
    }
    LT8960_DEBUG("write",*(uint16_t *)&reg, "at", (uint8_t)address);
}

void LT8960::readReg(const RegAddress address, uint16_t & reg){
    if(i2c_drv){
        i2c_drv->readReg((uint8_t)address, reg, MSB);
    }else if(spi_drv){
        TODO("not implemented yet");
    }
    LT8960_DEBUG("read",*(uint16_t *)&reg, "at", (uint8_t)address);
}

void LT8960::writeByte(const RegAddress address, const uint8_t data){
    if(i2c_drv){
        i2c_drv->writeReg((uint8_t)address, data);
    }else if(spi_drv){
        TODO("not implemented yet");
    }
}

void LT8960::readByte(const RegAddress address, uint8_t & data){
    if(spi_drv){
        spi_drv->writeSingle((uint8_t)((uint8_t)address & 0x80), CONT);
        delayT3();
        spi_drv->readSingle(data);
    }else if(i2c_drv){
        i2c_drv->readReg((uint8_t)address, data);
    }
}
