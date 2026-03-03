#pragma once

// AXP313A 是一款简化版的电源管理芯片，主要提供DC-DC转换功能
// 相比AXP192，AXP313A功能更加精简，主要用于基础的电源转换需求

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/enum/strong_type_gradation.hpp"

namespace ymd::drivers{

struct AXP313A_Prelude{
    enum class ErrorKind:uint8_t{
        NotReady,
        UnexpectedProductId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34);

// 寄存器地址枚举，严格对应AXP313A手册6.7.1寄存器列表地址
enum class RegAddr : uint8_t
{
    PowerOnSourceInd = 0x00,    // 00H 开机源指示寄存器 R
    PowerOutputCtrl  = 0x10,    // 10H 电源输出开关控制寄存器 R/W
    DcdcPwmCtrl      = 0x12,    // 12H DCDC PWM控制寄存器 R/W
    Dcdc1VoltSet     = 0x13,    // 13H DCDC1电压设置寄存器 R/W
    Dcdc2VoltSet     = 0x14,    // 14H DCDC2电压设置寄存器 R/W
    Dcdc3VoltSet     = 0x15,    // 15H DCDC3电压设置寄存器 R/W
    Aldo1VoltSet     = 0x16,    // 16H ALDO1电压设置寄存器 R/W
    Dldo1VoltSet     = 0x17,    // 17H DLDO1电压设置寄存器 R/W
    PowerOffRstCtrl  = 0x1A,    // 1AH 关机、重启控制寄存器 R/W
    PowerOffSeqCtrl  = 0x1B,    // 1BH 关机时序、按键关机控制寄存器 R/W
    PowerWakeCtrl    = 0x1C,    // 1CH 电源唤醒控制寄存器 R/W
    OutputMonCtrl    = 0x1D,    // 1DH 输出监视控制寄存器 R/W
    PokParamSet      = 0x1E,    // 1EH POK参数设置寄存器 R/W
    IrqCtrl          = 0x20,    // 20H IRQ控制寄存器 R/W
    IrqStatus        = 0x21     // 21H IRQ状态寄存器 R/W
};

};


struct AXP313A_PowerCtl_Regset:public AXP313A_Prelude{ 



// 00H 开机源指示寄存器 R
struct R8_PowerOnSourceInd : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PowerOnSourceInd;
    // VIN从低到高开机 0:否 1:是
    uint8_t vin_rise_poweron:1;
    // 保留位
    uint8_t reserved1:1;
    // PWRON按键开机 0:否 1:是
    uint8_t pwrbtn_poweron:1;
    // IRQ低电平开机 0:否 1:是
    uint8_t irq_low_poweron:1;
    // VIN高时EN上升/EN高时VIN上升开机 0:否 1:是
    uint8_t vin_en_rise_poweron:1;
    // 保留位
    uint8_t reserved5_7:3;
}DEF_R8(power_on_source_ind_reg);

// 10H 电源输出开关控制寄存器 R/W 0:关闭 1:打开
struct R8_PowerOutputCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PowerOutputCtrl;
    // DCDC1开关控制 定制默认值
    uint8_t dcdc1_en:1;
    // DCDC2开关控制 定制默认值
    uint8_t dcdc2_en:1;
    // DCDC3开关控制 定制默认值
    uint8_t dcdc3_en:1;
    // ALDO1开关控制 定制默认值
    uint8_t aldo1_en:1;
    // DLDO1开关控制 定制默认值
    uint8_t dldo1_en:1;
    // 保留位 默认为0
    uint8_t reserved5_7:3;
}DEF_R8(power_output_ctrl_reg);

// 12H DCDC PWM控制寄存器 R/W 默认00H
struct R8_DcdcPwmCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::DcdcPwmCtrl;
    // DCDC1 PFM/PWM 0:自动切换 1:固定PWM 默0
    uint8_t dcdc1_pwm_fix:1;
    // DCDC2 PFM/PWM 0:自动切换 1:固定PWM 默0
    uint8_t dcdc2_pwm_fix:1;
    // DCDC3 PFM/PWM 0:自动切换 1:固定PWM 默0
    uint8_t dcdc3_pwm_fix:1;
    // 保留位 默0
    uint8_t reserved3_4:2;
    // 保留位 默0
    uint8_t reserved5:1;
    // DCDC展频频率 0:50kHz 1:100kHz 默0
    uint8_t dcdc_spread_freq:1;
    // DCDC展频功能 0:关闭 1:打开 默0
    uint8_t dcdc_spread_en:1;
}DEF_R8(dcdc_pwm_ctrl_reg);

// 13H DCDC1电压设置寄存器 R/W 定制默认值
struct R8_Dcdc1VoltSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::Dcdc1VoltSet;
    // DCDC1电压设置位 0.5~3.4V 多档位步进 定制默认值
    uint8_t dcdc1_volt:7;
    // 保留位 默0
    uint8_t reserved7:1;
}DEF_R8(dcdc1_volt_set_reg);

// 14H DCDC2电压设置寄存器 R/W 定制默认值
struct R8_Dcdc2VoltSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::Dcdc2VoltSet;
    // DCDC2电压设置位 0.5~1.54V 多档位步进 定制默认值
    uint8_t dcdc2_volt:7;
    // 保留位 默0
    uint8_t reserved7:1;
}DEF_R8(dcdc2_volt_set_reg);

// 15H DCDC3电压设置寄存器 R/W 定制默认值
struct R8_Dcdc3VoltSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::Dcdc3VoltSet;
    // DCDC3电压设置位 0.5~1.84V 多档位步进 定制默认值
    uint8_t dcdc3_volt:7;
    // 保留位 默0
    uint8_t reserved7:1;
}DEF_R8(dcdc3_volt_set_reg);

// 16H ALDO1电压设置寄存器 R/W 定制默认值
struct R8_Aldo1VoltSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::Aldo1VoltSet;
    // ALDO1电压设置位 0.5~3.5V 100mV/step 定制默认值
    uint8_t aldo1_volt:5;
    // 保留位 默0
    uint8_t reserved5_7:3;
}DEF_R8(aldo1_volt_set_reg);

// 17H DLDO1电压设置寄存器 R/W 定制默认值
struct R8_Dldo1VoltSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::Dldo1VoltSet;
    // DLDO1电压设置位 0.5~3.5V 100mV/step 定制默认值
    uint8_t dldo1_volt:5;
    // 保留位 默0
    uint8_t reserved5_7:3;
}DEF_R8(dldo1_volt_set_reg);

// 1AH 关机、重启控制寄存器 R/W 默认20H(0x20)
struct R8_PowerOffRstCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PowerOffRstCtrl;
    // 温度保护阈值 0:125℃ 1:145℃ 默0
    uint8_t temp_prot_th:1;
    // 过温关机功能 0:不关机 1:关机 默0
    uint8_t otp_poweroff_en:1;
    // 保留位 默0
    uint8_t reserved2_3:2;
    // PWROK拉低重启 0:不重启 1:重启 默0
    uint8_t pwrok_low_rst_en:1;
    // 启动监控PWROK 0:不监控 1:监控 默1
    uint8_t pwrok_mon_en:1;
    // 软件重启控制 写1重启 自动清零 默0
    uint8_t sw_rst:1;
    // 软件关机控制 写1关机 自动清零 默0
    uint8_t sw_poweroff:1;
}DEF_R8(power_off_rst_ctrl_reg);

// 1BH 关机时序、按键关机控制寄存器 R/W 默认06H(0x06)
struct R8_PowerOffSeqCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PowerOffSeqCtrl;
    // 按键关机后重启 0:不重启 1:重启 默0
    uint8_t pwrbtn_off_rst:1;
    // 按键超时时长关机 0:不关机 1:关机 默1
    uint8_t pwrbtn_off_en:1;
    // 关机PWROK延时4ms 0:不延时 1:延时 默1
    uint8_t poweroff_pwrok_dly:1;
    // 输出关闭时序 0:同时关闭 1:反开机时序 默0
    uint8_t poweroff_seq:1;
    // 保留位 默0
    uint8_t reserved4_7:4;
}DEF_R8(power_off_seq_ctrl_reg);

// 1CH 电源唤醒控制寄存器 R/W 默认00H
struct R8_PowerWakeCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PowerWakeCtrl;
    // Sleep唤醒使能 0:关闭 1:打开 默0
    uint8_t sleep_wake_en:1;
    // 软件唤醒控制 写1唤醒 自动清零 默0
    uint8_t sw_wake:1;
    // 唤醒电压恢复 0:恢复默认 1:寄存器值 默0
    uint8_t wake_volt_restore:1;
    // 唤醒PWROK拉低 0:不拉低 1:拉低 默0
    uint8_t wake_pwrok_low:1;
    // IRQ低电平唤醒 0:关闭 1:打开 默0
    uint8_t irq_low_wake_en:1;
    // 保留位 默0
    uint8_t reserved5_7:3;
}DEF_R8(power_wake_ctrl_reg);

// 1DH 输出监视控制寄存器 R/W 默认1FH(0x1F)
struct R8_OutputMonCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::OutputMonCtrl;
    // DCDC过压关机 0:关闭 1:打开 默1
    uint8_t dcdc_ovp_poweroff_en:1;
    // DCDC1欠压关机 0:关闭 1:打开 默1
    uint8_t dcdc1_uvp_poweroff_en:1;
    // DCDC2欠压关机 0:关闭 1:打开 默1
    uint8_t dcdc2_uvp_poweroff_en:1;
    // DCDC3欠压关机 0:关闭 1:打开 默1
    uint8_t dcdc3_uvp_poweroff_en:1;
    // DLDO1过流关机 0:关闭 1:打开 默1
    uint8_t dldo1_ocp_poweroff_en:1;
    // 保留位 默0
    uint8_t reserved5_7:3;
}DEF_R8(output_mon_ctrl_reg);

// 1EH POK参数设置寄存器 R/W 默认80H(0x80)
struct R8_PokParamSet : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::PokParamSet;
    // IRQLEVEL设置 0:1.5s 1:2.5s 默0
    uint8_t irq_level:1;
    // OFFLEVEL设置 0:6s 1:10s 默0
    uint8_t off_level:1;
    // 保留位 默0
    uint8_t reserved2_6:5;
    // 保留位 默1
    uint8_t reserved7:1;
}DEF_R8(pok_param_set_reg);

// 20H IRQ控制寄存器 R/W 默认31H(0x31)
struct R8_IrqCtrl : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::IrqCtrl;
    // 芯片过温IRQ使能 0:关闭 1:打开 默1
    uint8_t otp_irq_en:1;
    // 保留位 默0
    uint8_t reserved1:1;
    // DCDC2欠压IRQ使能 0:关闭 1:打开 默0
    uint8_t dcdc2_uvp_irq_en:1;
    // DCDC3欠压IRQ使能 0:关闭 1:打开 默0
    uint8_t dcdc3_uvp_irq_en:1;
    // PWRON长按IRQ使能 0:关闭 1:打开 默1
    uint8_t pwrbtn_long_irq_en:1;
    // PWRON短按IRQ使能 0:关闭 1:打开 默1
    uint8_t pwrbtn_short_irq_en:1;
    // PWRON下降沿IRQ使能 0:关闭 1:打开 默0
    uint8_t pwrbtn_fall_irq_en:1;
    // PWRON上升沿IRQ使能 0:关闭 1:打开 默0
    uint8_t pwrbtn_rise_irq_en:1;
}DEF_R8(irq_ctrl_reg);

// 21H IRQ状态寄存器 R/W 默认00H 写1清零对应位
struct R8_IrqStatus : public Reg8<>
{
    static constexpr RegAddr REG_ADDR = RegAddr::IrqStatus;
    // 芯片过温IRQ状态 写1/温度恢复清零 默0
    uint8_t otp_irq_sta:1;
    // 保留位 默0
    uint8_t reserved1:1;
    // DCDC2欠压IRQ状态 写1/电压恢复清零 默0
    uint8_t dcdc2_uvp_irq_sta:1;
    // DCDC3欠压IRQ状态 写1/电压恢复清零 默0
    uint8_t dcdc3_uvp_irq_sta:1;
    // PWRON长按IRQ状态 写1清零 默0
    uint8_t pwrbtn_long_irq_sta:1;
    // PWRON短按IRQ状态 写1清零 默0
    uint8_t pwrbtn_short_irq_sta:1;
    // PWRON下降沿IRQ状态 写1清零 默0
    uint8_t pwrbtn_fall_irq_sta:1;
    // PWRON上升沿IRQ状态 写1清零 默0
    uint8_t pwrbtn_rise_irq_sta:1;
}DEF_R8(irq_status_reg);

};

}