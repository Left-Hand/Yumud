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

    static constexpr uint8_t default_id = 0b10110000;
protected:
    I2cDrv bus_drv;
    uint16_t buf;


    static constexpr uint8_t valid_chipid = 0x23;


    struct Mode1Reg{
        REG8_BEGIN
        uint8_t allcall:1;
        uint8_t sub:3;
        uint8_t sleep:1;
        uint8_t auto_inc:1;
        uint8_t extclk:1;
        uint8_t restart:1;
        REG8_END
    };

    struct Mode2Reg{
        REG8_BEGIN
        uint8_t outne:2;
        uint8_t outdrv:1;
        uint8_t och:1;
        uint8_t invrt:1;
        uint8_t __resv__:3;
        REG8_END
    };

    struct LedOnOffRegs{
        REG16_BEGIN
        uint16_t cvr:12;
        uint16_t full:1;
        uint16_t __resv__:3;
        REG16_END
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

    void write(const uint16_t & data) override{
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

    void setFrequency(uint32_t freq){
        static constexpr uint32_t osc_freq = 25000000;
        prescale = osc_freq / 4096 / freq - 1;
        writeReg(RegAddress::Prescale, prescale);
    }

    void setSubAddr(const uint8_t index, const uint8_t addr){
        sub_addr[index] = addr;
        writeReg(RegAddress(uint8_t(RegAddress::SubAddr) + index), sub_addr[index]);
    }


    // void init(){
    //     reset();
    //     delay(10);
    //     setLedCurrentLimit(CurrentLimit::Low);
    //     for(uint8_t i = 0; i< 16; i++){
    //         writeReg((RegAddress)((uint8_t)RegAddress::dim + i), (uint8_t)0);
    //     }
    //     ledMode = 0xffff;
    // }

    void init(){
        delay(10);
        DEBUG_PRINT("m1", readReg(RegAddress::Mode1));
        DEBUG_PRINT("m2", readReg(RegAddress::Mode2));
    }

    void reset(){
        // writeReg(RegAddress::swRst, (uint8_t)0x00);
    }

    void set(const Pin & pin) override{
        // buf |= (uint16_t)pin;
        // write(buf);
    }

    void clr(const Pin & pin) override{
        // buf &= ~(uint16_t)pin;
        // write(buf);
    }

    void setBits(const uint16_t & data) override{
        // buf |= data;
        // write(buf);
    }

    void clrBits(const uint16_t & data) override{
        // buf &= ~data;
        // write(buf);
    }

    void writeByIndex(const int8_t index, const bool data) override{
        // if(!isIndexValid(index))return;
        // if(data) buf |= 1 << index;
        // else buf &= ~(1 << index);
        // write(buf);
    }
    bool readByIndex(const int8_t index) override{
        // if(!isIndexValid(index)) return false;
        // read();
        // return (buf & (1 << index));
        return true;
    }

    void setModeByIndex(const int8_t & index, const PinMode & mode) override{
    //     if(!isIndexValid(index))return;
    //     uint16_t mask = 1 << index;
    //     if(PinModeUtils::isIn(mode)) dir |= mask;
    //     else dir &= ~mask;
    //     writeReg(RegAddress::dir, dir);

    //     if(index < 8){
    //         ctl.p0mod = PinModeUtils::isPP(mode);
    //         writeReg(RegAddress::ctl, ctl.data);
    //     }
    }

    // void enableIrqByIndex(const int8_t & index, const bool & en = true){
    //     if(!isIndexValid(index))return;
    //     writeReg(RegAddress::inten, (uint8_t)(en << index));
    // }

    // void enableLedMode(const Pin & pin, const bool & en = true){
    //     uint8_t index = CTZ((uint16_t)pin);
    //     if(en) ledMode &= ~(1 << index);
    //     else ledMode |= (1 << index);
    //     writeReg(RegAddress::ledMode, ledMode);
    // }

    // void setLedCurrentLimit(const CurrentLimit & limit){
    //     ctl.isel = (uint8_t)limit;
    //     writeReg(RegAddress::ctl, ctl.data);
    // }

    // void setLedCurrent(const Pin & pin, const uint8_t current){
    //     uint8_t index = CTZ((uint16_t)pin);
    //     if(index < 8) index += 4;
    //     else if(index < 12) index -= 8;
    //     writeReg((RegAddress)((uint8_t)RegAddress::dim + index), current);
    // }

    // bool isChipValid(){
    //     uint8_t chipId;
    //     readReg(RegAddress::chipId, chipId);
    //     return (chipId == valid_chipid);
    // }

    PCA9685 & operator << (const uint8_t & data){write(data); return *this;}
    PCA9685 & operator = (const uint16_t & data) override {write(data); return *this;}
};



#endif