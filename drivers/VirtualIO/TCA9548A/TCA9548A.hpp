#pragma once

#include "core/io/regs.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

// #include "hal/gpio/gpio_port.hpp"

namespace ymd::drivers{



class TCA9548A{
public:
class VirtualI2c: public hal::I2c{
    protected:
        TCA9548A & host_;
        const uint8_t ch_;
        BusError lead(const uint8_t address){return host_.lead(address, ch_);}
        void trail(){return host_.trail(ch_);}
    public:
        VirtualI2c(TCA9548A & host, const uint8_t ch);

        BusError write(const uint32_t data) override final {return host_.write(data);}
        BusError read(uint32_t & data, const Ack ack) override final {return host_.read(data, ack);}

        void set_baudrate(const uint32_t baud) override final{return host_.setBaudRate(baud);}
    };

protected:
    hal::I2c & i2c_;
    hal::I2cDrv self_i2c_drv_;
    uint8_t last_ch_ = -1;

    void switch_vbus(const uint8_t ch);

    BusError lead(const uint8_t address, const uint8_t ch);

    void trail(const uint8_t ch);

    void setBaudRate(const uint32_t baud){i2c_.set_baudrate(baud);}

    BusError write(const uint32_t data){
        return i2c_.write(data);
    }

    BusError read(uint32_t & data, const Ack ack){
        return i2c_.read(data, ack);
    }


    friend class VirtualI2c;

    std::array<VirtualI2c, 8> v_i2cs_{
        VirtualI2c(*this, 0),
        VirtualI2c(*this, 1),
        VirtualI2c(*this, 2),
        VirtualI2c(*this, 3),
        VirtualI2c(*this, 4),
        VirtualI2c(*this, 5),
        VirtualI2c(*this, 6),
        VirtualI2c(*this, 7),
    };
public:
    TCA9548A(const hal::I2cDrv & i2c_drv):
        i2c_(i2c_drv.bus()), self_i2c_drv_(i2c_drv){;}
    TCA9548A(hal::I2cDrv && i2c_drv):
        i2c_(i2c_drv.bus()), self_i2c_drv_(std::move(i2c_drv)){;}
    TCA9548A(hal::I2c & i2c, const uint8_t addr):
        i2c_(i2c), self_i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    auto & operator [](const size_t ch){
        if(unlikely(ch >= 8)) while(true);
        return v_i2cs_[ch];
    }

    auto which() const {return last_ch_;}

    BusError verify() {
        return self_i2c_drv_.verify();
    }
};

}