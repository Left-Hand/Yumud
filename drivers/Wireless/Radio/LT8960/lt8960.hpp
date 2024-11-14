#pragma once

#include "drivers/device_defs.h"


#ifdef LT8960_DEBUG
#define LT8960_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define LT8960_DEBUG(...)
#endif


namespace ymd::drivers{

class LT8960{
public:
    enum class PacketType:uint8_t{
        NRZLaw = 0,Manchester,Line8_10,Interleave
    };

    enum class TrailerBits:uint8_t{
        _4 = 0,_6,_8,_10,_12,_14,_16,_18
    };

    enum class SyncWordBits:uint8_t{
        _16, _32, _48, _64
    };

    enum class PreambleBits:uint8_t{
        _1 = 0, _2, _3, _4, _5, _6, _7, _8
    };

    enum class BrclkSel:uint8_t{
        Low = 0,Div1, Div2, Div4, Div8,
        Mhz1, Mhz12
    };

    enum class DataRate:uint8_t{
        Mbps1 = 0, Kbps250, Kbps125, Kbps62_5
    };


protected:

    #include "lt8960_regs.ipp"

    std::optional<SpiDrv> spi_drv;
    std::optional<I2cDrv> i2c_drv;


    GpioConcept * packet_status_gpio = nullptr;
    GpioConcept * fifo_status_gpio = nullptr;

    void delayT3(){delayMicroseconds(1);}

    void delayT5(){delayMicroseconds(1);}

    void writeReg(const RegAddress address, const uint16_t reg){
        if(i2c_drv){
            i2c_drv->writeReg((uint8_t)address, reg, MSB);
        }else if(spi_drv){
            TODO("not implemented yet");
        }
        LT8960_DEBUG("write",*(uint16_t *)&reg, "at", (uint8_t)address);
    }

    void readReg(const RegAddress address, uint16_t & reg){
        if(i2c_drv){
            i2c_drv->readReg((uint8_t)address, reg, MSB);
        }else if(spi_drv){
            TODO("not implemented yet");
        }
        LT8960_DEBUG("read",*(uint16_t *)&reg, "at", (uint8_t)address);
    }

    void writeByte(const RegAddress address, const uint8_t data){
        if(i2c_drv){
            i2c_drv->writeReg((uint8_t)address, data, MSB);
        }else if(spi_drv){
            TODO("not implemented yet");
        }
    }

    void readByte(const RegAddress address, uint8_t & data){
        if(spi_drv){
            spi_drv->writeSingle((uint8_t)((uint8_t)address & 0x80), CONT);
            delayT3();
            spi_drv->readSingle(data);
        }else if(i2c_drv){
            i2c_drv->readReg((uint8_t)address, data, MSB);
        }
    }
public:
    LT8960(const SpiDrv & _spi_drv) : spi_drv(_spi_drv) {;}
    LT8960(SpiDrv && _spi_drv) : spi_drv(_spi_drv) {;}
    LT8960(Spi & _spi, const uint8_t _index) : spi_drv(SpiDrv(_spi, _index)) {;}

    bool isRfSynthLocked(){
        readReg(RegAddress::RfSynthLock, (rfSynthLockReg));
        return rfSynthLockReg.synthLocked;
    }

    uint8_t getRssi(){
        readReg(RegAddress::RawRssi, rawRssiReg);
        return rawRssiReg.rawRssi;
    }

    void setRfChannel(const uint8_t ch){
        rfConfigReg.rfChannelNo = ch;
        writeReg(RegAddress::RfConfig, (rfConfigReg));
    }

    void setRfFreqMHz(const uint freq){}

    void setRadioMode(const bool isRx){
        if(isRx){
            rfConfigReg.txEn = false;
            rfConfigReg.rxEn = true;
        }else{
            rfConfigReg.rxEn = false;
            rfConfigReg.txEn = true;
        }
        writeReg(RegAddress::RfConfig, (rfConfigReg));
    }

    void setPaCurrent(const uint8_t current){
        paConfigReg.paCurrent = current;
        writeReg(RegAddress::PaConfig,(paConfigReg));
    }

    void setPaGain(const uint8_t gain){
        paConfigReg.paGain = gain;
        writeReg(RegAddress::PaConfig, (paConfigReg));
    }

    void enableRssi(const bool open = true){
        rssiPdnReg.rssiPdn = open;
        writeReg(RegAddress::RssiPdn, (rssiPdnReg));
    }

    void enableAutoCali(const bool open){
        autoCaliReg.autoCali = open;
        writeReg(RegAddress::AutoCali, (autoCaliReg));
    }

    uint8_t getDigiVersion(){
        readReg(RegAddress::DeviceID, (deviceIDReg));
        return deviceIDReg.digiVersion;
    }

    uint8_t getRfVersion(){
        readReg(RegAddress::DeviceID, (deviceIDReg));
        return deviceIDReg.rfVersion;
    }

    void setBrclkSel(const BrclkSel brclkSel){
        config1Reg.brclkSel = brclkSel;
        writeReg(RegAddress::Config1, (config1Reg));
    }

    void clearFifoWritePtr(){
        fifoPtrReg.clearWritePtr = 1;
        writeReg(RegAddress::FifoPtr, (fifoPtrReg));
    }

    void clearFifoReadPtr(){
        fifoPtrReg.clearReadPtr = 1;
        writeReg(RegAddress::FifoPtr, (fifoPtrReg));
    }

    void setSyncWordBitsgth(const SyncWordBits len){
        config1Reg.syncWordLen= len;
        writeReg(RegAddress::Config1, (config1Reg));
    }

    void setRetransTime(const uint8_t times){
        config2Reg.retransTimes = times - 1;
        writeReg(RegAddress::Config2, (config2Reg));
    }

    void enableAutoAck(const bool en = true){
        config3Reg.autoAck = en;
        writeReg(RegAddress::Config3, (config3Reg));
    }

    void init(){
        delay(5);
        setBrclkSel(BrclkSel::Mhz12);
        delay(5);
        enableRssi();
        enableAutoAck();
        setSyncWordBitsgth(SyncWordBits::_32);
    }
};

#ifdef LT8960_DEBUG
#undef LT8960_DEBUG
#endif

}
