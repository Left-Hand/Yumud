#pragma once

//这个驱动已经完成
//这个驱动还未测试

// SC8815 是一款同步升降压充电控制器，同时支持电池
// 反向放电工作。芯片支持VBAT电池端36V高压工作，
// 因此在适配器电压高于、低于或者等于电池电压时，均
// 能保证1-6节锂电池应用。当工作系统需要电池提供能
// 量时，SC8815也支持电池反向放电工作，输出设定的
// 电压，并且放电最高电压可达36V。

#include "core/io/regs.hpp"
#include "core/utils/result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class SC8815{
public:
    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

protected:
    using RegAddress = uint8_t;
    uint32_t bus_shunt_res_mohms_ = 0;
    uint32_t bat_shunt_res_mohms_ = 0;
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


    [[nodiscard]] IResult<> write_reg(const RegAddress address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(const RegAddress address, const uint16_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddress address, uint16_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg, LSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddress addr, uint8_t * data, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(data, len));
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(const auto & reg){
        return write_reg(reg.address, reg.as_val());
    }

    [[nodiscard]] IResult<> read_reg(auto & reg){
        return write_reg(reg.address, reg.as_val());
    }

    [[nodiscard]] IResult<> power_up();

public:

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b01100000);

    SC8815(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    SC8815(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    SC8815(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<Interrupts> interrupts();

    [[nodiscard]] IResult<> init(const BatConfig & bat_conf = {
        .vcell_set = BatVoltType::_4_2V,
        .csel = BatCellsType::_1S,
        .use_ext_setting = false,
        .ircomp = BatIrCompType::_20m
    });

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<real_t> get_bus_volt();
    [[nodiscard]] IResult<real_t> get_bus_curr();
    [[nodiscard]] IResult<real_t> get_bat_volt();
    [[nodiscard]] IResult<real_t> get_bat_curr();
    [[nodiscard]] IResult<real_t> get_adin_volt();

    [[nodiscard]] IResult<> set_bus_curr_limit(const real_t curr);
    [[nodiscard]] IResult<> set_bat_curr_limit(const real_t curr);
    [[nodiscard]] IResult<> set_output_volt(const real_t volt);

    [[nodiscard]] IResult<> set_internal_vbus_ref(const real_t volt);
    [[nodiscard]] IResult<> set_external_vbus_ref(const real_t volt);

    [[nodiscard]] IResult<> set_ibat_lim_ratio();

    [[nodiscard]] IResult<> enable_otg(const bool en = true);
    [[nodiscard]] IResult<> enable_trikle_charge(const bool en = true);
    [[nodiscard]] IResult<> enable_ovp_protect(const bool en = true);
    [[nodiscard]] IResult<> enable_dither(const bool en = true);
    [[nodiscard]] IResult<> enable_adc_conv(const bool en = true);
    [[nodiscard]] IResult<> enable_pfm_mode(const bool en = true);
    [[nodiscard]] IResult<> enable_sfb(const bool en = true);
    [[nodiscard]] IResult<> enable_gpo(const bool en = true);
    [[nodiscard]] IResult<> enable_pgate(const bool en = true);
    [[nodiscard]] IResult<> set_bat_volt(const BatVoltType bat_volt);
    [[nodiscard]] IResult<> set_bat_cells(const BatCellsType bat_cells);
    [[nodiscard]] IResult<> set_bat_cells(const uint32_t bat_cells);
    [[nodiscard]] IResult<> enable_vbat_use_extneral(const bool use);
    [[nodiscard]] IResult<> set_bat_ir_comp(const BatIrCompType bat_ir_comp);
    
    [[nodiscard]] IResult<> set_ibat_ratio(const IBatRatio ratio);
    [[nodiscard]] IResult<> set_ibus_ratio(const IBusRatio ratio);
    [[nodiscard]] IResult<> set_vbat_mon_ratio(const VBatMonRatio ratio);
    [[nodiscard]] IResult<> set_vbus_ratio(const VBusRatio ratio);
    
    
    [[nodiscard]] IResult<> reconf_bat(const BatConfig & config);
    [[nodiscard]] IResult<> reconf_ratio(const RatioConfig & config);
    [[nodiscard]] IResult<> reconf_interrupt_mask(const Interrupts mask);
};

}