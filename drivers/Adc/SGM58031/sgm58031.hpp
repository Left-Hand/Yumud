#pragma once

#include "drivers/device_defs.h"

#ifndef SGM58031_DEBUG
#define SGM58031_DEBUG(...) DEBUG_LOG(...)
#endif

namespace ymd::drivers{

class SGM58031{
public:
    enum class DataRate:uint8_t{
        DR6_25 = 0,DR12_5, DR25, DR50, DR100, DR200, DR400, DR800,
        DR7_5 = 0b1000, DR15, DR30, DR60, DR120, DR240, DR480, DR960
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class FS:uint8_t{
        FS6_144 = 0, FS4_096, FS2_048, FS1_024, FS0_512, FS0_256
    };

    enum class PGA:uint8_t{
        RT2_3 = 0, RT1, RT2, RT4, RT8, RT16
    };
protected:
    hal::I2cDrv i2c_drv_;

    struct ConfigReg:public Reg16{
        
        uint8_t compQue : 2;
        uint8_t compLat : 1;
        uint8_t compPol : 1;
        uint8_t compMode :1;
        uint8_t dataRate :3;
        uint8_t mode:   1;
        uint8_t pga:    3;
        uint8_t mux:    3;
        uint8_t os:     1;
        
    };

    struct Config1Reg:public Reg16{
        
        uint8_t __resv1__    :3;
        uint8_t extRef      :1;
        uint8_t busFlex     :1;
        uint8_t __resv2__   :1;
        uint8_t burnOut     :1;
        uint8_t drSel       :1;
        uint8_t pd          :1;
        uint8_t __resv3__   :7;
        
    };

    struct DeviceIdReg:public Reg16{
        
        uint8_t __resv1__   :5;
        uint8_t ver         :3;
        uint8_t id          :5;
        uint8_t __resv2__   :3;
        
    };

    struct TrimReg:public Reg16{
        
        uint16_t gn         :11;
        uint8_t __resv__    :5;
        
    };

    struct ConvReg:public Reg16{
        uint16_t data;
        
    };

    struct LowThrReg:public Reg16{
        uint16_t data;
        
    };

    struct HighThrReg:public Reg16{
        uint16_t data;
        
    };

    ConvReg convReg;
    ConfigReg configReg;
    LowThrReg lowThrReg;
    HighThrReg highThrReg;
    Config1Reg config1Reg;
    DeviceIdReg deviceIdReg;
    TrimReg trimReg;

    enum class RegAddress:uint8_t{
        Conv = 0,
        Config,LowThr, HighThr, Config1, DeviceID,Trim
    };

    real_t fullScale;
    void writeReg(const RegAddress addr, const uint16_t data){
        i2c_drv_.writeReg((uint8_t)addr, data, MSB);
    }

    void readReg(const RegAddress addr, uint16_t & data){
        i2c_drv_.readReg((uint8_t)addr, data, MSB);
    }

public:
    scexpr uint8_t default_i2c_addr = 0b0100000;

    SGM58031(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SGM58031(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    SGM58031(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}

    void init();

    void getDeviceId(){
        readReg(RegAddress::DeviceID, deviceIdReg);
    }

    bool isIdle(){
        readReg(RegAddress::Config, configReg);
        return configReg.os;
    }

    void startConv(){
        config1Reg.pd = true;
        writeReg(RegAddress::Config1, config1Reg);
        configReg.os = true;
        writeReg(RegAddress::Config, configReg);
    }

    int16_t getConvData(){
        readReg(RegAddress::Conv, convReg);
        return *(int16_t *)&convReg;
    }

    real_t getConvVoltage(){
        int16_t data = getConvData();
        real_t uni = real_t(data) / (1 << 15);
        return uni * fullScale;
    }
    void setContMode(const bool continuous){
        configReg.mode = continuous;
        writeReg(RegAddress::Config, configReg);
    }

    void setDataRate(const DataRate _dr);

    void setMux(const MUX _mux){
        configReg.mux = (uint8_t)_mux;
        writeReg(RegAddress::Config, configReg);
    }

    void setFS(const FS fs);

    void setFS(const real_t _fs, const real_t _vref);

    void setTrim(const real_t _trim);
    void enableCh3AsRef(bool yes){
        config1Reg.extRef = yes;
        writeReg(RegAddress::Config1, config1Reg);
    }
};

#ifdef SGM58031_DEBUG
#undef SGM58031_DEBUG
#endif


}