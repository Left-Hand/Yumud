//0x52

#ifndef __TCS34725_HPP__

#define __TCS34725_HPP__

#include "../bus/busdrv.hpp"

#ifndef TCS34725_DEBUG
#include "../bus/uart/uart1.hpp"
#define TCS34725_DEBUG(...) uart1.println(__VA_ARGS__)
#endif

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

#ifndef REG8_BEGIN
#define REG8_BEGIN union{struct{
#endif

#ifndef REG8_END
#define REG8_END };uint8_t data;};
#endif


class TCS34725{
public:
    enum class Gain:uint8_t{
        X1 = 0, X4, X16, X60 
    };
protected:
    I2cDrv & busdrv;

    struct Reg16{
    public:
        Reg16 & operator = (const uint16_t & _data){(uint16_t &)*this = _data; return * this;}
        explicit operator uint16_t() const {return *(uint16_t *)this;}
    };

    struct Reg8{
    public:
        Reg8 & operator = (const uint8_t & _data){(uint8_t &)*this = _data; return * this;}
        explicit operator uint8_t() const {return *(uint8_t *)this;}
    };

    struct EnableReg:public Reg8{
        REG8_BEGIN
        uint8_t powerOn : 1;
        uint8_t adcEn : 1;
        uint8_t __resv1__ :2;
        uint8_t waitEn : 1;
        uint8_t intEn : 1;
        uint8_t __resv2__ :3;
        REG8_END
    };

    struct IntegrationReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };
    
    struct WaitTimeReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct LowThrReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct HighThrReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct IntPersistenceReg:public Reg8{
        REG8_BEGIN
        uint8_t __resv__ :4;
        uint8_t apers   :4;
        REG8_END
    };

    struct LongWaitReg:public Reg8{
        REG8_BEGIN
        uint8_t __resv1__ :1;
        uint8_t waitLong : 1;
        uint8_t __resv2__ :6;
        REG8_END
    };

    struct GainReg:public Reg8{
        REG8_BEGIN
        uint8_t gain        :2;
        uint8_t __resv2__   :6;
        REG8_END
    };

    struct DeviceIdReg:public Reg8{
        REG8_BEGIN
        REG8_END
    };

    struct StatusReg:public Reg8{
        REG8_BEGIN
        uint8_t doneflag    :1;
        uint8_t __resv1__   :3;
        uint8_t intflag     :1;
        uint8_t __resv2__   :3;
        REG8_END
    };

    struct ClearDataReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct RedDataReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct GreenDataReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct BlueDataReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct{
        EnableReg enableReg;
        IntegrationReg integrationReg;
        WaitTimeReg waitTimeReg;
        LowThrReg lowThrReg;
        HighThrReg highThrReg;
        IntPersistenceReg intPersistenceReg;
        LongWaitReg longWaitReg;
        GainReg gainReg;
        DeviceIdReg deviceIdReg;
        StatusReg statusReg;
        ClearDataReg clearDataReg;
        RedDataReg redDataReg;
        GreenDataReg greenDataReg;
        BlueDataReg blueDataReg;
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

    uint8_t convRegAddress(const RegAddress regAddress, bool repeat = true){
        return ((uint8_t) regAddress) | 0x80 | (repeat ? 1 << 5 : 0);
    }

    void writeReg(const RegAddress & regAddress, const Reg16 & regData){
        busdrv.writeReg(convRegAddress(regAddress), (uint16_t)regData, false);
    }

    void readReg(const RegAddress & regAddress, Reg16 & regData){
        busdrv.readReg(convRegAddress(regAddress), (uint16_t &)regData, false);
    }

    void writeReg(const RegAddress & regAddress, const Reg8 & regData){
        busdrv.writeReg(convRegAddress(regAddress, false), (uint8_t)regData);
    }

    void readReg(const RegAddress & regAddress, Reg8 & regData){
        busdrv.readReg(convRegAddress(regAddress, false), (uint8_t &)regData);
    }

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        busdrv.readPool(convRegAddress(regAddress), data_ptr, 2, len, false);
    }

public:
    TCS34725(I2cDrv & _busdrv):busdrv(_busdrv){;}

    void setIntegration(const uint16_t & ms){
        uint16_t cycles = CLAMP(ms * 10 / 24, 1, 256);
        uint8_t temp = 256 - cycles;
        integrationReg.data = temp;
        writeReg(RegAddress::Integration, integrationReg);
    }

    void setWaitTime(const uint16_t & ms){
        uint16_t ms_l = MAX(ms * 10 / 24,1);
        uint16_t value;
        bool longwaitFlag = false;
        if(ms_l <= 256){
            value = 256 - ms_l;
        }else{
            uint16_t ms_h = CLAMP(ms * 10 / 24 / 12, 1, 256);
            value = 256 - ms_h;
            longwaitFlag = true;
        }

        waitTimeReg.data = value;
        writeReg(RegAddress::WaitTime, waitTimeReg);
        if(longwaitFlag){
            longWaitReg.waitLong = true;
            writeReg(RegAddress::LongWait, longWaitReg);
        }
    }

    void setIntThrLow(const uint16_t & thr){
        lowThrReg.data = thr;
        writeReg(RegAddress::LowThr, lowThrReg);
    }

    void setIntThrHigh(const uint16_t & thr){
        highThrReg.data = thr;
        writeReg(RegAddress::HighThr, highThrReg);
    }

    void setIntPersistence(const uint8_t & times){
        if(times >= 5){
            uint8_t value = 0b0100 + (times / 5) - 1;
            intPersistenceReg.data = value;
        }else{
            intPersistenceReg.data = (uint8_t)MIN(times, 3);
        }

        writeReg(RegAddress::Integration, integrationReg);
    }

    void setGain(const Gain & gain){
        gainReg.data = (uint8_t)gain;
        writeReg(RegAddress::Gain, gainReg);
    }

    void getId(){
        readReg(RegAddress::DeviceId, deviceIdReg);
    }

    bool isIdle(){
        readReg(RegAddress::Status, statusReg);
        return statusReg.doneflag;
    }

    void setPower(const bool & on){
        enableReg.powerOn = on;
        writeReg(RegAddress::Enable, enableReg);
    }

    void startConv(){
        enableReg.adcEn = true;
        writeReg(RegAddress::Enable, enableReg);
    }

    void getCRGB(uint16_t & c, uint16_t & r, uint16_t & g, uint16_t & b){
        uint16_t buf[4];
        requestRegData(RegAddress::ClearData, (uint8_t *)&buf, 8);

        c = buf[0];
        r = buf[1];
        g = buf[2];
        b = buf[3];
    }


    void init(){
        setPower(true);
        setIntegration(240);
        setGain(Gain::X1);
    }

};

#ifdef TCS34725_DEBUG
#undef TCS34725_DEBUG
#endif

#endif