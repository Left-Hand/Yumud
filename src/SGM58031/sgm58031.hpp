#ifndef __SGM58031_HPP__

#define __SGM58031_HPP__

#include "../bus/busdrv.hpp"

#ifndef SGM_DEBUG
#include "../bus/uart/uart1.hpp"
#define SGM_DEBUG(...) uart1.println(__VA_ARGS__)
#endif

#ifndef REG16_BEGIN
#define REG16_BEGIN union{struct{
#endif

#ifndef REG16_END
#define REG16_END };uint16_t data;};
#endif

class SGM58031{
public:
    enum class DataRate:uint8_t{
        DR6_25 = 0,DR12_5, DR25, DR50, DR100, DR200, DR400, DR800,
        DR7_5 = 0b1000, DR15, DR30, DR60, DR120, DR240, DR480, DR960
    };

    enum class MUX:uint8_t{
        P0N1 = 0, P0N3, P1N3, P2N3, P0NG, P1NG, P2NG, P3NG
    };

    enum class PGA:uint8_t{
        FS6_144 = 0, FS4_096, FS2_048, FS1_024, FS0_512, FS0_256
    };
protected:
    BusDrv & busdrv;

    struct Reg16{
        Reg16 & operator = (const uint16_t & _data){(uint16_t &)*this = _data; return * this;}
        explicit operator uint16_t() const {return *(uint16_t *)this;}
    };

    struct ConfigReg:public Reg16{
        REG16_BEGIN
        uint8_t compQue : 2;
        uint8_t compLat : 1;
        uint8_t compPol : 1;
        uint8_t compMode :1;
        uint8_t dataRate :3;
        uint8_t mode:   1;
        uint8_t pga:    3;
        uint8_t mux:    3;
        uint8_t os:     1;
        REG16_END
    };

    struct Config1Reg:public Reg16{
        REG16_BEGIN
        uint8_t __resv1__    :3;
        uint8_t extRef      :1;
        uint8_t busFlex     :1;
        uint8_t __resv2__   :1;
        uint8_t burnOut     :1;
        uint8_t drSel       :1;
        uint8_t pd          :1;
        uint8_t __resv3__   :7;
        REG16_END
    };

    struct ChipIdReg:public Reg16{
        REG16_BEGIN
        uint8_t __resv1__   :5;
        uint8_t ver         :3;
        uint8_t id          :5;
        uint8_t __resv2__   :3;
        REG16_END
    };

    struct TrimReg:public Reg16{
        REG16_BEGIN
        uint16_t gn         :11;
        uint8_t __resv__    :5;
        REG16_END
    };

    struct ConvReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct LowThrReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct HighThrReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct{
        ConvReg convReg;
        ConfigReg configReg;
        LowThrReg lowThrReg;
        HighThrReg highThrReg;
        Config1Reg config1Reg;
        ChipIdReg chipIdReg;
        TrimReg trimReg;
    };

    enum class RegAddress:uint8_t{
        Conv = 0,
        Config,LowThr, HighThr, Config1, ChipID,Trim 
    };

    void writeReg(const RegAddress & regAddress, const Reg16 & regData){
        busdrv.writeReg((uint8_t)regAddress, (uint16_t)regData);
    }

    void readReg(const RegAddress & regAddress, Reg16 & regData){
        busdrv.readReg((uint8_t)regAddress, (uint16_t &)regData);
    }

    void requestRegData(const RegAddress & regAddress, uint8_t * data_ptr, const size_t len){
        busdrv.readPool((uint8_t)regAddress, data_ptr, 2, len);
    }

public:
    SGM58031(BusDrv & _busdrv):busdrv(_busdrv){;}

    void init(){
        readReg(RegAddress::Config, configReg);
        readReg(RegAddress::LowThr, lowThrReg);
        readReg(RegAddress::HighThr, highThrReg);
        readReg(RegAddress::Trim, trimReg);
        readReg(RegAddress::ChipID, chipIdReg);
    }

    void getId(){
        readReg(RegAddress::ChipID, chipIdReg);
        // SGM_DEBUG("Ver:", chipIdReg.ver, "Id", chipIdReg.id, uint16_t(chipIdReg));
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

    void setContMode(const bool continuous){
        configReg.mode = continuous;
        writeReg(RegAddress::Config, configReg);
    }

    void setDataRate(const DataRate & _dr){
        uint8_t dr = (uint8_t)_dr;
        configReg.dataRate = dr & 0b111;
        config1Reg.drSel = dr >> 3;
        writeReg(RegAddress::Config, configReg);
        writeReg(RegAddress::Config1, config1Reg);
    }

    void setMux(const MUX & _mux){
        uint8_t mux = (uint8_t)_mux;
        configReg.mux = mux;
        writeReg(RegAddress::Config, configReg);
    }

    void setPga(const PGA & _pga){
        uint8_t pga = (uint8_t)_pga;
        configReg.pga = pga;
        writeReg(RegAddress::Config, configReg);
    }

};

#ifdef SGM_DEBUG
#undef SGM_DEBUG
#endif

#endif