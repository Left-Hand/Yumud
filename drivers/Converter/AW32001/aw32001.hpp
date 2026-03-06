#pragma once

// 驱动参考代码:
// https://wiki.lckfb.com/zh-hans/hspi-sf32lb52/lckfb-hspi-sf32lb52/iic.html

// 官方介绍(datasheet @page1)：
// AW32001A是一款高度集成的锂离子/锂聚合物电池线性充电器，
// 具有系统电源路径管理功能。AW32001A的充电过程包括：
// 预充电、快速充电和恒压调节。充电参数和工作模式可以通过I2C接口进行编程设置。
// 充电过程自动运行，当电池电压在充电完成状态后下降至VBAT_REG-VRECH以下时，
// 会重新开始充电。

// AW32001A专为空间受限的便携式应用而设计。
// 芯片可以从交流适配器或USB端口获取输入电源，
// 为系统负载供电并对电池进行充电。同时，
// 芯片通过限制从输入到系统以及从电池到系统的电流来提供系统短路保护功能。
// 这些特性有效保护电池或芯片免受损坏。输入电流限制、
// 放电电流限制和安全定时器等参数都可以通过I2C接口进行编程。

// 此外，还集成了输入过压保护、输入欠压闭锁和输入余量电压检测等功能，以实现良好的输入源检测。
// AW32001A将充电路径与系统电源供应分离，以实现电源管理功能。
// 系统电源供应具有最高优先级，不依赖于电池是否存在。
// 当输入端出现不良的功率受限适配器时，AW32001A会首先减少充电电流
// 如果系统负载对于输入源来说仍然过重，AW32001A将进一步减少输入到系统的电流，
// 以防止输入源被拉低。在这种情况下，如果系统电压比电池电压低30mV，
// 则电池到系统的供电路径将完全导通以为系统负载供电，这称为补充模式。

#include "aw32001_prelude.hpp"


namespace ymd::drivers{


struct AW32001 final:public AW32001_Prelude{ 
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34);

    explicit AW32001(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){}

    explicit AW32001(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}

    explicit AW32001(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    IResult<> set_charge_current(const ChargeCurrent current){
        auto reg = RegCopy(regs_.charge_current_control_reg);
        reg.ichg = current.to_bits();
        return write_reg(reg);
    }

    // Set input current limit
    IResult<> set_input_current_limit(const IIN_LIM limit) {
        auto reg = RegCopy(regs_.intput_source_control_reg);
        reg.iin_lim = limit;
        return write_reg(reg);
    }

    // Set VIN DPM voltage
    IResult<> set_vin_dpm(const VIN_DPM voltage) {
        auto reg = RegCopy(regs_.intput_source_control_reg);
        reg.vin_dpm = voltage;
        return write_reg(reg);
    }

    // Set charge voltage
    IResult<> set_charge_voltage(const ChargeVoltage voltage) {
        auto reg = RegCopy(regs_.charge_voltage_reg);
        reg.vbat_reg = voltage.to_bits();
        return write_reg(reg);
    }

    // Set discharge current
    IResult<> set_discharge_current(const DischargeCurrent current) {
        auto reg = RegCopy(regs_.discharge_current_control_reg);
        reg.idischg = current.to_bits();
        return write_reg(reg);
    }

    // Set termination current
    IResult<> set_termination_current(const TerminationCurrent current) {
        auto reg = RegCopy(regs_.discharge_current_control_reg);
        reg.iterm = current.to_bits();
        return write_reg(reg);
    }

    // Set system voltage
    IResult<> set_system_voltage(const VsysReg voltage) {
        auto reg = RegCopy(regs_.system_voltage_reg);
        reg.vsys_reg = voltage.to_bits();
        return write_reg(reg);
    }

    // Set watchdog timer
    IResult<> set_watchdog_timer(const WatchdogTime time) {
        auto reg = RegCopy(regs_.timer_control_reg);
        reg.watchdog = time;
        return write_reg(reg);
    }

    // Set fast charge time
    IResult<> set_fast_charge_time(const FastChargeTime time) {
        auto reg = RegCopy(regs_.timer_control_reg);
        reg.chg_time = time;
        return write_reg(reg);
    }
private:
    hal::I2cDrv i2c_drv_;
    AW32001_Regset regs_ = {};
    IResult<> write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};


}