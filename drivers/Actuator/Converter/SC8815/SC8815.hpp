#pragma once

#include "drivers/device_defs.h"



namespace ymd::drivers{

class SC8815{
protected:
    using RegAddress = uint8_t;
    uint bus_shunt_res_mohms_ = 0;
    uint bat_shunt_res_mohms_ = 0;
    real_t fb_up_res_kohms_ = 0;
    real_t fb_down_res_kohms_ = 0;

    hal::I2cDrv i2c_drv_;


    enum class IBatRatio:uint8_t{
        _6x = 0,
        _12x = 1//default
    };

    enum class IBusRatio:uint8_t{
        _6x = 1,//default
        _3x = 2//default
    };

    enum class VBatMonRatio:uint8_t{
        _12_5x = 0,//default
        _5x = 1
    };
    
    enum class VBusRatio:uint8_t{
        _12_5x = 0,//default
        _5x = 1,
    };

    enum class SwitchingFreq{
        _150kHz = 0b00,
        _300kHz = 0b01,
        _450kHz = 0b11,
    };

    enum class DeadZone{
        _20ns = 0b00, //default
        _40ns = 0b01,
        _60ns = 0b10,
        _80ns = 0b11,
    };

    enum class BatVoltType:uint8_t{
        _4_1V,
        _4_2V,//default
        _4_25V,
        _4_3V,
        _4_35V,
        _4_4V,
        _4_45V,
        _4_5V,
    };

    enum class BatCellsType:uint8_t{
        _1S,
        _2S,
        _3S,
        _4S
    };

    enum class BatIrCompType:uint8_t{
        _0m,
        _20m,
        _40m,
        _80m,
    };

    struct BatConfig{
        BatVoltType vcell_set;
        BatCellsType csel;
        bool use_ext_setting;
        BatIrCompType ircomp;
    };

    struct Interrupts{
        uint8_t :1;
        uint8_t eoc:1;
        uint8_t otp:1;
        uint8_t vbus_short:1;
        uint8_t :1;
        uint8_t indet:1;
        uint8_t ac_ok:1;
        uint8_t :1;
    };
    

    struct VbatSetReg:public Reg8<>{
        scexpr RegAddress address = 0x00;

        uint8_t vcell_set:3;
        uint8_t csel:2;
        uint8_t vbat_sel:1;
        uint8_t ircomp:2;
    };

    struct VbusRefISetReg:public Reg16<>{
        using Reg16::operator=;
        scexpr RegAddress address = 0x01;

        uint16_t :16;
    };

    struct VbusRefESetReg:public Reg16<>{
        using Reg16::operator=;
        scexpr RegAddress address = 0x03;
        
        uint16_t :16;
    };
    
    struct IBusLimSetReg:public Reg8<>{
        using Reg8::operator=;
        scexpr RegAddress address = 0x05;
        
        uint8_t :8;
    };
    
    struct IBatLimSetReg:public Reg8<>{
        using Reg8::operator=;
        scexpr RegAddress address = 0x06;

        uint8_t :8;
    };

    
    struct VinSetReg:public Reg8<>{
        scexpr RegAddress address = 0x07;

        uint8_t :8;
    };
    
    struct RatioReg:public Reg8<>{
        scexpr RegAddress address = 0x08;

        uint8_t vbus_ratio:1;
        uint8_t vbat_mon_ratio:1;
        uint8_t ibus_ratio:2;
        uint8_t ibat_ratio:1;
        uint8_t :3;
    };

    struct RatioConfig{
        VBusRatio vbus_ratio;
        VBatMonRatio vbat_mon_ratio;
        IBusRatio ibus_ratio;
        IBatRatio ibat_ratio;
    };

    struct Ctrl0SetReg:public Reg8<>{
        scexpr RegAddress address = 0x09;

        uint8_t dt_set:2;
        uint8_t freq_set:3;
        uint8_t vinreg_ratio:1;
        uint8_t :2;
        uint8_t en_otg:1;
    };

    struct Ctrl1SetReg:public Reg8<>{
        scexpr RegAddress address = 0x0A;
        
        uint8_t :2;
        uint8_t dis_ovp:1;
        uint8_t trickle_set:3;
        uint8_t fb_sel:1;
        uint8_t dis_term:1;
        uint8_t dis_trickle:1;
        uint8_t ichar_set:1;
    };

    struct Ctrl2SetReg:public Reg8<>{
        scexpr RegAddress address = 0x0B;
        
        uint8_t slew_set:2;
        uint8_t en_dither:1;
        uint8_t factory:1;
        uint8_t :4;
    };

    struct Ctrl3SetReg:public Reg8<>{
        scexpr RegAddress address = 0x0C;
        
        uint8_t en_pfm:1;
        uint8_t eoc_set:1;
        uint8_t dis_shortfoldback:1;
        uint8_t loop_set:1;
        uint8_t ilim_bw_sel:1;
        uint8_t ad_start:1;
        uint8_t gpo_ctrl:1;
        uint8_t en_pgate:1;
    };

    struct VbusFbValueReg:public Reg16<>{
        scexpr RegAddress address = 0x0d;

        uint16_t value;
    };

    struct VbatFbValueReg:public Reg16<>{
        scexpr RegAddress address = 0x0f;

        uint16_t value;
    };

    struct IBusValueReg:public Reg16<>{
        scexpr RegAddress address = 0x11;

        uint16_t :16;
    };

    struct IBatValueReg:public Reg16<>{
        scexpr RegAddress address = 0x13;

        uint16_t :16;
    };

    struct AdinValueReg:public Reg16<>{
        scexpr RegAddress address = 0x03;

        uint16_t :16;
    };

    struct StatusReg:public Reg8<>, public Interrupts{
        using Reg8::operator =;

        scexpr RegAddress address = 0x17;
    };

    struct MaskReg:public Reg8<>, public Interrupts{
        using Reg8::operator =;

        scexpr RegAddress address = 0x19;
    };


    VbatSetReg vbat_set_reg = {};
    VbusRefISetReg vbus_ref_i_set_reg = {};
    VbusRefESetReg vbus_ref_e_set_reg = {};

    IBusLimSetReg ibus_lim_set_reg = {};
    IBatLimSetReg ibat_lim_set_reg = {};
    
    VinSetReg vin_set_reg = {};
    RatioReg ratio_reg = {};

    Ctrl0SetReg ctrl0_set_reg = {};
    Ctrl1SetReg ctrl1_set_reg = {};
    Ctrl2SetReg ctrl2_set_reg = {};
    Ctrl3SetReg ctrl3_set_reg = {};

    VbusFbValueReg vbus_fb_value_reg = {};
    VbatFbValueReg vbat_fb_value_reg = {};

    IBusValueReg ibus_value_reg = {};
    IBatValueReg ibat_value_reg = {};

    AdinValueReg adin_value_reg = {};
    StatusReg status_reg = {};
    MaskReg mask_reg = {};


    BusError writeReg(const RegAddress address, const uint8_t reg){
        return i2c_drv_.writeReg(uint8_t(address), reg);
    }

    BusError readReg(const RegAddress address, uint8_t & reg){
        return i2c_drv_.readReg(uint8_t(address), reg);
    }

    BusError writeReg(const RegAddress address, const uint16_t reg){
        return i2c_drv_.writeReg(uint8_t(address), reg, LSB);
    }

    BusError readReg(const RegAddress address, uint16_t & reg){
        return i2c_drv_.readReg(uint8_t(address), reg, LSB);
    }
    BusError requestPool(const RegAddress addr, uint8_t * data, size_t len){
        return i2c_drv_.readMulti(uint8_t(addr), std::span(data, len));
    }

    SC8815 & powerUp();
    public:

    scexpr uint8_t default_i2c_addr = 0b01100000;

    SC8815(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SC8815(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    SC8815(hal::I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    Interrupts interrupts();

    SC8815 & init(const BatConfig & bat_conf = {
        .vcell_set = BatVoltType::_4_2V,
        .csel = BatCellsType::_1S,
        .use_ext_setting = false,
        .ircomp = BatIrCompType::_20m
    });

    bool verify();

    SC8815 & reset();
    real_t getBusVolt();
    real_t getBusCurr();
    real_t getBatVolt();
    real_t getBatCurr();
    real_t getAdinVolt();

    SC8815 & setBusCurrLimit(const real_t curr);
    SC8815 & setBatCurrLimit(const real_t curr);
    SC8815 & setOutputVolt(const real_t volt);

    SC8815 & setInternalVbusRef(const real_t volt);
    SC8815 & setExternalVbusRef(const real_t volt);

    SC8815 & setIBatLimRatio();

    SC8815 & enableOtg(const bool en = true);
    SC8815 & enableTrikleCharge(const bool en = true);
    SC8815 & enableOvpProtect(const bool en = true);
    SC8815 & enableDither(const bool en = true);
    SC8815 & enableAdcConv(const bool en = true);
    SC8815 & enablePfmMode(const bool en = true);
    SC8815 & enableSFB(const bool en = true);
    SC8815 & enableGpo(const bool en = true);
    SC8815 & enablePgate(const bool en = true);
    SC8815 & setBatVolt(const BatVoltType bat_volt);
    SC8815 & setBatCells(const BatCellsType bat_cells);
    SC8815 & setBatCells(const uint bat_cells);
    SC8815 & enableVbatUseExtneral(const bool use);
    SC8815 & setBatIrComp(const BatIrCompType bat_ir_comp);
    
    SC8815 & setIBatRatio(const IBatRatio ratio);
    SC8815 & setIBusRatio(const IBusRatio ratio);
    SC8815 & setVBatMonRatio(const VBatMonRatio ratio);
    SC8815 & setVBusRatio(const VBusRatio ratio);
    
    
    SC8815 & reconfBat(const BatConfig & config);
    SC8815 & reconfRatio(const RatioConfig & config);
    SC8815 & reconfInterruptMask(const Interrupts mask);
};

}