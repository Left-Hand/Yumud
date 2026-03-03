#include "axp192_prelude.hpp"


using namespace ymd;
using namespace ymd::drivers;

using Self = AXP192;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

#if 0

IResult<> Self::set_dcdc1_voltage(const DcdcVoltage voltage){
    auto reg = RegCopy(pw_regs_.dcdc1_voltage_setting_reg);
    reg.dcdc1_voltage_setting = voltage.to_original();
    return write_reg(reg);
}
IResult<> Self::set_dcdc2_voltage(const DcdcVoltage voltage){
    auto reg = RegCopy(pw_regs_.dcdc2_voltage_setting_reg);
    reg.dcdc2_voltage_setting = voltage.to_original();
    return write_reg(reg);
}
IResult<> Self::set_dcdc3_voltage(const DcdcVoltage voltage){
    auto reg = RegCopy(pw_regs_.dcdc3_voltage_setting_reg);
    reg.dcdc3_voltage_setting = voltage.to_original();
    return write_reg(reg);
}
#endif