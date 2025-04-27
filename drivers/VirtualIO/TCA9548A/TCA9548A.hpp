

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Option.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{



class TCA9548A{
public:
class VirtualI2c final: public hal::I2c{
    protected:
        TCA9548A & host_;
        const uint8_t ch_;
        hal::BusError lead(const hal::LockRequest req){return host_.lead(req.id(), ch_);}
        void trail(){return host_.trail(ch_);}
    public:
        VirtualI2c(TCA9548A & host, const uint8_t ch);

        hal::BusError write(const uint32_t data) {return host_.write(data);}
        hal::BusError read(uint32_t & data, const Ack ack) {return host_.read(data, ack);}
        hal::BusError unlock_bus() {return host_.unlock_bus();}
        hal::BusError set_baudrate(const uint32_t baud){return host_.set_baudrate(baud);}
    };

protected:
    hal::I2c & i2c_;
    hal::I2cDrv self_i2c_drv_;
    Option<uint8_t> last_ch_ = None;

    hal::BusError switch_vbus(const uint8_t ch);
    hal::BusError unlock_bus(){return i2c_.unlock_bus();}

    hal::BusError lead(const uint8_t address, const uint8_t ch);

    void trail(const uint8_t ch);

    hal::BusError set_baudrate(const uint32_t baud){return i2c_.set_baudrate(baud);}

    hal::BusError write(const uint32_t data){
        return i2c_.write(data);
    }

    hal::BusError read(uint32_t & data, const Ack ack){
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
    TCA9548A(hal::I2cDrv && i2c_drv):
        i2c_(i2c_drv.bus()), self_i2c_drv_(std::move(i2c_drv)){;}
    TCA9548A(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_(i2c), self_i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    auto & operator [](const size_t ch){
        if(unlikely(ch >= 8)) while(true);
        return v_i2cs_[ch];
    }

    auto which() const {return last_ch_;}

    hal::BusError verify() {
        return self_i2c_drv_.verify();
    }
};

}