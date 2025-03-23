#pragma once

#include "core/io/regs.hpp"

#include "hal/gpio/vport.hpp"
#include "concept/pwm_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

class AW9523: public hal::VGpioPortIntf<16>{
public:
    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    scexpr uint8_t default_i2c_addr = 0b10110000;
protected:
    hal::I2cDrv i2c_drv_;
    uint16_t buf;
    scexpr uint8_t valid_chipid = 0x23;

    struct CtlReg:Reg8<>{
        uint8_t isel:2;
        uint8_t __resv1__:2;
        uint8_t p0mod:1;
        uint8_t __resv2__:3;
    };

    enum class RegAddress:uint8_t{
        in = 0x00,
        out = 0x02,
        dir = 0x04,
        inten = 0x06,
        chipId = 0x10,
        ctl = 0x11,
        ledMode = 0x12,
        dim = 0x20,
        swRst = 0x7f
    };

    uint16_t dir;
    uint16_t inten;
    CtlReg ctl;
    uint16_t ledMode;

    BusError write_reg(const RegAddress addr, const uint8_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data);
    };

    BusError write_reg(const RegAddress addr, const uint16_t data){
        return i2c_drv_.write_reg(uint8_t(addr), data, LSB);
    }

    BusError read_reg(const RegAddress addr, uint8_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data);
    }

    BusError read_reg(const RegAddress addr, uint16_t & data){
        return i2c_drv_.read_reg(uint8_t(addr), data, LSB);
    }

    void write_port(const uint16_t data) override{
        buf = data;
        write_reg(RegAddress::out, buf);
    }

    uint16_t read_port() override{
        read_reg(RegAddress::in, buf);
        return buf;
    }


    class AW9523Pwm:public hal::PwmIntf{
    protected:
        AW9523 & aw9523;
        hal::Pin pin;

        AW9523Pwm(AW9523 & _aw9523, const hal::Pin _pin):aw9523(_aw9523), pin(_pin){;}

        DELETE_COPY_AND_MOVE(AW9523Pwm)
        
        friend class AW9523;
    public:

        void init(){
            aw9523.enable_led_mode(pin);
        }

        AW9523Pwm & operator = (const real_t duty) override{
            aw9523.set_led_current(pin,int(255 * duty));
            return *this;
        }
    };

public:
    AW9523(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2c & bus):i2c_drv_(hal::I2cDrv(bus, default_i2c_addr)){;}

    void init();
    void reset(){
        write_reg(RegAddress::swRst, (uint8_t)0x00);
    }
    

    void set_pin(const uint16_t data) override{
        buf |= data;
        write_port(buf);
    }

    void clr_pin(const uint16_t data) override{
        buf &= ~data;
        write_port(buf);
    }

    void write_by_index(const int index, const bool data) override;
    
    bool read_by_index(const int index) override;

    void set_mode(const int index, const hal::GpioMode mode) override;

    void enable_irq_by_index(const int index, const bool en = true);

    void enable_led_mode(const hal::Pin pin, const bool en = true);

    void set_led_current_limit(const CurrentLimit limit);

    void set_led_current(const hal::Pin pin, const uint8_t current);
    
    bool verify();
    AW9523 & operator = (const uint16_t data) override {write_port(data); return *this;}

    AW9523Pwm operator [](const size_t index){
        return AW9523Pwm(*this, hal::Pin(1 << index));
    }
};

};