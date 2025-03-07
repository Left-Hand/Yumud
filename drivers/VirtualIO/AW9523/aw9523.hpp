#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

#include "drivers/device_defs.h"


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

    void writeReg(const RegAddress addr, const uint8_t data){
        i2c_drv_.writeReg(uint8_t(addr), data);
    };

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv_.writeReg(uint8_t(addr), data, LSB);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        i2c_drv_.readReg(uint8_t(addr), data);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv_.readReg(uint8_t(addr), data, LSB);
    }

    void writePort(const uint16_t data) override{
        buf = data;
        writeReg(RegAddress::out, buf);
    }

    uint16_t readPort() override{
        readReg(RegAddress::in, buf);
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
            aw9523.enableLedMode(pin);
        }

        AW9523Pwm & operator = (const real_t duty) override{
            aw9523.setLedCurrent(pin,int(255 * duty));
            return *this;
        }
    };

public:
    AW9523(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    AW9523(hal::I2c & bus):i2c_drv_(hal::I2cDrv(bus, default_i2c_addr)){;}

    void init();
    void reset(){
        writeReg(RegAddress::swRst, (uint8_t)0x00);
    }
    

    void setPin(const uint16_t data) override{
        buf |= data;
        writePort(buf);
    }

    void clrPin(const uint16_t data) override{
        buf &= ~data;
        writePort(buf);
    }

    void writeByIndex(const int index, const bool data) override;
    
    bool readByIndex(const int index) override;

    void setMode(const int index, const hal::GpioMode mode) override;

    void enableIrqByIndex(const int index, const bool en = true);

    void enableLedMode(const hal::Pin pin, const bool en = true);

    void setLedCurrentLimit(const CurrentLimit limit);

    void setLedCurrent(const hal::Pin pin, const uint8_t current);
    
    bool verify();
    AW9523 & operator = (const uint16_t data) override {writePort(data); return *this;}

    AW9523Pwm operator [](const size_t index){
        return AW9523Pwm(*this, hal::Pin(1 << index));
    }
};

};