#pragma once
#include "core/io/regs.hpp"
#include "hal/gpio/port_virtual.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{ 
class NCA9555: public hal::VGpioPortIntf<16>{
protected:
    hal::I2cDrv i2c_drv_;

    scexpr uint8_t default_i2c_addr = 0b01000000;

    using RegAddress = uint8_t;

    struct InputReg:public Reg16<>{
        scexpr RegAddress address = 0x00;
        uint16_t :16;
    };

    struct OutputReg:public Reg16<>{
        using Reg16::operator =;
        scexpr RegAddress address = 0x02;
        uint16_t :16;
    };

    struct InversionReg:public Reg16<>{
        using Reg16::operator =;
        scexpr RegAddress address = 0x04;
        uint16_t :16;
    };

    struct ConfigReg:public Reg16<>{
        scexpr RegAddress address = 0x06;
        uint16_t :16;
    };

    InputReg input_reg;
    OutputReg output_reg;
    InversionReg inversion_reg;
    ConfigReg config_reg;


    BusError write_reg(const uint8_t addr, const uint16_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data, LSB);
    }

    BusError read_reg(const uint8_t addr, uint16_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data, LSB);
    }

public:

    NCA9555(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    NCA9555(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    NCA9555(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(hal::I2cDrv{i2c, default_i2c_addr}){;}

    void init();
    void setInversion(const uint16_t mask);
    void writePort(const uint16_t data) override;
    uint16_t readPort() override;
    void setMode(const int index, const hal::GpioMode mode)override;
};
}