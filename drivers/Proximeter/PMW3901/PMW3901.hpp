#pragma once

#include "drivers/device_defs.h"


namespace ymd::drivers{

class PMW3901 {
protected:
    SpiDrv spi_drv_;

    uint8_t isInited;

    uint8_t Motion;
    int16_t deltaX;
    int16_t deltaY;

    void writeByte(const uint8_t command, const uint8_t data);
    void readByte(const uint8_t command, uint8_t & data);
    void readBytes(const uint8_t command, uint8_t * data, const size_t len);
public:
    PMW3901(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PMW3901(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    PMW3901(Spi & spi, const uint8_t index):spi_drv_(SpiDrv(spi, index)){;}

    void init();
    void update();
};

}