#pragma once

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

#include "drivers/device_defs.h"


namespace yumud::drivers{

class AW9523: public PortVirtualConcept<16>{
public:
    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    scexpr uint8_t default_i2c_addr = 0b10110000;
protected:
    I2cDrv bus_drv;
    uint16_t buf;
    scexpr uint8_t valid_chipid = 0x23;

    struct CtlReg:Reg8{
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

    struct{
        uint16_t dir;
        uint16_t inten;
        CtlReg ctl;
        uint16_t ledMode;
    };

    void writeReg(const RegAddress addr, const uint8_t data){
        bus_drv.writeReg((uint8_t)addr, data);
    };

    void writeReg(const RegAddress addr, const uint16_t data){
        bus_drv.writeReg((uint8_t)addr, data, LSB);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        bus_drv.readReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        bus_drv.readReg((uint8_t)addr, data, LSB);
    }

    void write(const uint16_t data) override{
        buf = data;
        writeReg(RegAddress::out, buf);
    }

    uint16_t read() override{
        readReg(RegAddress::in, buf);
        return buf;
    }


    class AW9523Pwm:public PwmChannel{
    protected:
        AW9523 & aw9523;
        Pin pin;

        AW9523Pwm(AW9523 & _aw9523, const Pin _pin):aw9523(_aw9523), pin(_pin){;}

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

    std::array<AW9523Pwm, 16> channels = {
        AW9523Pwm(*this, Pin::_0),
        AW9523Pwm(*this, Pin::_1),
        AW9523Pwm(*this, Pin::_2),
        AW9523Pwm(*this, Pin::_3),
        AW9523Pwm(*this, Pin::_4),
        AW9523Pwm(*this, Pin::_5),
        AW9523Pwm(*this, Pin::_6),
        AW9523Pwm(*this, Pin::_7),
        AW9523Pwm(*this, Pin::_8),
        AW9523Pwm(*this, Pin::_9),
        AW9523Pwm(*this, Pin::_10),
        AW9523Pwm(*this, Pin::_11),
        AW9523Pwm(*this, Pin::_12),
        AW9523Pwm(*this, Pin::_13),
        AW9523Pwm(*this, Pin::_14),
        AW9523Pwm(*this, Pin::_15),
    };

public:
    AW9523(const I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    AW9523(I2cDrv && _bus_drv):bus_drv(_bus_drv){;}
    AW9523(I2c & bus):bus_drv(I2cDrv(bus, default_i2c_addr)){;}

    void init(){
        reset();
        delay(10);
        setLedCurrentLimit(CurrentLimit::Low);
        for(uint8_t i = 0; i< 16; i++){
            writeReg((RegAddress)((uint8_t)RegAddress::dim + i), (uint8_t)0);
        }
        ledMode = 0xffff;
    }

    void reset(){
        writeReg(RegAddress::swRst, (uint8_t)0x00);
    }
    
    void set(const Pin pin) override{
        buf |= (uint16_t)pin;
        write(buf);
    }
    
    void clr(const Pin pin) override{
        buf &= ~(uint16_t)pin;
        write(buf);
    }

    void set(const uint16_t data) override{
        buf |= data;
        write(buf);
    }

    void clr(const uint16_t data) override{
        buf &= ~data;
        write(buf);
    }

    void writeByIndex(const int index, const bool data) override{
        if(!isIndexValid(index))return;
        if(data) buf |= 1 << index;
        else buf &= ~(1 << index);
        write(buf);
    }
    
    bool readByIndex(const int index) override{
        if(!isIndexValid(index)) return false;
        read();
        return (buf & (1 << index));
    }

    void setMode(const int index, const PinMode mode) override{
        if(!isIndexValid(index))return;
        uint16_t mask = 1 << index;
        if(PinModeUtils::isIn(mode)) dir |= mask;
        else dir &= ~mask;
        writeReg(RegAddress::dir, dir);

        if(index < 8){
            ctl.p0mod = PinModeUtils::isPP(mode);
            writeReg(RegAddress::ctl, ctl);
        }
    }

    void enableIrqByIndex(const int index, const bool en = true){
        if(!isIndexValid(index))return;
        writeReg(RegAddress::inten, (uint8_t)(en << index));
    }

    void enableLedMode(const Pin pin, const bool en = true){
        uint index = CTZ((uint16_t)pin);
        if(en) ledMode &= ~(1 << index);
        else ledMode |= (1 << index);
        writeReg(RegAddress::ledMode, ledMode);
    }

    void setLedCurrentLimit(const CurrentLimit limit);

    void setLedCurrent(const Pin pin, const uint8_t current);
    
    bool verify(){
        uint8_t chipId;
        readReg(RegAddress::chipId, chipId);
        return (chipId == valid_chipid);
    }

    // AW9523 & operator << (const uint8_t data){write(data); return *this;}
    
    AW9523 & operator = (const uint16_t data) override {write(data); return *this;}

    AW9523Pwm & operator [](const size_t index){
        return channels[index];
    }
};

};