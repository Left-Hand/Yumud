#ifndef __PCA9685B_HPP__

#define __PCA9685B_HPP__

#include "../hal/gpio/gpio.hpp"
#include "../hal/gpio/port_virtual.hpp"
#include "../drivers/CommonIO/Led/rgbLed.hpp"
#include "../hal/timer/pwm/pwm_channel.hpp"
#include "../drivers/device_defs.h"

class PCA9685: public PortVirtualConcept<16>{
public:
    enum class CurrentLimit{
        Max, High, Medium, Low
    };

    scexpr uint8_t default_id = 0b10110000;
protected:
    I2cDrv bus_drv;
    uint16_t buf;


    scexpr uint8_t valid_chipid = 0x23;


    struct Mode1Reg:public Reg8{
        uint8_t allcall:1;
        uint8_t sub:3;
        uint8_t sleep:1;
        uint8_t auto_inc:1;
        uint8_t extclk:1;
        uint8_t restart:1;
    };

    struct Mode2Reg:public Reg8{
        uint8_t outne:2;
        uint8_t outdrv:1;
        uint8_t och:1;
        uint8_t invrt:1;
        uint8_t __resv__:3;
    };

    struct LedOnOffRegs{
        uint16_t cvr:12;
        uint16_t full:1;
        uint16_t __resv__:3;
    };

    struct LedRegs{
        LedOnOffRegs on;
        LedOnOffRegs off;
    };

    enum class RegAddress:uint8_t{
        Mode1,
        Mode2,
        SubAddr = 0x02,
        Prescale = 0x7f
    };

    struct{
        Mode1Reg mode1;
        Mode2Reg mode2;
        uint8_t sub_addr[3];
        uint8_t all_addr;
        LedRegs sub_channels[16];
        LedRegs all_channel;
        uint8_t prescale;
    };

    void writeReg(const RegAddress addr, const uint8_t data){
        bus_drv.writeReg((uint8_t)addr, data);
    };

    void writeReg(const RegAddress addr, const uint16_t data){
        bus_drv.writeReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        bus_drv.readReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        bus_drv.readReg((uint8_t)addr, data);
    }

    uint8_t readReg(const RegAddress addr){
        uint8_t data;
        bus_drv.readReg((uint8_t)addr, data);
        return data;
    }

    void write(const uint16_t data) override{
        // buf = data;
        // writeReg(RegAddress::out, buf);
    }

    uint16_t read() override{
        // readReg(RegAddress::in, buf);
        // return buf;
        return true;
    }

public:
    PCA9685(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    PCA9685(I2cDrv && _bus_drv):bus_drv(_bus_drv){;}
    PCA9685(I2c & _bus):bus_drv{_bus, default_id}{;}

    void setFrequency(uint32_t freq);

    void setPwm(const uint8_t channel, const uint16_t on, const uint16_t off);

    void setSubAddr(const uint8_t index, const uint8_t addr){
        sub_addr[index] = addr;
        writeReg(RegAddress(uint8_t(RegAddress::SubAddr) + index), sub_addr[index]);
    }

    void init();

    void reset();

    void set(const Pin pin) override;

    void clr(const Pin pin) override;

    void set(const uint16_t data) override;

    void clr(const uint16_t data) override;
    
    void writeByIndex(const int index, const bool data) override;

    bool readByIndex(const int index) override;

    void setMode(const int index, const PinMode mode) override;

    PCA9685 & operator = (const uint16_t data) override {write(data); return *this;}
};



#endif