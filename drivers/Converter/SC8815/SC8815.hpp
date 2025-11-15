#pragma once

//这个驱动已经完成
//这个驱动还未测试

// SC8815 是一款同步升降压充电控制器，同时支持电池
// 反向放电工作。芯片支持VBAT电池端36V高压工作，
// 因此在适配器电压高于、低于或者等于电池电压时，均
// 能保证1-6节锂电池应用。当工作系统需要电池提供能
// 量时，SC8815也支持电池反向放电工作，输出设定的
// 电压，并且放电最高电压可达36V。

#include "sc8815_prelude.hpp"

namespace ymd::drivers{

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
            reg.as_bits(), std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = i2c_drv_.read_reg(
            uint8_t(T::REG_ADDR), 
            reg.as_mut_bits(), std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    
    [[nodiscard]] IResult<> read_burst(
        const RegAddr addr, 
        const std::span<uint8_t> pbuf
    ){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    [[nodiscard]] IResult<> power_up();
};

}