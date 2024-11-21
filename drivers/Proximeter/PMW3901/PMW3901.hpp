#pragma once

#include "drivers/device_defs.h"
#include "types/image/image.hpp"

namespace ymd::drivers{

namespace internal{
#pragma pack(push, 1)
struct PMW3901_Data {
uint8_t motion;
int16_t dx;
int16_t dy;
};
#pragma pack(pop)
}

class PMW3901:public internal::PMW3901_Data{
protected:
    SpiDrv spi_drv_;
    real_t x;
    real_t y;

    bool assertReg(const uint8_t command, const uint8_t data);
    void writeReg(const uint8_t command, const uint8_t data);
    void readReg(const uint8_t command, uint8_t & data);

    void updateData();
public:
    PMW3901(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PMW3901(SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    PMW3901(Spi & spi, const uint8_t index):spi_drv_(SpiDrv(spi, index)){;}

    bool verify();
    void init();
    void update();
    void update(const real_t rad);
    auto getMotion(){
        return std::make_tuple(dx, dy);
    }

    auto getPosition(){
        return std::make_tuple(x, y);
    }

    void setLed(bool on);
    void readImage(ImageWritable<Grayscale> & img);
};

}