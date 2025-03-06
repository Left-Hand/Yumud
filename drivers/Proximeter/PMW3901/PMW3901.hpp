#pragma once

#include "drivers/device_defs.h"
#include "drivers/Proximeter/FlowSensor.hpp"
#include "types/image/image.hpp"

namespace ymd::drivers{

namespace internal{
#pragma pack(push, 1)

struct MotionReg:public Reg8<>{
    using Reg8::operator=;

    uint8_t frameFrom0:1;
    uint8_t runMode:2;
    const uint8_t __resv1__:1 = 0;
    uint8_t rawFrom0:1;
    uint8_t __resv2__:2;
    uint8_t occured:1;
};

struct DeltaReg:public Reg16<>{
    using Reg16::operator =;

    uint16_t :16;
};

struct PMW3901_Data {
MotionReg motion;
uint8_t observation;
DeltaReg dx;
DeltaReg dy;
};
#pragma pack(pop)
}

class PMW3901:public internal::PMW3901_Data, public FlowSensor{
protected:
    hal::SpiDrv spi_drv_;
    real_t x_cm;
    real_t y_cm;

    bool assertReg(const uint8_t command, const uint8_t data);
    void writeReg(const uint8_t command, const uint8_t data);
    void readReg(const uint8_t command, uint8_t & data);
    void readMulti(const uint8_t commnad, uint8_t * data, const size_t len);

    void readDataSlow();
    void readDataBurst();
    void readData();
public:
    PMW3901(const PMW3901 & other) = delete;
    PMW3901(PMW3901 && other) = delete;

    PMW3901(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PMW3901(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    PMW3901(hal::Spi & spi, const uint8_t index):spi_drv_(hal::SpiDrv(spi, index)){;}

    bool verify();
    void init();
    void update();
    void update(const real_t rad);
    // auto getMotion(){
    //     return std::make_tuple(int16_t(dx), int16_t(dy));
    // }

    std::tuple<real_t, real_t> getPosition() override{
        return std::make_tuple(x_cm * real_t(0.01), y_cm * real_t(0.01));
    }

    void setLed(bool on);
    void readImage(ImageWritable<Grayscale> & img);
};

}