#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"
#include "drivers/device_defs.h"


namespace ymd::drivers{

class TCS34725{
public:
    enum class Gain:uint8_t{
        X1 = 0, X4, X16, X60 
    };

protected:
    I2cDrv i2c_drv_;

    struct EnableReg:public Reg8{
        uint8_t powerOn : 1;
        uint8_t adcEn : 1;
        uint8_t __resv1__ :2;
        uint8_t waitEn : 1;
        uint8_t intEn : 1;
        uint8_t __resv2__ :3;
    };

    struct IntPersistenceReg:public Reg8{
        using Reg8::operator=;
        uint8_t __resv__ :4;
        uint8_t apers   :4;
    };

    struct LongWaitReg:public Reg8{
        uint8_t __resv1__ :1;
        uint8_t waitLong : 1;
        uint8_t __resv2__ :6;
    };

    struct GainReg:public Reg8{
        using Reg8::operator=;
        uint8_t gain        :2;
        uint8_t __resv2__   :6;
    };

    struct StatusReg:public Reg8{
        uint8_t done_flag    :1;
        uint8_t __resv1__   :3;
        uint8_t interrupt_flag     :1;
        uint8_t __resv2__   :3;
    };


    struct{
        EnableReg enableReg;
        uint8_t integrationReg;
        uint8_t waitTimeReg;
        uint16_t lowThrReg;
        uint16_t highThrReg;
        IntPersistenceReg intPersistenceReg;
        LongWaitReg longWaitReg;
        GainReg gainReg;
        uint8_t deviceIdReg;
        StatusReg statusReg;
        uint16_t crgb[4] = {0};
    };

    enum class RegAddress:uint8_t{
        Enable = 0x00,
        Integration = 0x01,
        WaitTime = 0x03,
        LowThr = 0x04,
        HighThr = 0x06,
        IntPersistence = 0x0C,
        LongWait = 0x0D,
        Gain = 0x0F,
        DeviceId = 0x12,
        Status = 0x13,
        ClearData = 0x14,
        RedData = 0x16,
        GreenData = 0x18,
        BlueData = 0x1A
    };

    uint8_t convRegAddress(const RegAddress addr, bool repeat = true){
        return ((uint8_t) addr) | 0x80 | (repeat ? 1 << 5 : 0);
    }

    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv_.writeReg(convRegAddress(addr), (uint16_t)data, LSB);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv_.readReg(convRegAddress(addr), (uint16_t &)data, LSB);
    }

    void writeReg(const RegAddress addr, const uint8_t data){
        i2c_drv_.writeReg(convRegAddress(addr, false), (uint8_t)data, LSB);
    }

    void readReg(const RegAddress addr, uint8_t & data){
        i2c_drv_.readReg(convRegAddress(addr, false), (uint8_t &)data, LSB);
    }

    void requestRegData(const RegAddress addr, uint16_t * data_ptr, const size_t len);

public:
    scexpr uint8_t default_i2c_addr = 0x52;

    TCS34725(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    TCS34725(I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    TCS34725(I2c & bus, const uint8_t addr = default_i2c_addr):i2c_drv_(bus, addr){;}

    void setIntegration(const uint16_t ms){
        uint16_t cycles = CLAMP(ms * 10 / 24, 1, 256);
        uint8_t temp = 256 - cycles;
        integrationReg = temp;
        writeReg(RegAddress::Integration, integrationReg);
    }

    void setWaitTime(const uint16_t ms){
        uint16_t ms_l = MAX(ms * 10 / 24,1);
        uint16_t value;
        bool long_waitFlag = false;
        if(ms_l <= 256){
            value = 256 - ms_l;
        }else{
            uint16_t ms_h = CLAMP(ms * 10 / 24 / 12, 1, 256);
            value = 256 - ms_h;
            long_waitFlag = true;
        }

        waitTimeReg = value;
        writeReg(RegAddress::WaitTime, waitTimeReg);
        if(long_waitFlag){
            longWaitReg.waitLong = true;
            writeReg(RegAddress::LongWait, longWaitReg);
        }
    }

    void setIntThrLow(const uint16_t thr){
        lowThrReg = thr;
        writeReg(RegAddress::LowThr, lowThrReg);
    }

    void setIntThrHigh(const uint16_t thr){
        highThrReg = thr;
        writeReg(RegAddress::HighThr, highThrReg);
    }

    void setIntPersistence(const uint8_t times);

    void setGain(const Gain gain){
        gainReg = (uint8_t)gain;
        writeReg(RegAddress::Gain, gainReg);
    }

    uint8_t getId(){
        readReg(RegAddress::DeviceId, deviceIdReg);
        return deviceIdReg;
    }

    bool isIdle(){
        readReg(RegAddress::Status, statusReg);
        return statusReg.done_flag;
    }

    void setPower(const bool on){
        enableReg.powerOn = on;
        writeReg(RegAddress::Enable, enableReg);
    }

    void startConv(){
        enableReg.adcEn = true;
        writeReg(RegAddress::Enable, enableReg);
    }

    void update();

    std::tuple<real_t, real_t, real_t, real_t> getCRGB();

    void init(){
        setPower(true);
        setIntegration(240);
        setGain(Gain::X1);
    }
};

};