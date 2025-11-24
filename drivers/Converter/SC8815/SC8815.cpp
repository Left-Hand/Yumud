#include "SC8815.hpp"
#include "core/debug/debug.hpp"
#include "primitive/pwm_channel.hpp"
#include "primitive/analog_channel.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define SC8815_DEBUG_EN

#ifdef SC8815_DEBUG_EN
#undef SC8815_DEBUG
#define SC8815_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define SC8815_PANIC(...) PANIC(__VA_ARGS__)
#define SC8815_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define SC8815_DEBUG(...)
#define SC8815_PANIC(...)  PANIC()
#define SC8815_ASSERT(cond, ...) ASSERT(cond)
#endif

using Error = SC8815::Error;

template<typename T = void>
using IResult = Result<T, Error>;


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

static constexpr real_t to_num(SC8815::VBusRatio ratio){
    switch(ratio){
        case SC8815::VBusRatio::_12_5x: return 12.5_r;
        case SC8815::VBusRatio::_5x: return 5_r;
        default: __builtin_unreachable();
    }
}

static constexpr real_t to_num(SC8815::IBatRatio ratio){
    switch(ratio){
        case SC8815::IBatRatio::_6x:  return 6_r;
        case SC8815::IBatRatio::_12x: return 12_r;
        default: __builtin_unreachable();
    }
}

static constexpr uint32_t to_num(SC8815::IBusRatio ratio){
    switch(ratio){
        case SC8815::IBusRatio::_6x:  return 6;
        case SC8815::IBusRatio::_3x: return 3;
        default: __builtin_unreachable();
    }
}

static constexpr real_t to_num(SC8815::VBatMonRatio ratio){
    switch(ratio){
        case SC8815::VBatMonRatio::_12_5x: return 12.5_r;
        case SC8815::VBatMonRatio::_5x: return 5_r;
        default: __builtin_unreachable();
    }
}


IResult<> SC8815::init(const BatConfig & bat_conf){
    if(const auto res = this -> power_up();
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = reconf_bat({
        bat_conf
    }); res.is_err()) return Err(res.unwrap_err());
    
    if(const auto res = reconf_ratio({
        .vbus_ratio = VBusRatio::_12_5x,
        .vbat_mon_ratio = VBatMonRatio::_12_5x,
        .ibus_ratio = IBusRatio::_3x,
        .ibat_ratio = IBatRatio::_6x
    }); res.is_err()) return Err(res.unwrap_err());
    
    if(const auto res = reconf_interrupt_mask({
        .eoc = true,
        .otp = true,
        .vbus_short = true,
        .indet = true,
        .ac_ok = true
    }); res.is_err()) return Err(res.unwrap_err());

    // set_bat_curr_limit(1.2_r);
    // set_bus_curr_limit(1.2_r);
    // setVinRegRefVolt(12);
    // enable_otg(false);

    return Ok();
}

IResult<> SC8815::reset(){
    TODO();
    return Ok();
}

IResult<> SC8815::validate(){
    TODO();
    return Ok();
}


IResult<real_t> SC8815::get_bus_volt(){
    const auto ratio = to_num(regs_.ratio_reg.vbus_ratio);
    auto & reg = regs_.vbus_fb_value_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(inv_b10(reg.to_bits(), 1) * ratio / 1000);
}

IResult<real_t> SC8815::get_bus_curr(){
    const auto ratio = to_num(regs_.ratio_reg.ibus_ratio);
    auto & reg = regs_.ibus_value_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(0.05_r * ratio * inv_b10(uint16_t(reg.to_bits()), 1) / 
        (3 * bus_shunt_res_mohms_));
}

IResult<real_t> SC8815::get_bat_volt(){
    const auto ratio = to_num(regs_.ratio_reg.vbat_mon_ratio);
    auto & reg = regs_.vbat_fb_value_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(inv_b10(reg.to_bits(), 1) * ratio * 2 / 1000);
}

IResult<real_t> SC8815::get_bat_curr(){
    const auto ratio = to_num(regs_.ratio_reg.ibat_ratio);
    auto & reg = regs_.ibat_value_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok((0.05_r * ratio * inv_b10(reg.to_bits(), 1) / 
        (3 * bus_shunt_res_mohms_)));
}

IResult<real_t> SC8815::get_adin_volt(){
    auto & reg = regs_.adin_value_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(real_t(inv_b10(reg.to_bits(), 1)) / 1000);
}

IResult<> SC8815::set_bus_curr_limit(const real_t limit_ma){
    const auto ratio = to_num(regs_.ratio_reg.ibus_ratio);
    auto reg = RegCopy(regs_.ibus_lim_set_reg);
    reg.as_bits_mut() = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    return write_reg(reg);
}

IResult<> SC8815::set_bat_curr_limit(const real_t limit_ma){
    const auto ratio = to_num(regs_.ratio_reg.ibat_ratio);
    auto reg = RegCopy(regs_.ibat_lim_set_reg);
    reg.as_bits_mut() = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    return write_reg(reg);
}

IResult<> SC8815::set_output_volt(const real_t volt){

    //判断 VBUS 电压反馈的模式
    if (regs_.ctrl1_set_reg.fb_sel){
        const uint16_t value = int(volt * fb_down_res_kohms_ / (fb_up_res_kohms_ + fb_down_res_kohms_) / 2);                         //计算对应的参考电压

        //得到 VBUSREF 寄存器 2 的值
        uint16_t tmp2;

        for (tmp2 = 0; tmp2 < 3; tmp2++)
        {
            if (((value - tmp2 - 1) % 4) == 0)
            {
                break;
            }
        }

        //得到 VBUSREF 寄存器 1 的值
        const uint16_t tmp1 = (value - tmp2 - 1) / 4;

        //写入到 SC8815 VBUSREF_E_SET 寄存器

        auto reg = RegCopy(regs_.vbus_ref_e_set_reg);
        reg.as_bits_mut() = uint16_t(tmp1 | (tmp2 << 14));
        return write_reg(reg);
    }else{
        const auto ratio = to_num(regs_.ratio_reg.vbus_ratio); //取得 VBUS 电压的比率
        const uint16_t value = int(volt / ratio);   //计算对应的参考电压

        uint16_t tmp2;
        //得到 VBUSREF 寄存器 2 的值
        for (tmp2 = 0; tmp2 < 3; tmp2++){
            if (((value - tmp2 - 1) % 4) == 0){
                break;
            }
        }

        //得到 VBUSREF 寄存器 1 的值
        const uint16_t tmp1 = (tmp1 - tmp2 - 1) / 4;
        auto reg = RegCopy(regs_.vbus_ref_i_set_reg);
        reg.as_bits_mut() = uint16_t(tmp1 | (tmp2 << 14));

        return write_reg(reg);
    }


}

IResult<> SC8815::set_internal_vbus_ref(const real_t volt){
    auto reg = RegCopy(regs_.vbus_ref_i_set_reg);
    reg.as_bits_mut() = b10(int(volt * 1000), 2);
    return write_reg(reg);

}

IResult<> SC8815::set_external_vbus_ref(const real_t volt){
    auto reg = RegCopy(regs_.vbus_ref_e_set_reg);
    reg.as_bits_mut() = b10(int(volt * 1000), 2);
    return write_reg(reg);

}


IResult<> SC8815::set_ibat_lim_ratio(){
    TODO();
    return Ok();
}

IResult<> SC8815::enable_otg(const Enable en){
    auto reg = RegCopy(regs_.ctrl0_set_reg);
    reg.en_otg = en == EN;
    return write_reg(reg);
}

IResult<> SC8815::enable_trikle_charge(const Enable en){
    auto reg = RegCopy(regs_.ctrl1_set_reg);
    reg.dis_trickle = (!en).to_bool();
    return write_reg(reg);

}

IResult<> SC8815::enable_ovp_protect(const Enable en){
    TODO();
    return Ok();
}

IResult<> SC8815::power_up(){
    auto reg = RegCopy(regs_.ctrl2_set_reg);
    reg.factory = 1;
    //Factory setting bit. MCU shall write this bit to 1 after power up.
    return write_reg(reg);
}

IResult<> SC8815::enable_dither(const Enable en){
    auto reg = RegCopy(regs_.ctrl2_set_reg);
    reg.en_dither = en == EN;
    return write_reg(reg);

}

IResult<> SC8815::enable_adc_conv(const Enable en){
    auto reg = RegCopy(regs_.ctrl3_set_reg);
    reg.ad_start = en == EN;
    return write_reg(reg);
}

IResult<> SC8815::enable_pfm_mode(const Enable en){
    auto reg = RegCopy(regs_.ctrl3_set_reg);
    reg.en_pfm = en == EN;
    return write_reg(reg);
}

IResult<> SC8815::enable_sfb(const Enable en){
    auto reg = RegCopy(regs_.ctrl3_set_reg);
    reg.dis_shortfoldback = en == DISEN;
    return write_reg(reg);
}

IResult<> SC8815::enable_gpo(const Enable en){
    auto reg = RegCopy(regs_.ctrl3_set_reg);
    reg.gpo_ctrl = en == EN;
    return write_reg(reg);
}

IResult<> SC8815::enable_pgate(const Enable en){
    auto reg = RegCopy(regs_.ctrl3_set_reg);
    reg.en_pgate = en == EN;
    return write_reg(reg);
}

IResult<> SC8815::reconf_bat(const BatConfig & config){
    if(const auto res = this -> set_bat_volt(config.vcell_set);
        res.is_err()) return res;
    if(const auto res = this -> set_bat_ir_comp(config.ircomp);
        res.is_err()) return res;
    if(const auto res = this -> set_bat_cells(config.csel);
        res.is_err()) return res;
    if(const auto res = this -> enable_vbat_use_extneral(config.use_ext_setting);
        res.is_err()) return res;
    return Ok();
}

IResult<SC8815::Interrupts> SC8815::interrupts(){
    auto & reg = regs_.mask_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.interrupts_mask);
}

IResult<> SC8815::reconf_interrupt_mask(const Interrupts mask){
    auto reg = RegCopy(regs_.mask_reg);
    reg.as_bits_mut() = std::bit_cast<uint8_t>(mask);
    return write_reg(reg);
}



IResult<> SC8815::set_bat_volt(const BatVolt bat_volt){
    auto reg = RegCopy(regs_.vbat_set_reg);
    reg.vcell_set = bat_volt;
    return write_reg(reg);
}

IResult<> SC8815::set_bat_cells(const BatCells bat_cells){
    auto reg = RegCopy(regs_.vbat_set_reg);
    reg.csel = bat_cells;
    return write_reg(reg);
}

IResult<> SC8815::enable_vbat_use_extneral(const bool use){
    auto reg = RegCopy(regs_.vbat_set_reg);
    reg.vbat_sel = use;
    return write_reg(reg);
}

IResult<> SC8815::set_bat_ir_comp(const BatIrComp bat_ir_comp){
    auto reg = RegCopy(regs_.vbat_set_reg);
    reg.ircomp = bat_ir_comp;
    return write_reg(reg);
}

IResult<> SC8815::set_ibat_ratio(const IBatRatio ratio){
    auto reg = RegCopy(regs_.ratio_reg);
    reg.ibat_ratio = ratio;
    return write_reg(reg);
}

IResult<> SC8815::set_ibus_ratio(const IBusRatio ratio){
    auto reg = RegCopy(regs_.ratio_reg);
    reg.ibus_ratio = ratio;
    return write_reg(reg);
}

IResult<> SC8815::set_vbat_mon_ratio(const VBatMonRatio ratio){
    auto reg = RegCopy(regs_.ratio_reg);
    reg.vbat_mon_ratio = ratio;
    return write_reg(reg);
}

IResult<> SC8815::set_vbus_ratio(const VBusRatio ratio){
    auto reg = RegCopy(regs_.ratio_reg);
    reg.vbus_ratio = ratio;
    return write_reg(reg);
}

IResult<> SC8815::reconf_ratio(const RatioConfig & config){
    if(const auto res = this->set_ibat_ratio(config.ibat_ratio);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = this->set_ibus_ratio(config.ibus_ratio);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = this->set_vbat_mon_ratio(config.vbat_mon_ratio);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = this->set_vbus_ratio(config.vbus_ratio);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}