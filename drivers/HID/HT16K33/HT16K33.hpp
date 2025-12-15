#pragma once

//这个驱动已经完成

//HT16K33是一款使用标准I2C协议的Led矩阵扫描/按键扫描芯片 同时带有中断引脚

#include "ht16k33_prelude.hpp"

namespace ymd::drivers{

class HT16K33 final:public HT16K33_Regs{
public:
    template<typename Set>
    explicit HT16K33(Set && set, const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv, None)
    {
        resetting(std::forward<Set>(set));
    }

    template<typename Set>
    explicit HT16K33(Set && set, 
        const hal::I2cDrv & i2c_drv, 
        const hal::GpioIntf & int_input_gpio)
    :
        phy_(i2c_drv, Some(hal::InterruptInput(int_input_gpio)))
    {
        resetting(std::forward<Set>(set));
    }
    
    template<typename Set>
    void resetting(Set && set){
        package_ = set.PACKAGE;
    }

    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<bool> is_any_key_pressed();
    
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_int_pin_func(const IntPinFunc func);

    [[nodiscard]] IResult<BoolLevel> get_intreg_status();

    [[nodiscard]] IResult<> update_displayer(const GcRam & gc_ram){
        return update_displayer(0,gc_ram.as_bytes());
    }


    [[nodiscard]] IResult<> update_displayer(
        const size_t offset, std::span<const uint8_t> pbuf);


    [[nodiscard]] IResult<> clear_displayer();

    [[nodiscard]] IResult<KeyData> get_key_data();
private:
    using Phy = HT16K33_Phy;
    Phy phy_;
    Package package_;

    [[nodiscard]] IResult<> write_command(const Command cmd);

    [[nodiscard]] IResult<> setup_system(const Enable en);

    [[nodiscard]] IResult<> setup_displayer(const BlinkFreq freq, const Enable en);

    [[nodiscard]] IResult<> set_pulse_duty(const PulseDuty duty);

};
}