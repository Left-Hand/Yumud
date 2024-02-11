#ifndef __LT8920_HPP__

#define __LT8920_HPP__

#include "../bus/bus_inc.h"
#include "../types/real.hpp"

#ifndef LT8920_DEBUG
#include "../bus/uart/uart1.hpp"
#define LT8920_DEBUG(...) uart1.println(__VA_ARGS__)
#endif

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

class LT8920{
public:
    enum class PacketType:uint8_t{
        NRZLaw = 0,Manchester,Line8_10,Interleave
    };

    enum class TrailerLen:uint8_t{
        b4 = 0,b6,b8,b10,b12,b14,b16,b18
    };

    enum class SyncWordLen:uint8_t{
        b16, b32, b48, b64
    };

    enum class PreambleLen:uint8_t{
        b1 = 0, b2, b3, b4, b5, b6, b7, b8
    };

    enum class BrclkSel:uint8_t{
        Low = 0,Div1, Div2, Div4, Div8,
        Mhz1, Mhz12
    };

    enum class DataRate:uint8_t{
        Mbps1 = 0, Kbps250, Kbps125, Kbps62_5
    };


protected:
    SpiDrv & bus_drv;
    struct Reg16{};

    struct RfSynthLockReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv1__ :12;
        uint16_t synthLock:1;
        uint16_t __resv2__ :3;
        REG16_END
    };

    struct RawRssiReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv__ :10;
        uint16_t rawRssi:6;
        REG16_END
    };

    struct RfConfigReg:public Reg16{
        REG16_BEGIN
        uint16_t rfChannelNo :6;
        uint16_t rxEn:1;
        uint16_t txEn:1;
        uint16_t __resv__ :8;
        REG16_END
    };

    struct PaConfigReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv1__ :7;
        uint16_t paGain:4;
        uint16_t __resv2__ :1;
        uint16_t paCurrent:4;
        REG16_END
    };

    struct OscEnableReg:public Reg16{
        REG16_BEGIN
        uint16_t en :1;
        uint16_t __resv__ :15;
        REG16_END
    };

    struct RssiPdnReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv1__ :8;
        uint16_t rssiPdn:1;
        uint16_t __resv2__ :7;
        REG16_END
    };

    struct AutoCaliReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv__ :2;
        uint16_t autoCali:1;
        uint16_t __resv1__ :14;
        REG16_END
    };

    struct DeviceIDReg:public Reg16{
        REG16_BEGIN
        uint16_t digiVersion:3;
        uint16_t __resv1__ :1;
        uint16_t rfVersion:4;
        uint16_t __resv2__ :8;
        REG16_END
    };

    struct Config1Reg:public Reg16{
        REG16_BEGIN
        uint16_t __resv1__ :1;
        uint16_t clkSel:3;
        uint16_t __resv2__ :2;
        uint16_t packetType:2;
        uint16_t trailerLen:3;
        uint16_t syncWordLen :2;
        uint16_t preambleLen :2;
        REG16_END
    };

    struct Delay1Reg:public Reg16{
        REG16_BEGIN
        uint16_t txPaOnDelays:6;
        uint16_t txPaOffDelays:2;
        uint16_t vcoOnDelays:8;
        REG16_END
    };

    struct Delay2Reg:public Reg16{
        REG16_BEGIN
        uint16_t txSwOnDelays:6;
        uint16_t __resv__ :2;
        uint16_t rxCwDelays:7;
        uint16_t bpktDirect:1;
        REG16_END
    };

    struct Config2Reg:public Reg16{
        REG16_BEGIN
        uint16_t scramableData :7;
        uint16_t misoTri :1;
        uint16_t retransTimes:4;
        uint16_t brclkOnSleep:1;
        uint16_t __resv__ :1;
        uint16_t sleepMode:1;
        uint16_t powerDown:1;
        REG16_END
    };

    struct SyncWord0Reg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct SyncWord1Reg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct SyncWord2Reg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct SyncWord3Reg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct ThresholdReg:public Reg16{
        REG16_BEGIN
        uint16_t syncWordThreshold:6;
        uint16_t fifoFullThreshold:4;
        uint16_t fifoEmptyThreshold:4;
        REG16_END
    };

    struct Config3Reg:public Reg16{
        REG16_BEGIN
        uint16_t crcInitalData:8;
        uint16_t __resv1__ :2;
        uint16_t pktFifoPolarity:1;
        uint16_t autoAck:1;
        uint16_t fwTermTx :1;
        uint16_t packLengthEN:1;
        uint16_t __resv2__ :1;
        uint16_t crcOn:1;
    };

    struct RxConfigReg:public Reg16{
        REG16_BEGIN
        uint16_t rxAckTime:8;
        uint16_t __resv__ :2;
        uint16_t scanRssiChNo:6;
        REG16_END
    };

    struct RssiConfigReg:public Reg16{
        REG16_BEGIN
        uint16_t waitRssiScanTime :8;
        uint16_t rssiStartChOffset:7;
        uint16_t scanRssiEn :1;
        REG16_END
    };

    struct DataRateReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv__ :8;
        uint16_t dataRate:8;
        REG16_END
    };

    struct FlagReg:public Reg16{
        REG16_BEGIN
        uint16_t __resv__ :5;
        uint16_t fifoFlag:1;
        uint16_t pktFlag:1;
        uint16_t syncWordRecv:1;
        uint16_t framerFlag:6;
        uint16_t fec23ErrorFlag:1;
        uint16_t crcErrorFlag:1;
        REG16_END
    };

    struct FifoReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct FifoPtrReg:public Reg16{
        REG16_BEGIN
        uint16_t fifoReadPtr:6;
        uint16_t __resv1__:1;
        uint16_t clearReadPtr:1;
        uint16_t fifoWritePtr:6;
        uint16_t __resv2__:1;
        uint16_t clearWritePtr:1;
        REG16_END
    };

    struct{
        RfSynthLockReg rfSynthLockReg;
        RawRssiReg rawRssiReg;
        RfConfigReg rfConfigReg;
        PaConfigReg paConfigReg;
        OscEnableReg oscEnableReg;
        RssiPdnReg rssiPdnReg;
        AutoCaliReg autoCaliReg;
        DeviceIDReg deviceIDReg;
        Config1Reg config1Reg;
        Delay1Reg delay1Reg;
        Delay2Reg delay2Reg;
        Config2Reg config2Reg;
        SyncWord0Reg syncWord0Reg;
        SyncWord1Reg syncWord1Reg;
        SyncWord2Reg syncWord2Reg;
        SyncWord3Reg syncWord3Reg;
        ThresholdReg thresholdReg;
        Config3Reg config3Reg;
        RxConfigReg rxConfigReg;
        RssiConfigReg rssiConfigReg;
        DataRateReg dataRateReg;
        FlagReg flagReg;
        FifoReg fifoReg;
        FifoPtrReg fifoPtrReg;
    };

    enum class RegAddress:uint8_t{
        RfSynthLock = 3,
        RawRssi = 6,
        RfConfig = 7,
        PaConfig = 9,
        OscEnable = 10,
        RssiPdn = 11,
        AutoCali = 23,
        DeviceID = 29,
        Config1 = 32,
        Delay1 = 33,
        Delay2 = 34,
        Config2 = 35,
        SyncWord0 = 36,
        SyncWord1 = 37,
        SyncWord2 = 38,
        SyncWord3 = 39,
        Threshold = 40,
        Config3 = 41,
        RxConfig = 42,
        RssiConfig = 43,
        DataRate = 44,
        Flag = 48,
        Fifo = 50,
        FifoPtr = 52
    }

    __fast_inline void delayT3(){
        delayNanoseconds(41);
    }

    __fast_inline void delayT5(){
        delayNanoseconds(450);
    }

    void writeReg(const RegAddress& address, const Reg16 & reg){
        bus_drv.write((uint8_t)address & 0x80, false);
        delayT3();

        bus_drv.write((uint8_t)((uint16_t)reg) >> 8);
        delayT5();
        bus_drv.write((uint8_t)((uint16_t)reg) & 0xff);
    }

    void readReg(const RegAddress& address, Reg16 & reg){
        uint8_t temp;
        bus_drv.transmit(temp, (uint8_t)address & 0x80, false);
        flagReg.data &= 0xff;
        flagReg.data |= temp << 8;

        delayT3();

        uint8_t buf[2];
        bus_drv.read(buf[1], false);
        delayT5();
        bus_drv.read(buf[0]);

        uint8_t * reg_ptr = (uint8_t *)&reg;
        reg_ptr[0] = buf[0];
        reg_ptr[1] = buf[1];
    }

    void writeByte(const RegAddress& address, const uint8_t & data){
        bus_drv.write((uint8_t)address & 0x80, false);
        delayT3();

        bus_drv.write(data);
    }

    void readByte(const RegAddress& address, uint8_t & data){
        bus_drv.write((uint8_t)address & 0x80, false);
        delayT3();
        bus_drv.read(data);
    }
public:


    bool isRfSynthLocked(){
        read(RegAddress::RfSynthLock, rfSynthLockReg);
        return rfSynthLockReg.rfSynthLock;
    }
    uint8_t getRssi(){
        readReg(RegAddress::RawRssi, rawRssiReg);
        return rawRssiReg.data;
    }
    void setRfChannel(const uint8_t & ch){
        rfConfigReg.rfChannelNo = ch;
        writeReg(RegAddress::RfConfig, rfConfigReg);
    }

    void setRadioMode(const bool & isRx){
        if(isRx){
            rfConfigReg.rxMode = 1;
            rfConfigReg.txMode = 0;
        }else{
            rfConfigReg.txMode = 1;
            rfConfigReg.rxMode = 0;
        }
        writeReg(RegAddress::RfConfig, rfConfigReg);
    }

    void setPaCurrent(const uint8_t & current){
        paConfigReg.paCurrent = current;
        writeReg(RegAddress::PaConfig, paConfigReg);
    }

    void setPaGain(const uint8_t & gain){
        paConfigReg.paGain = gain;
        writeReg(RegAddress::PaConfig, paConfigReg);
    }

    void enableRssi(const bool & open = enable){
        rssiPdnReg.rssiPdn = open;
        writeReg(RegAddress::RssiPdn, rssiPdnReg);
    }

    void enableAutoCali(const bool & open){
        autoCaliReg.autoCali = open;
        writeReg(RegAddress::AutoCali, autoCaliReg);
    }

    uint8_t getDigiVersion(){
        readReg(RegAddress::DeviceID, deviceIDReg);
        return deviceIDReg.digiVersion;
    }

    uint8_t getRfVersion(){
        readReg(RegAddress::DeviceID, deviceIDReg);
        return deviceIDReg.rfVersion;
    }

    void setBrclkSel(const BrclkSel & brclkSel){
        rfConfigReg.brclkSel = brclkSel;
        writeReg(RegAddress::RfConfig, rfConfigReg);
    }

    void clearFifoWritePtr(){
        fifoPtrReg.clearWritePtr = 1;
        writeReg(RegAddress::FifoPtr, fifoPtrReg);
    }

    void clearFifoReadPtr(){
        fifoPtrReg.clearReadPtr = 1;
        writeReg(RegAddress::FifoPtr, fifoPtrReg);
    }

    void setSyncWordLength(const SyncWordLen & len){
        syncWord0Reg.syncWordLength = (uint8_t)SyncWordLen;
        writeReg(RegAddress::SyncWord0, syncWord0Reg);
    }

    void setRetransTime(const uint8_t & times){
        config2Reg.retransTimes = times - 1;
        writeReg(RegAddress::Config2, config2Reg);
    }

    void set
    void enableAutoAck(const bool & en = true){
        config3Reg.autoAck = en;
        writeReg(RegAddress::Config3, config3Reg);
    }
    void init(){
        setBrclkSel(BrclkSel::Mhz12);
        delay(5);
        enableRssi();
        enableAutoAck();
        setSyncWordLength(SyncWordLen::b32);

    }
};


#ifdef LT8920_DEBUG
#undef LT8920_DEBUG
#endif
#endif