#pragma once

#include "core/io/regs.hpp"
#include "drivers/Proximeter/FlowSensor.hpp"
#include "types/image/image.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

namespace details{
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
MotionReg motion = {};
uint8_t observation = {};
DeltaReg dx = {};
DeltaReg dy = {};
};
#pragma pack(pop)
}

class PMW3901:public details::PMW3901_Data, public FlowSensorIntf{
protected:
    enum class Error{
        
    };

    hal::SpiDrv spi_drv_;
    real_t x_cm = {};
    real_t y_cm = {};

    bool assert_reg(const uint8_t command, const uint8_t data);
    BusError write_reg(const uint8_t command, const uint8_t data);
    BusError read_reg(const uint8_t command, uint8_t & data);
    BusError read_burst(const uint8_t commnad, uint8_t * data, const size_t len);

    void read_data_slow();
    void read_data_burst();
    void read_data();
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

    Vector2_t<real_t> get_position(){
        return {x_cm * real_t(0.01), y_cm * real_t(0.01)};
    }

    void set_led(bool on);
    void read_image(ImageWritable<Grayscale> & img);
};

}