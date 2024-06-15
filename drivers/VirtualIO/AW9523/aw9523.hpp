#ifndef __AW9523B_HPP__

#define __AW9523B_HPP__

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "drivers/device_defs.h"

class AW9523: public PortVirtualConcept<16>{
public:
    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    static constexpr uint8_t default_id = 0b10110000;
protected:
    I2cDrv & bus_drv;
    uint16_t buf;
    static constexpr uint8_t valid_chipid = 0x23;

    struct CtlReg{
        REG8_BEGIN
        uint8_t isel:2;
        uint8_t __resv1__:2;
        uint8_t p0mod:1;
        uint8_t __resv2__:3;
        REG8_END
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

    void writeReg(const RegAddress & addr, const uint8_t & data){
        bus_drv.writeReg((uint8_t)addr, data);
    };

    void writeReg(const RegAddress & addr, const uint16_t & data){
        bus_drv.writeReg((uint8_t)addr, data, false);
    }

    void readReg(const RegAddress & addr, uint8_t & data){
        bus_drv.readReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress & addr, uint16_t & data){
        bus_drv.readReg((uint8_t)addr, data, false);
    }

    void write(const uint16_t & data) override{
        buf = data;
        writeReg(RegAddress::out, buf);
    }

    uint16_t read() override{
        readReg(RegAddress::in, buf);
        return buf;
    }

public:
    AW9523(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}

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
    void set(const Pin & pin) override{
        buf |= (uint16_t)pin;
        write(buf);
    }
    void clr(const Pin & pin) override{
        buf &= ~(uint16_t)pin;
        write(buf);
    }

    void setBits(const uint16_t & data) override{
        buf |= data;
        write(buf);
    }

    void clrBits(const uint16_t & data) override{
        buf &= ~data;
        write(buf);
    }

    void writeByIndex(const int8_t index, const bool data) override{
        if(!isIndexValid(index))return;
        if(data) buf |= 1 << index;
        else buf &= ~(1 << index);
        write(buf);
    }
    bool readByIndex(const int8_t index) override{
        if(!isIndexValid(index)) return false;
        read();
        return (buf & (1 << index));
    }

    void setModeByIndex(const int8_t & index, const PinMode & mode) override{
        if(!isIndexValid(index))return;
        uint16_t mask = 1 << index;
        if(PinModeUtils::isIn(mode)) dir |= mask;
        else dir &= ~mask;
        writeReg(RegAddress::dir, dir);

        if(index < 8){
            ctl.p0mod = PinModeUtils::isPP(mode);
            writeReg(RegAddress::ctl, ctl.data);
        }
    }

    void enableIrqByIndex(const int8_t & index, const bool & en = true){
        if(!isIndexValid(index))return;
        writeReg(RegAddress::inten, (uint8_t)(en << index));
    }

    void enableLedMode(const Pin & pin, const bool & en = true){
        uint8_t index = CTZ((uint16_t)pin);
        if(en) ledMode &= ~(1 << index);
        else ledMode |= (1 << index);
        writeReg(RegAddress::ledMode, ledMode);
    }

    void setLedCurrentLimit(const CurrentLimit & limit){
        ctl.isel = (uint8_t)limit;
        writeReg(RegAddress::ctl, ctl.data);
    }

    void setLedCurrent(const Pin & pin, const uint8_t current){
        uint8_t index = CTZ((uint16_t)pin);
        if(index < 8) index += 4;
        else if(index < 12) index -= 8;
        writeReg((RegAddress)((uint8_t)RegAddress::dim + index), current);
    }

    bool isChipValid(){
        uint8_t chipId;
        readReg(RegAddress::chipId, chipId);
        return (chipId == valid_chipid);
    }

    AW9523 & operator << (const uint8_t & data){write(data); return *this;}
    AW9523 & operator = (const uint16_t & data) override {write(data); return *this;}


};

class AW9523Pwm:public PwmChannel{
protected:
    AW9523 & aw9523;
    Pin pin;
public:
    AW9523Pwm(AW9523 & _aw9523, const Pin & _pin):aw9523(_aw9523), pin(_pin){;}

    void init() override{
        aw9523.enableLedMode(pin);
    }

    AW9523Pwm & operator = (const real_t & duty) override{
        aw9523.setLedCurrent(pin,int(255 * duty));
        return *this;
    }
};

class AW9523RgbLed :public RgbLedConcept<true>{
protected:
    using Color = Color_t<real_t>;

    AW9523 & aw;
    Pin pin_r;
    Pin pin_g;
    Pin pin_b;

    void _update(const Color &color){
        aw.setLedCurrent(pin_r,int(255 * color.r));
        aw.setLedCurrent(pin_g,int(255 * color.g));
        aw.setLedCurrent(pin_b,int(255 * color.b));
    }
public:
    AW9523RgbLed(AW9523 & _aw, const Pin & _pin_r, const Pin & _pin_g, const Pin & _pin_b):
        aw(_aw), pin_r(_pin_r), pin_g(_pin_g), pin_b(_pin_b) {;}

    void init() override{
        aw.enableLedMode(pin_r);
        aw.enableLedMode(pin_g);
        aw.enableLedMode(pin_b);
    }

    AW9523RgbLed & operator = (const Color &color){
        setColor(color);
        return *this;
    }
};



#endif