#include "SC8815.hpp"
#include "core/debug/debug.hpp"
#include "concept/pwm_channel.hpp"
#include "concept/analog_channel.hpp"

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


scexpr uint16_t b10(const int value, const int step) {
        
    int cnt = (value / step) - 1;
    uint8_t byte2 = cnt % 4;

    // uint8_t byte1 = ((cnt >> 2) << 6);
    uint8_t byte1 = ((cnt << 4) & 0xC0);
    
    return (byte2 << 8) | byte1;
}

scexpr int inv_b10(const uint16_t data, const int step) {
    
    uint8_t byte1 = data & 0xFF;
    uint8_t byte2 = data >> 14;

    return (4 * byte1 + byte2 + 1) * step;
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
    const auto ratio = ratio_reg.vbus_ratio ? 5.0_r : 12.5_r;
    if(const auto res = read_reg(vbus_fb_value_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(inv_b10(vbus_fb_value_reg, 1) * ratio / 1000);
}

IResult<real_t> SC8815::get_bus_curr(){
    const auto ratio = ratio_reg.ibus_ratio ? 6 : 3;
    if(const auto res = read_reg(ibus_value_reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(0.05_r * ratio * inv_b10(uint16_t(ibus_value_reg), 1) / 
        (3 * bus_shunt_res_mohms_));
}

IResult<real_t> SC8815::get_bat_volt(){
    const auto ratio = ratio_reg.vbat_mon_ratio ? 5.0_r : 12.5_r;
    if(const auto res = read_reg(vbat_fb_value_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(inv_b10(uint16_t(vbat_fb_value_reg), 1) * ratio * 2 / 1000);
}

IResult<real_t> SC8815::get_bat_curr(){
    const auto ratio = ratio_reg.ibat_ratio ? 6 : 3;
    if(const auto res = read_reg(ibat_value_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok((0.05_r * ratio * inv_b10(ibat_value_reg, 1) / 
        (3 * bus_shunt_res_mohms_)));
}

IResult<real_t> SC8815::get_adin_volt(){
    if(const auto res = read_reg(adin_value_reg);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(real_t(inv_b10(adin_value_reg, 1)) / 1000);
}

IResult<> SC8815::set_bus_curr_limit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x0c) == 4) ? 6 : 3;
    ibus_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    return write_reg(ibus_lim_set_reg);
}

IResult<> SC8815::set_bat_curr_limit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x10) == 16) ? 12 : 6;
    ibat_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    return write_reg(ibat_lim_set_reg);
}

IResult<> SC8815::set_output_volt(const real_t volt){

    //判断 VBUS 电压反馈的模式
    if (ctrl1_set_reg.fb_sel){
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

        vbus_ref_e_set_reg = uint16_t(tmp1 | (tmp2 << 14));
        return write_reg(vbus_ref_e_set_reg);
    }else{
        const int ratio = (ratio_reg & 0x01) ? 10 : 25; //取得 VBUS 电压的比率
        const uint16_t value = int(volt / ratio);   //计算对应的参考电压

        uint16_t tmp2;
        //得到 VBUSREF 寄存器 2 的值
        for (tmp2 = 0; tmp2 < 3; tmp2++)
        {
            if (((value - tmp2 - 1) % 4) == 0)
            {
                break;
            }
        }

        //得到 VBUSREF 寄存器 1 的值
        const uint16_t tmp1 = (tmp1 - tmp2 - 1) / 4;
        vbus_ref_i_set_reg = uint16_t(tmp1 | (tmp2 << 14));

        return write_reg(vbus_ref_i_set_reg);
    }


}

IResult<> SC8815::set_internal_vbus_ref(const real_t volt){
    vbus_ref_i_set_reg = b10(int(volt * 1000), 2);
    return write_reg(vbus_ref_i_set_reg);

}

IResult<> SC8815::set_external_vbus_ref(const real_t volt){
    vbus_ref_e_set_reg = b10(int(volt * 1000), 2);
    return write_reg(vbus_ref_e_set_reg);

}


IResult<> SC8815::set_ibat_lim_ratio(){
    TODO();
    return Ok();
}

IResult<> SC8815::enable_otg(const bool en){
    ctrl0_set_reg.en_otg = en;
    return write_reg(ctrl0_set_reg);
}

IResult<> SC8815::enable_trikle_charge(const bool en){
    ctrl1_set_reg.dis_trickle = !en;
    return write_reg(ctrl1_set_reg);

}

IResult<> SC8815::enable_ovp_protect(const bool en){
    TODO();
    return Ok();
}

IResult<> SC8815::power_up(){
    ctrl2_set_reg.factory = 1;//Factory setting bit. MCU shall write this bit to 1 after power up.
    return write_reg(ctrl2_set_reg);
}

IResult<> SC8815::enable_dither(const bool en){
    ctrl2_set_reg.en_dither = en;
    return write_reg(ctrl2_set_reg);

}

IResult<> SC8815::enable_adc_conv(const bool en){
    ctrl3_set_reg.ad_start = en;
    return write_reg(ctrl3_set_reg);
}

IResult<> SC8815::enable_pfm_mode(const bool en){
    ctrl3_set_reg.en_pfm = en;
    return write_reg(ctrl3_set_reg);
}

IResult<> SC8815::enable_sfb(const bool en){
    ctrl3_set_reg.dis_shortfoldback = !en;
    return write_reg(ctrl3_set_reg);
}

IResult<> SC8815::enable_gpo(const bool en){
    ctrl3_set_reg.gpo_ctrl = en;
    return write_reg(ctrl3_set_reg);
}

IResult<> SC8815::enable_pgate(const bool en){
    ctrl3_set_reg.en_pgate = en;
    return write_reg(ctrl3_set_reg);
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
    if(const auto res = read_reg(mask_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(static_cast<Interrupts>(mask_reg));
}

IResult<> SC8815::reconf_interrupt_mask(const Interrupts mask){
    mask_reg = std::bit_cast<uint8_t>(mask);
    return write_reg(mask_reg);
}



IResult<> SC8815::set_bat_volt(const BatVoltType bat_volt){
    vbat_set_reg.vcell_set = uint8_t(bat_volt);
    return write_reg(vbat_set_reg);
}

IResult<> SC8815::set_bat_cells(const BatCellsType bat_cells){
    vbat_set_reg.vcell_set = uint8_t(bat_cells);
    return write_reg(vbat_set_reg);
}

IResult<> SC8815::set_bat_cells(const uint32_t bat_cells){
    if(bat_cells > 4) SC8815_PANIC();
    return set_bat_cells(BatCellsType(bat_cells));
}

IResult<> SC8815::enable_vbat_use_extneral(const bool use){
    vbat_set_reg.vbat_sel = use;
    return write_reg(vbat_set_reg);
}

IResult<> SC8815::set_bat_ir_comp(const BatIrCompType bat_ir_comp){
    vbat_set_reg.ircomp = uint8_t(bat_ir_comp);
    return write_reg(vbat_set_reg);
}

IResult<> SC8815::set_ibat_ratio(const IBatRatio ratio){
    ratio_reg.ibat_ratio = uint8_t(ratio);
    return write_reg(ratio_reg);
}

IResult<> SC8815::set_ibus_ratio(const IBusRatio ratio){
    ratio_reg.ibus_ratio = uint8_t(ratio);
    return write_reg(ratio_reg);
}

IResult<> SC8815::set_vbat_mon_ratio(const VBatMonRatio ratio){
    ratio_reg.vbat_mon_ratio = uint8_t(ratio);
    return write_reg(ratio_reg);
}

IResult<> SC8815::set_vbus_ratio(const VBusRatio ratio){
    ratio_reg.vbus_ratio = uint8_t(ratio);
    return write_reg(ratio_reg);
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