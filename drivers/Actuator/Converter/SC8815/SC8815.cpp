#include "SC8815.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;

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


#define WRITE_REG(reg) write_reg(reg.address, reg).unwrap();
#define READ_REG(reg) read_reg(reg.address, reg).unwrap();

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



SC8815 & SC8815::init(const BatConfig & bat_conf){
    this -> powerUp()

    .reconfBat({
        bat_conf
    }).reconfRatio({
        .vbus_ratio = VBusRatio::_12_5x,
        .vbat_mon_ratio = VBatMonRatio::_12_5x,
        .ibus_ratio = IBusRatio::_3x,
        .ibat_ratio = IBatRatio::_6x
    }).reconfInterruptMask({
        .eoc = true,
        .otp = true,
        .vbus_short = true,
        .indet = true,
        .ac_ok = true
    })
    ;

    // setBatCurrLimit(1.2_r);
    // setBusCurrLimit(1.2_r);
    // setVinRegRefVolt(12);
    // enableOtg(false);

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

    return real_t(inv_b10(adin_value_reg, 1)) / 1000;
}

SC8815 & SC8815::setBusCurrLimit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x0c) == 4) ? 6 : 3;
    ibus_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    WRITE_REG(ibus_lim_set_reg);

    return *this;
}

SC8815 & SC8815::setBatCurrLimit(const real_t limit_ma){
    const auto ratio = ((ratio_reg & 0x10) == 16) ? 12 : 6;
    ibat_lim_set_reg = uint16_t(
        16 * (limit_ma * 1000) * bus_shunt_res_mohms_ / (625 * ratio) - 1
    );

    WRITE_REG(ibat_lim_set_reg);

    return *this;
}

SC8815 & SC8815::setOutputVolt(const real_t volt){

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

    return *this;
}

SC8815 & SC8815::setInternalVbusRef(const real_t volt){
    vbus_ref_i_set_reg = b10(int(volt * 1000), 2);
    WRITE_REG(vbus_ref_i_set_reg)
    return *this;
}

SC8815 & SC8815::setExternalVbusRef(const real_t volt){
    vbus_ref_e_set_reg = b10(int(volt * 1000), 2);
    WRITE_REG(vbus_ref_e_set_reg)
    return *this;
}


SC8815 & SC8815::setIBatLimRatio(){
    return *this;
}

SC8815 & SC8815::enableOtg(const bool en){
    ctrl0_set_reg.en_otg = en;
    WRITE_REG(ctrl0_set_reg)
    return *this;
}

SC8815 & SC8815::enableTrikleCharge(const bool en){
    ctrl1_set_reg.dis_trickle = !en;
    WRITE_REG(ctrl1_set_reg)
    return *this;
}

SC8815 & SC8815::enableOvpProtect(const bool en){
    return *this;
}

SC8815 & SC8815::powerUp(){
    ctrl2_set_reg.factory = 1;//Factory setting bit. MCU shall write this bit to 1 after power up.
    WRITE_REG(ctrl2_set_reg);
    return*this;
}

SC8815 & SC8815::enableDither(const bool en){
    ctrl2_set_reg.en_dither = en;
    WRITE_REG(ctrl2_set_reg);
    return *this;
}

SC8815 & SC8815::enableAdcConv(const bool en){
    ctrl3_set_reg.ad_start = en;
    WRITE_REG(ctrl3_set_reg);
    return *this;
}

SC8815 & SC8815::enablePfmMode(const bool en){
    ctrl3_set_reg.en_pfm = en;
    WRITE_REG(ctrl3_set_reg);
    return*this;
}

SC8815 & SC8815::enableSFB(const bool en){
    ctrl3_set_reg.dis_shortfoldback = !en;
    WRITE_REG(ctrl3_set_reg);
    return *this;
}

SC8815 & SC8815::enableGpo(const bool en){
    ctrl3_set_reg.gpo_ctrl = en;
    WRITE_REG(ctrl3_set_reg);
    return *this;
}

SC8815 & SC8815::enablePgate(const bool en){
    ctrl3_set_reg.en_pgate = en;
    WRITE_REG(ctrl3_set_reg);
    return *this;
}

SC8815 & SC8815::reconfBat(const BatConfig & config){
    this -> setBatVolt(config.vcell_set);
    this -> setBatIrComp(config.ircomp);
    this -> setBatCells(config.csel);
    this -> enableVbatUseExtneral(config.use_ext_setting);

    return *this;
}

SC8815::Interrupts SC8815::interrupts(){
    READ_REG(mask_reg);
    return static_cast<Interrupts>(mask_reg);
}

SC8815 & SC8815::reconfInterruptMask(const Interrupts mask){
    mask_reg = std::bit_cast<uint8_t>(mask);
    WRITE_REG(mask_reg);
    return *this;
}



SC8815 & SC8815::setBatVolt(const BatVoltType bat_volt){
    vbat_set_reg.vcell_set = uint8_t(bat_volt);
    WRITE_REG(vbat_set_reg)
    return *this;
}

SC8815 & SC8815::setBatCells(const BatCellsType bat_cells){
    vbat_set_reg.vcell_set = uint8_t(bat_cells);
    WRITE_REG(vbat_set_reg)
    return *this;
}

SC8815 & SC8815::setBatCells(const uint bat_cells){
    if(bat_cells > 4) SC8815_PANIC();
    setBatCells(BatCellsType(bat_cells));
    return *this;
}

SC8815 & SC8815::enableVbatUseExtneral(const bool use){
    vbat_set_reg.vbat_sel = use;
    WRITE_REG(vbat_set_reg)
    return *this;
}

SC8815 & SC8815::setBatIrComp(const BatIrCompType bat_ir_comp){
    vbat_set_reg.ircomp = uint8_t(bat_ir_comp);
    WRITE_REG(vbat_set_reg)
    return *this;
}

SC8815 & SC8815::setIBatRatio(const IBatRatio ratio){
    ratio_reg.ibat_ratio = uint8_t(ratio);
    WRITE_REG(ratio_reg);
    return *this;
}

SC8815 & SC8815::setIBusRatio(const IBusRatio ratio){
    ratio_reg.ibus_ratio = uint8_t(ratio);
    WRITE_REG(ratio_reg);
    return *this;
}

SC8815 & SC8815::setVBatMonRatio(const VBatMonRatio ratio){
    ratio_reg.vbat_mon_ratio = uint8_t(ratio);
    WRITE_REG(ratio_reg);
    return *this;
}

SC8815 & SC8815::setVBusRatio(const VBusRatio ratio){
    ratio_reg.vbus_ratio = uint8_t(ratio);
    WRITE_REG(ratio_reg);
    return *this;
}

SC8815 & SC8815::reconfRatio(const RatioConfig & config){
    this->setIBatRatio(config.ibat_ratio);
    this->setIBusRatio(config.ibus_ratio);
    this->setVBatMonRatio(config.vbat_mon_ratio);
    this->setVBusRatio(config.vbus_ratio);

    return *this;
}