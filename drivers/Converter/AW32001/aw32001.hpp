#pragma once

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
    [[nodiscard]] IResult<> write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const RegAddr address, uint8_t & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(address), reg);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(T::ADDRESS, reg.as_bits_mut());
    }
};


}