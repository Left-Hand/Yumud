#pragma once

#include "drivers/device_defs.h"
#include "types/image/image.hpp"

namespace ymd::drivers{

class PMW3901 {
protected:
    SpiDrv spi_drv_;

    uint8_t Motion;
    struct :public Reg16{uint16_t data;}deltaX;
    struct :public Reg16{uint16_t data;}deltaY;

    bool assertReg(const uint8_t command, const uint8_t data){
        uint8_t temp = 0;
        readReg(command, temp);
        return temp == data;
    }

    void writeReg(const uint8_t command, const uint8_t data);
    void readReg(const uint8_t command, uint8_t & data);
public:
    PMW3901(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PMW3901(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    PMW3901(Spi & spi, const uint8_t index):spi_drv_(SpiDrv(spi, index)){;}

    void init();
    void update();
    std::tuple<int16_t, int16_t> getMotion(){
        return {deltaX, deltaY};
    }

    void setLed(bool on);
    void readImage(ImageWritable<Grayscale> & img);
};

}