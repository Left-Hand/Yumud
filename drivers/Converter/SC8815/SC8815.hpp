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


struct SC8815_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0110000);

    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;


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

    enum class SwitchingFreq:uint8_t{
        _150kHz = 0b00,
        _300kHz = 0b01,
        _450kHz = 0b11,
    };

    enum class DeadZone:uint8_t{
        _20ns = 0b00, //default
        _40ns = 0b01,
        _60ns = 0b10,
        _80ns = 0b11,
    };

    enum class BatVolt:uint8_t{
        _4_1V,
        _4_2V,//default
        _4_25V,
        _4_3V,
        _4_35V,
        _4_4V,
        _4_45V,
        _4_5V,
    };

    enum class BatCells:uint8_t{
        _1S,
        _2S,
        _3S,
        _4S
    };

    enum class BatIrComp:uint8_t{
        _0m,
        _20m,
        _40m,
        _80m,
    };

    struct BatConfig{
        BatVolt vcell_set;
        BatCells csel;
        bool use_ext_setting;
        BatIrComp ircomp;
    };

    struct RatioConfig{
        VBusRatio vbus_ratio;
        VBatMonRatio vbat_mon_ratio;
        IBusRatio ibus_ratio;
        IBatRatio ibat_ratio;
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

    static_assert(sizeof(Interrupts) == 1);
};

struct SC8815_Regs:public SC8815_Prelude {

    struct R8_VbatSet:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x00;

        BatVolt vcell_set:3;
        BatCells csel:2;
        uint8_t vbat_sel:1;
        BatIrComp ircomp:2;
    }DEF_R8(vbat_set_reg)

    struct R16_VbusRefISet:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x01;

        uint16_t value;
    }DEF_R16(vbus_ref_i_set_reg)

    struct R16_VbusRefESet:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x03;
        
        uint16_t value;
    }DEF_R16(vbus_ref_e_set_reg)
    
    struct R8_IBusLimSet:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x05;
        
        uint8_t :8;
    }DEF_R8(ibus_lim_set_reg)
    
    struct R8_IBatLimSet:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x06;

        uint8_t :8;
    }DEF_R8(ibat_lim_set_reg)

    struct R8_VinSet:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x07;

        uint8_t :8;
    }DEF_R8(vin_set_reg)
    
    struct R8_Ratio:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x08;

        VBusRatio vbus_ratio:1;
        VBatMonRatio vbat_mon_ratio:1;
        IBusRatio ibus_ratio:2;
        IBatRatio ibat_ratio:1;
        uint8_t :3;
    }DEF_R8(ratio_reg)

    struct R8_Ctrl0Set:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x09;

        uint8_t dt_set:2;
        uint8_t freq_set:2;
        uint8_t vinreg_ratio:1;
        uint8_t :2;
        uint8_t en_otg:1;
    }DEF_R8(ctrl0_set_reg)

    struct R8_Ctrl1Set:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x0A;
        
        uint8_t :2;
        uint8_t dis_ovp:1;
        uint8_t trickle_set:1;
        uint8_t fb_sel:1;
        uint8_t dis_term:1;
        uint8_t dis_trickle:1;
        uint8_t ichar_set:1;
    }DEF_R8(ctrl1_set_reg)

    struct R8_Ctrl2Set:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x0B;
        
        uint8_t slew_set:2;
        uint8_t en_dither:1;
        uint8_t factory:1;
        uint8_t :4;
    }DEF_R8(ctrl2_set_reg)

    struct R8_Ctrl3Set:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x0C;
        
        uint8_t en_pfm:1;
        uint8_t eoc_set:1;
        uint8_t dis_shortfoldback:1;
        uint8_t loop_set:1;
        uint8_t ilim_bw_sel:1;
        uint8_t ad_start:1;
        uint8_t gpo_ctrl:1;
        uint8_t en_pgate:1;
    }DEF_R8(ctrl3_set_reg)

    struct R16_VbusFbValue:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x0d;

        uint16_t value;
    }DEF_R16(vbus_fb_value_reg)

    struct R16_VbatFbValue:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x0f;

        uint16_t value;
    }DEF_R16(vbat_fb_value_reg)

    struct R16_IBusValue:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x11;

        uint16_t value;
    }DEF_R16(ibus_value_reg)

    struct R16_IBatValue:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x13;

        uint16_t value;
    }DEF_R16(ibat_value_reg)

    struct R16_AdinValue:public Reg16<>{
        static constexpr RegAddress REG_ADDR = 0x03;

        uint16_t value;
    }DEF_R16(adin_value_reg)

    struct R8_Status:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x17;
        Interrupts interrupts;
    }DEF_R8(status_reg)

    struct R8_Mask:public Reg8<>{
        static constexpr RegAddress REG_ADDR = 0x19;
        Interrupts interrupts_mask;
    }DEF_R8(mask_reg)
};

class SC8815 final:public SC8815_Prelude{
public:
    static constexpr auto DEFAULT_BAT_CONFIG = BatConfig {
        .vcell_set = BatVolt::_4_2V,
        .csel = BatCells::_1S,
        .use_ext_setting = false,
        .ircomp = BatIrComp::_20m
    };

    explicit SC8815(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit SC8815(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit SC8815(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<Interrupts> interrupts();

    [[nodiscard]] IResult<> init(const BatConfig & bat_conf);

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

    [[nodiscard]] IResult<> enable_otg(const Enable en);
    [[nodiscard]] IResult<> enable_trikle_charge(const Enable en);
    [[nodiscard]] IResult<> enable_ovp_protect(const Enable en);
    [[nodiscard]] IResult<> enable_dither(const Enable en);
    [[nodiscard]] IResult<> enable_adc_conv(const Enable en);
    [[nodiscard]] IResult<> enable_pfm_mode(const Enable en);
    [[nodiscard]] IResult<> enable_sfb(const Enable en);
    [[nodiscard]] IResult<> enable_gpo(const Enable en);
    [[nodiscard]] IResult<> enable_pgate(const Enable en);

    [[nodiscard]] IResult<> set_bat_volt(const BatVolt bat_volt);

    [[nodiscard]] IResult<> set_bat_cells(const BatCells bat_cells);

    [[nodiscard]] IResult<> enable_vbat_use_extneral(const bool use);
    [[nodiscard]] IResult<> set_bat_ir_comp(const BatIrComp bat_ir_comp);
    
    [[nodiscard]] IResult<> set_ibat_ratio(const IBatRatio ratio);
    [[nodiscard]] IResult<> set_ibus_ratio(const IBusRatio ratio);
    [[nodiscard]] IResult<> set_vbat_mon_ratio(const VBatMonRatio ratio);
    [[nodiscard]] IResult<> set_vbus_ratio(const VBusRatio ratio);
    
    
    [[nodiscard]] IResult<> reconf_bat(const BatConfig & config);
    [[nodiscard]] IResult<> reconf_ratio(const RatioConfig & config);
    [[nodiscard]] IResult<> reconf_interrupt_mask(const Interrupts mask);
private:
    using Regs = SC8815_Regs;
    hal::I2cDrv i2c_drv_;

    Regs regs_;
    uint32_t bus_shunt_res_mohms_ = 0;
    uint32_t bat_shunt_res_mohms_ = 0;
    real_t fb_up_res_kohms_ = 0;
    real_t fb_down_res_kohms_ = 0;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = i2c_drv_.write_reg(
            uint8_t(T::REG_ADDR), 
            reg.as_val(), LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = i2c_drv_.read_reg(
            uint8_t(T::REG_ADDR), 
            reg.as_ref(), LSB);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    
    [[nodiscard]] IResult<> read_burst(
        const RegAddress addr, 
        const std::span<uint8_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    [[nodiscard]] IResult<> power_up();
};

}