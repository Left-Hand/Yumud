#pragma once

// 这个驱动正在开发中

// AXP192 是高度集成的电源系统管理芯片，针对单芯锂电池(锂离子或锂聚合物)且需要多路电源转换
// 输出的应用，提供简单易用而又可以灵活配置的完整电源解决方案，充分满足目前日益复杂的应用处理
// 器系统对于电源相对复杂而精确控制的要求。 
// AXP192 内部集成了一个自适应的 USB-Compatible 的充电器，3 路降压转换器(Buck DC-DC 
// converter)，4 路线性稳压器(LDO)，电压/电流/温度监视等多路12-Bit ADC。为保证电源系统安全稳定，
// AXP192 还整合了过/欠压(OVP/UVP)、过温(OTP)、过流(OCP)等保护电路。

#include "axp192_prelude.hpp"

namespace ymd::drivers{

class AXP192 final: public AXP192_Prelude{

    explicit AXP192(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){}

    explicit AXP192(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}

    explicit AXP192(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){}

    IResult<> set_dcdc1_voltage(const DcdcVoltage voltage);
    IResult<> set_dcdc2_voltage(const DcdcVoltage voltage);
    IResult<> set_dcdc3_voltage(const DcdcVoltage voltage);
private:
    hal::I2cDrv i2c_drv_;
    AXP192_PowerCtl_Regset pw_regs_ = {};
    AXP192_GpioCtl_RegSet gp_regs_ = {};

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

    IResult<> read_bulk(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_bulk(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    #if 0
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
    #endif
};

}