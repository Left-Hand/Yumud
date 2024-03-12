#ifndef __AW9523B_HPP__

#define __AW9523B_HPP__

#include "src/gpio/gpio.hpp"
#include "src/gpio/port_virtual.hpp"
#include "device_defs.h"

class AW9523: public PortVirtual<16>{
public:
    enum class CurrentLimit{
        Low, Medium, High, Max
    };

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
    }

    struct{
        uint16_t dir;
        uint16_t inten;
    //     uint8_t chipId;
        CtlReg ctl;
        uint16_t ledMode;
    //     uint8_t dim[16];
    //     uint8_t swRst;
    };

    void writeReg(const RegAddress & addr, const uint8_t & data){
        bus_drv.write((uint8_t)addr, data);
    };

    void writeReg(const RegAddress & addr, const uint16_t & data){
        bus_drv.writeReg((uint8_t)addr, data);
    }

    void readReg(const RegAddress & addr, uint8_t & data){
        bus_drv.read((uint8_t)addr, data);
    }

    void readReg(const RegAddress & addr, uint16_t & data){
        bus_drv.readReg((uint8_t)addr, data);
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
        buf |= (data << index);
        write(buf);
    }
    bool readByIndex(const int8_t index) override{
        if(!isIndexValid(index))return;
        read();
        return (buf & (1 << index));
    }

    void setModeByIndex(const int8_t & index, const PinMode & mode){
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

    void enableIrqByIndex(const int8_t & index, const bool & en = true){
        if(!isIndexValid(index))return;
        writeReg(RegAddress::inten, en << index);
    }

    void enableLedModeByIndex(const int8_t & index, const bool & en = true){
        if(!isIndexValid(index) || (index >= 8))return;
        writeReg(RegAddress::ledMode, (!en) << index);
    }

    void setLedCurrentLimit(const CurrentLimit & limit){
        ctl.isel = (uint8_t)limit;
        writeReg(RegAddress::ctl, ctl);
    }

    void setLedCurrent(const Pin & pin, const uint8_t current){
        uint8_t index = CTZ((uint8_t)pin);
        if(index < 8) index += 4;
        else if(index < 12) index -= 8;
        writeByIndex((RegAddress)((uint8_t)RegAddress::dim + index), current);
    }

    bool isChipValid(){
        uint8_t chipId;
        readReg(RegAddress::chipId, chipId);
        return (chipId == valid_chipid);
    }

    AW9523 & operator << (const uint8_t & data){write(data); return *this;}
    AW9523 & operator = (const uint16_t & data) override {write(data); return *this;}


};
#endif