#include "SC8815.hpp"

using namespace ymd::drivers;

SC8815 & SC8815::init(){
    return *this;
}

SC8815 & SC8815::reset(){
    return *this;
}

bool SC8815::verify(){
    return true;
}


real_t SC8815::getBusVolt(){
    const auto ratio = ratio_reg.vbus_ratio ? 5.0_r : 12.5_r;
    READ_REG(vbus_fb_value_reg);
    return inv_b10(vbus_fb_value_reg, 1) * ratio / 1000;
}

real_t SC8815::getBusCurr(){
    const auto ratio = ratio_reg.ibus_ratio ? 6 : 3;
    READ_REG(ibus_value_reg);

    return (0.05_r * ratio * inv_b10(uint16_t(ibus_value_reg), 1) / 
        (3 * bus_shunt_res_mohms_));
}

real_t SC8815::getBatVolt(){
    const auto ratio = ratio_reg.vbat_mon_ratio ? 5.0_r : 12.5_r;
    READ_REG(vbat_fb_value_reg);
    return inv_b10(uint16_t(vbat_fb_value_reg), 1) * ratio * 2 / 1000;
}

real_t SC8815::getBatCurr(){
    const auto ratio = ratio_reg.ibat_ratio ? 6 : 3;
    READ_REG(ibat_value_reg);

    return (0.05_r * ratio * inv_b10(ibat_value_reg, 1) / 
        (3 * bus_shunt_res_mohms_));
}

real_t SC8815::getAdinVolt(){
    READ_REG(adin_value_reg);

    return inv_b10(adin_value_reg, 1) / 1000;
}

auto & SC8815::setBusCurrLimit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x0c) == 4) ? 6 : 3;
    ibus_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    WRITE_REG(ibus_lim_set_reg);

    return *this;
}

auto & SC8815::setBatCurrLimit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x10) == 16) ? 12 : 6;
    ibat_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    WRITE_REG(ibat_lim_set_reg);

    return *this;
}

void SC8815::setOutputVolt(const iq_t volt){

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
        WRITE_REG(vbus_ref_e_set_reg);
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
       
        WRITE_REG(vbus_ref_i_set_reg);
    }
}