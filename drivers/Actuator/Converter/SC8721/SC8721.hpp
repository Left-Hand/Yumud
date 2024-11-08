#pragma once

#include "drivers/device_defs.h"

// #define SC8721_DEBUG

#ifdef SC8721_DEBUG
#undef SC8721_DEBUG
#define SC8721_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define SC8721_DEBUG(...)
#endif

namespace yumud::drivers{

class SC8721{
public:
    scexpr uint8_t default_i2c_addr = 0b01100000;

    SC8721(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SC8721(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    SC8721(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(I2cDrv(i2c, addr)){;}

    void update();

    bool verify();

    void reset();

    void setTargetVoltage(const real_t volt);

    void enableExternalFb(const bool en = true);

protected:
    using RegAddress = uint8_t;

    I2cDrv i2c_drv_;

    struct CSOReg:public Reg8{
        scexpr RegAddress address = 0x01;

        uint8_t cso_set;
    };

    struct SlopeCompReg:public Reg8{
        scexpr RegAddress address = 0x02;

        uint8_t slop_comp:1;
        uint8_t :7;
    };

    struct VoutSetReg:public Reg16{//msb
        scexpr RegAddress address = 0x03;

        uint8_t vout_set_msb:8;
        uint8_t vout_set_lsb:2;
        uint8_t fb_dir:1;
        uint8_t fb_on:1;
        uint8_t fb_sel:1;
        uint8_t :3;
    };

    struct GlobalCtrlReg:public Reg8{//msb
        scexpr RegAddress address = 0x05;

        uint8_t :1;
        uint8_t reg_load:1;
        uint8_t dis_dcdc:1;
        uint8_t :5;
    };

    struct SysSetReg:public Reg8{//msb
        scexpr RegAddress address = 0x06;

        uint8_t :4;
        uint8_t en_vinreg:1;
        uint8_t :1;
        uint8_t ext_dt:1;
        uint8_t en_pwm:1;
    };

    struct FreqSetReg:public Reg8{
        scexpr RegAddress address = 0x08;

        uint8_t freq_set:2;
        uint8_t :6;
    };

    struct Status1Reg:public Reg8{
        scexpr RegAddress address = 0x09;

        uint8_t ocp:1;
        uint8_t :2;
        uint8_t thd:1;
        uint8_t :2;
        uint8_t vout_vin_h:1;
        uint8_t vout_short:1;
    };

    struct Status2Reg:public Reg8{
        scexpr RegAddress address = 0x0A;

        uint8_t :1;
        uint8_t ibus_flag:1;
        uint8_t vinreg_flag:1;
        uint8_t :4;
        uint8_t vinovp:1;
    };

    struct{
        CSOReg cso_reg;
        SlopeCompReg slope_comp_reg;
        VoutSetReg vout_set_reg;
        GlobalCtrlReg global_ctrl;
        SysSetReg sys_set_reg;
        FreqSetReg freq_set_reg;
        Status1Reg status1_reg;
        Status2Reg status2_reg;
    };

    
    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv_.writeReg((uint8_t)address, reg, MSB);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv_.readReg((uint8_t)address, reg, MSB);
    }

    void requestPool(const RegAddress addr, uint8_t * data, size_t len){
        i2c_drv_.readMulti((uint8_t)addr, data, len, MSB);
    }

};

}