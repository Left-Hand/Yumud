#pragma once

#include "drivers/device_defs.h"

#define SC8815_DEBUG

#ifdef SC8815_DEBUG
#undef SC8815_DEBUG
#define SC8815_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define SC8815_PANIC(...) PANIC(__VA_ARGS__)
#define SC8815_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define SC8815_DEBUG(...)
#define SC8815_PANIC(...)  PANIC()
#define SC8815_ASSERT(cond, ...) ASSERT(cond)
#endif


#define WRITE_REG(reg) writeReg(reg.address, reg);
#define READ_REG(reg) readReg(reg.address, reg);

namespace ymd::drivers{

class SC8815{
public:
    scexpr uint8_t default_i2c_addr = 0b01100000;

    SC8815(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SC8815(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    SC8815(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(I2cDrv(i2c, addr)){;}

    SC8815 & init();

    bool verify();

    SC8815 & reset();

protected:
    using RegAddress = uint8_t;
    uint bus_shunt_res_mohms_;
    uint bat_shunt_res_mohms_;
    real_t fb_up_res_kohms_;
    real_t fb_down_res_kohms_;

    I2cDrv i2c_drv_;

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

    auto & setBatVolt(const BatVoltType bat_volt){
        vbat_set_reg.vcell_set = uint8_t(bat_volt);
        WRITE_REG(vbat_set_reg)
        return *this;
    }

    auto & setBatCells(const BatCellsType bat_cells){
        vbat_set_reg.vcell_set = uint8_t(bat_cells);
        WRITE_REG(vbat_set_reg)
        return *this;
    }

    auto & setBatCells(const uint bat_cells){
        if(bat_cells > 4) SC8815_PANIC();
        setBatCells(BatCellsType(bat_cells));
        return *this;
    }

    auto & setVbatUseExtneral(const bool use){
        vbat_set_reg.vbat_sel = use;
        WRITE_REG(vbat_set_reg)
        return *this;
    }

    auto & setBatIrComp(const BatIrCompType bat_ir_comp){
        vbat_set_reg.ircomp = uint8_t(bat_ir_comp);
        WRITE_REG(vbat_set_reg)
        return *this;
    }

    // template<typename T>
    static constexpr uint16_t b10(const int value, const int step) {
        
        int cnt = (value / step) - 1;
        uint8_t byte2 = cnt % 4;

        // uint8_t byte1 = ((cnt >> 2) << 6);
        uint8_t byte1 = ((cnt << 4) & 0xC0);
        
        return (byte2 << 8) | byte1;
    }

    static constexpr int inv_b10(const uint16_t data, const int step) {
        
        uint8_t byte1 = data & 0xFF;
        uint8_t byte2 = data >> 14;

        return (4 * byte1 + byte2 + 1) * step;
    }

    auto & setInternalVbusRef(const real_t volt){
        vbus_ref_i_set_reg = b10(int(volt * 1000), 2);
        WRITE_REG(vbus_ref_i_set_reg)
        return *this;
    }

    auto & setExternalVbusRef(const real_t volt){
        vbus_ref_e_set_reg = b10(int(volt * 1000), 2);
        WRITE_REG(vbus_ref_e_set_reg)
        return *this;
    }

    auto & setIBusCurrLimit(const real_t curr){
        TODO();
        return *this;
    }

    auto & setEBusCurrLimit(const real_t curr){
        TODO();
        return *this;
    }

    auto & setVinRegRefVolt(const real_t volt){
        TODO();
        return *this;
    }

    enum class IBatRatio:uint8_t{
        _6x = 0,
        _12x = 1//default
    };

    enum class IBusRatio:uint8_t{
        _6x = 1,//default
        _3x = 2//default
    };

    enum class VbatMonRatio:uint8_t{
        _12_5x = 0,//default
        _5x = 1
    };
    
    enum class VBusRatio:uint8_t{
        _12_5x = 0,//default
        _5x = 1,
    };

    auto & setIBatLimRatio(){
        return *this;
    }

    auto & enableOtg(const bool en = true){
        ctrl0_set_reg.en_otg = en;
        WRITE_REG(ctrl0_set_reg)
        return *this;
    }

    auto & enableTrikleCharge(const bool en = true){
        ctrl1_set_reg.dis_trickle = !en;
        WRITE_REG(ctrl1_set_reg)
        return *this;
    }

    auto & enableOvpProtect(const bool en = true){
        return *this;
    }

    auto & powerUp(){
        ctrl2_set_reg.factory = 1;//Factory setting bit. MCU shall write this bit to 1 after power up.
        WRITE_REG(ctrl2_set_reg);
        return*this;
    }

    auto & enableDither(const bool en = true){
        ctrl2_set_reg.en_dither = en;
        WRITE_REG(ctrl2_set_reg);
        return *this;
    }

    auto & enableAdcConv(const bool en = true){
        ctrl3_set_reg.ad_start = en;
        WRITE_REG(ctrl3_set_reg);
        return *this;
    }

    auto & enablePfmMode(const bool en = true){
        ctrl3_set_reg.en_pfm = en;
        WRITE_REG(ctrl3_set_reg);
        return*this;
    }

    auto & enableSFB(const bool en = true){
        ctrl3_set_reg.dis_shortfoldback = !en;
        WRITE_REG(ctrl3_set_reg);
        return *this;
    }

    auto & enableGpo(const bool en = true){
        ctrl3_set_reg.gpo_ctrl = en;
        WRITE_REG(ctrl3_set_reg);
        return *this;
    }

    auto & enablePgate(const bool en = true){
        ctrl3_set_reg.en_pgate = en;
        WRITE_REG(ctrl3_set_reg);
        return *this;
    }

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


    struct VbatSetReg:public Reg8{
        scexpr RegAddress address = 0x00;

        uint8_t vcell_set:3;
        uint8_t csel:2;
        uint8_t vbat_sel:1;
        uint8_t ircomp:2;
    };

    struct VbusRefISetReg:public Reg16{
        using Reg16::operator=;
        scexpr RegAddress address = 0x01;

        uint16_t :16;
    };

    struct VbusRefESetReg:public Reg16{
        using Reg16::operator=;
        scexpr RegAddress address = 0x03;
        
        uint16_t :16;
    };
    
    struct IBusLimSetReg:public Reg8{
        using Reg8::operator=;
        scexpr RegAddress address = 0x05;
        
        uint8_t :8;
    };
    
    struct IBatLimSetReg:public Reg8{
        using Reg8::operator=;
        scexpr RegAddress address = 0x06;

        uint8_t :8;
    };

    
    struct VinSetReg:public Reg8{
        scexpr RegAddress address = 0x07;

        uint8_t :8;
    };
    
    struct RatioReg:public Reg8{
        scexpr RegAddress address = 0x08;

        uint8_t vbus_ratio:1;
        uint8_t vbat_mon_ratio:1;
        uint8_t ibus_ratio:2;
        uint8_t ibat_ratio:1;
        uint8_t :3;
    };

    struct Ctrl0SetReg:public Reg8{
        scexpr RegAddress address = 0x09;

        uint8_t dt_set:2;
        uint8_t freq_set:3;
        uint8_t vinreg_ratio:1;
        uint8_t :2;
        uint8_t en_otg:1;
    };

    struct Ctrl1SetReg:public Reg8{
        scexpr RegAddress address = 0x0A;
        
        uint8_t :2;
        uint8_t dis_ovp:1;
        uint8_t trickle_set:3;
        uint8_t fb_sel:1;
        uint8_t dis_term:1;
        uint8_t dis_trickle:1;
        uint8_t ichar_set:1;
    };

    struct Ctrl2SetReg:public Reg8{
        scexpr RegAddress address = 0x0B;
        
        uint8_t slew_set:2;
        uint8_t en_dither:1;
        uint8_t factory:1;
        uint8_t :4;
    };

    struct Ctrl3SetReg:public Reg8{
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

    struct VbusFbValueReg:public Reg16{
        scexpr RegAddress address = 0x0d;

        uint16_t value;
    };

    struct VbatFbValueReg:public Reg16{
        scexpr RegAddress address = 0x0f;

        uint16_t value;
    };

    struct IBusValueReg:public Reg16{
        scexpr RegAddress address = 0x11;

        uint16_t :16;
    };

    struct IBatValueReg:public Reg16{
        scexpr RegAddress address = 0x13;

        uint16_t :16;
    };

    struct AdinValueReg:public Reg16{
        scexpr RegAddress address = 0x03;

        uint16_t :16;
    };

    struct StatusReg:public Reg8{
        scexpr RegAddress address = 0x17;

        uint8_t :1;
        uint8_t eoc:1;
        uint8_t otp:1;
        uint8_t vbus_short:1;
        uint8_t :1;
        uint8_t indet:1;
        uint8_t ac_ok:1;
        uint8_t :1;
    };

    struct MaskReg:public Reg8{
        scexpr RegAddress address = 0x19;

        uint8_t :1;
        uint8_t eoc:1;
        uint8_t otp:1;
        uint8_t vbus_short:1;
        uint8_t :1;
        uint8_t indet:1;
        uint8_t ac_ok:1;
        uint8_t :1;
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


    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv_.writeReg((uint8_t)address, reg);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv_.readReg((uint8_t)address, reg);
    }

    void writeReg(const RegAddress address, const uint16_t reg){
        i2c_drv_.writeReg((uint8_t)address, reg, LSB);
    }

    void readReg(const RegAddress address, uint16_t & reg){
        i2c_drv_.readReg((uint8_t)address, reg, LSB);
    }
    void requestPool(const RegAddress addr, uint8_t * data, size_t len){
        i2c_drv_.readMulti((uint8_t)addr, data, len);
    }
public:
    real_t getBusVolt();
    real_t getBusCurr();
    real_t getBatVolt();
    real_t getBatCurr();
    real_t getAdinVolt();

    auto & setBusCurrLimit(const real_t curr);
    auto & setBatCurrLimit(const real_t curr);

    void setOutputVolt(const iq_t volt);

};

}