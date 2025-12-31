#pragma once

// 评价：低廉且功能丰富，猜测是电源IC和MCU合封

// 这个驱动正在开发中


// 官方介绍：AXP192 是高度集成的电源系统管理芯片，针对单芯锂电池(锂离子或锂聚合物)且需要多路电源转换
// 输出的应用，提供简单易用而又可以灵活配置的完整电源解决方案，充分满足目前日益复杂的应用处理
// 器系统对于电源相对复杂而精确控制的要求。 
// AXP192 内部集成了一个自适应的 USB-Compatible 的充电器，3 路降压转换器(Buck DC-DC 
// converter)，4 路线性稳压器(LDO)，电压/电流/温度监视等多路12-Bit ADC。为保证电源系统安全稳定，
// AXP192 还整合了过/欠压(OVP/UVP)、过温(OTP)、过流(OCP)等保护电路。 
// AXP192 的智慧电能平衡(Intelligent Power Select, IPS™)电路可以在 USB 以及外部交流适配器、锂电
// 池和应用系统负载之间安全透明的分配电能，并且在只有外部输入电源而没有电池(或者电池过放/损坏)
// 的情况下也可以使应用系统正常工作。 
// AXP192 具有外部适配器和USB以及电池等三输入能力，支持可充电备用电池。 
// AXP192 提供了一个与主机通讯的两线串行通讯接口:Two Wire Serial Interface (TWSI)，应用处理器可
// 以通过这个接口去打开或关闭某些电源输出，设置它们的电压，访问内部寄存器和多种测量数据(包括Fuel 
// Gauge)。高精度（0.5%）的电量测量数据方便消费者更清楚的实时掌握电能使用状况，给消费者带来前
// 所未有的设备电能使用体验。 
//  AXP192 提供 6mm x 6mm 48-pin QFN 封装。

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/strong_type_gradation.hpp"

namespace ymd::drivers{

struct AXP192_Prelude{
    enum class ErrorKind:uint8_t{
        NotReady,
        UnexpectedProductId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34);

    using RegAddr = uint8_t;

    enum class PowerOnTime:uint8_t{
        // 0: 128mS； 01: 512mS；  
        // 10: 1S；     11: 2S. 
        _128ms = 0b00,
        _512ms = 0b01,
        _1s = 0b10,
        _2s = 0b11
    };

    DEF_U8_STRONG_TYPE_GRADATION(DcdcVoltage, from_mv, 
        uint32_t, 700, 2275, 25)
    DEF_U8_STRONG_TYPE_GRADATION(Ldo2Voltage, from_mv, 
        uint32_t, 1800, 3300, 100)
    DEF_U8_STRONG_TYPE_GRADATION(Ldo3Voltage, from_mv, 
        uint32_t, 1800, 3300, 100)

    DEF_U8_STRONG_TYPE_GRADATION(DcDcSwitchingFrequency, from_hz, 
        uint32_t, 1'500000, 2'625000, 75000)


    enum class VbusCurrentLimit:uint8_t{
        _500mA = 0b0,
        _100mA = 0b1
    };

    enum class VholdVoltage:uint8_t{
        _4_0V = 0b000,
        _4_1V = 0b001,
        _4_2V = 0b010,
        _4_3V = 0b011,
        _4_4V = 0b100,
        _4_5V = 0b101,
        _4_6V = 0b110,
        _4_7V = 0b111
    };



    enum class VoffVoltage:uint8_t{
        _2_6V = 0b000,
        _2_7V = 0b001,
        _2_8V = 0b010,
        _2_9V = 0b011,
        _3_0V = 0b100,
        _3_1V = 0b101,
        _3_2V = 0b110,
        _3_3V = 0b111
    };
};

struct AXP192_PowerCtl_Regset:public AXP192_Prelude{ 
    // 地址         寄存器描述 R/W 默认值 
    // 00           电源状态寄存器 R  
    // 01           电源模式/充电状态寄存器 R  
    // 04           OTG VBUS状态寄存器 R  
    // 06-0B        数据缓存寄存器 0-5 R/W F0/0F/00/FF/
    // 00/00 
    // 10           EXTEN & DC-DC2开关控制寄存器 R/W X5H 
    // 12           DC-DC1/3 & LDO2/3开关控制寄存器 R/W XFH 
    // 23           DC-DC2电压设置寄存器 R/W 16H 
    // 25           DC-DC2电压斜率参数设置寄存器 R/W 00H 
    // 26           DC-DC1电压设置寄存器 R/W 68H 
    // 27           DC-DC3电压设置寄存器 R/W 48H 
    // 28           LDO2/3电压设置寄存器 R/W CFH 
    // 30           VBUS-IPSOUT通路设置寄存器 R/W 60H 
    // 31           VOFF关机电压设置寄存器 R/W X3H 
    // 32           关机、电池检测、CHGLED控制寄存器 R/W 46H 
    // 33           充电控制寄存器1 R/W C8H 
    // 34           充电控制寄存器2 R/W 41H 
    // 35           备用电池充电控制寄存器 R/W 22H  
    // 36           PEK参数设置寄存器 R/W 5DH 
    // 37           DCDC转换器工作频率设置寄存器 R/W 08H 
    // 38           电池充电低温报警设置寄存器 R/W A5H 
    // 39           电池充电高温报警设置寄存器 R/W 1FH 
    // 3A           APS低电Level1设置寄存器 R/W 68H 
    // 3B           APS低电Level2设置寄存器 R/W 5FH 
    // 3C           电池放电低温报警设置寄存器 R/W FCH 
    // 3D           电池放电高温报警设置寄存器 R/W 16H 
    // 80           DCDC工作模式设置寄存器 R/W E0H 
    // 82           ADC使能设置寄存器1 R/W 83H 
    // 83           ADC使能设置寄存器2 R/W 80H 
    // 84           ADC 采样率设置，TS pin控制寄存器 R/W 32H 
    // 85           GPIO [3:0]输入范围设置寄存器 R/W X0H 
    // 86           GPIO1 ADC IRQ上升沿门限设置 R/W FFH 
    // 87           GPIO1 ADC IRQ下降沿门限设置 R/W 00H 
    // 8A           定时器控制寄存器 R/W 00H 
    // 8B           VBUS监测设置寄存器 R/W 00H 
    // 8F           过温关机控制寄存器 R/W 01H 

    // 00           电源状态寄存器 R  


    struct R8_PowerInputState:public Reg8<>{
        // 指示启动源是否为ACIN或VBUS 
        // 0:启动源非ACIN/VBUS； 1:启动源为 ACIN/VBUS 
        uint8_t setup_source_is_acin_or_vbus:1;

        // 示ACIN和VBUS输入是否在PCB被短接 
        uint8_t is_acin_or_vbus_short_circuit:1;

        // 指示电池电流方向 
        // 0:电池在放电；1:电池被充电
        uint8_t battery_is_charging_either_discharge:1;

        // 指示VBUS接入在使用之前是否大于 VHOLD 
        uint8_t is_vbus_great_than_vhold:1;

        // 指示VBUS是否可用 
        uint8_t is_vbus_available:1;
        
        // 指示VBUS输入是否存在
        uint8_t is_vbus_exist:1;
        
        // 指示ACIN是否可用 
        uint8_t is_acin_available:1;
        
        // 指示ACIN输入是否存在
        uint8_t is_acin_exist:1;
    }DEF_R8(power_input_state_reg)
    // 01           电源模式/充电状态寄存器 R  

    struct R8_PowerModeChargeState:public Reg8<>{
        uint8_t :1;
        uint8_t powerdown_strategy_is_b_either_a:1;
        uint8_t is_charging_current_less_than_expected:1;
        uint8_t is_on_battery_activate_mode:1;
        uint8_t :1;
        uint8_t is_battery_present:1;
        uint8_t is_charing:1;
        uint8_t is_overheat:1;
    }DEF_R8(power_mode_charge_state_reg)
    // 04           OTG VBUS状态寄存器 R  

    struct R8_UsbOtgVbusState:public Reg8<>{
        uint8_t is_session_end:1;
        uint8_t is_vbus_session_valid:1;
        uint8_t is_vbus_valid:1;
        uint8_t :5;
    }DEF_R8(usb_otg_vbus_state_reg)

    // 10           EXTEN & DC-DC2开关控制寄存器 R/W X5H 

    struct R8_ExtenDcDc2SwitchCtrl:public Reg8<>{
        uint8_t dcdc2_en:1;
        uint8_t :1;
        uint8_t ext_en:1;
        uint8_t :5;
    }DEF_R8(exten_dcdc2_switch_ctrl_reg)
    // 12           DC-DC1/3 & LDO2/3开关控制寄存器 R/W XFH 

    struct R8_PowerOutputCtrl:public Reg8<>{
        uint8_t dcdc1_en:1;
        uint8_t dcdc3_en:1;
        uint8_t ldo2_en:1;
        uint8_t ldo3_en:1;
        uint8_t dcdc2_en:1;
        uint8_t :1;
        uint8_t ext_en:1;
        uint8_t :1;
    }DEF_R8(power_output_ctrl_reg)

    // 23           DC-DC2电压设置寄存器 R/W 16H 


    struct R8_DcDc2VoltageSetting:public Reg8<>{
        uint8_t dcdc2_voltage_setting:7;
        uint8_t :1;
    }DEF_R8(dcdc2_voltage_setting_reg)
    // 25           DC-DC2电压斜率参数设置寄存器 R/W 00H 
    // 26           DC-DC1电压设置寄存器 R/W 68H 


    struct R8_DcDc1VoltageSetting:public Reg8<>{
        uint8_t dcdc1_voltage_setting:7;
        uint8_t :1;
    }DEF_R8(dcdc1_voltage_setting_reg)

    // 27           DC-DC3电压设置寄存器 R/W 48H 

    struct R8_DcDc3VoltageSetting:public Reg8<>{
        uint8_t dcdc3_voltage_setting:7;
        uint8_t :1;
    }DEF_R8(dcdc3_voltage_setting_reg)

    // 28           LDO2/3电压设置寄存器 R/W CFH 

    struct R8_Ldo23VoltageSetting:public Reg8<>{
        uint8_t ldo3_voltage_setting:4;
        uint8_t ldo2_voltage_setting:4;
    }DEF_R8(ldo23_voltage_setting_reg)

    // 30           VBUS-IPSOUT通路设置寄存器 R/W 60H 

    struct R8_VbusIpsoutRaouteManage:public Reg8<>{
        VbusCurrentLimit vbus_current_limit:1;
        uint8_t vbus_current_limit_en:1;
        uint8_t :1;
        VholdVoltage vhold_setting:3;
        uint8_t vhold_voltage_limit_en:1;
        uint8_t is_vbus_ipsout_en_nomatter_v_busen:1;
    }DEF_R8(vbus_ipsout_route_manage_reg)

    // 31           VOFF关机电压设置寄存器 R/W X3H 
    // 32           关机、电池检测、CHGLED控制寄存器 R/W 46H 
    // 33           充电控制寄存器1 R/W C8H 
    // 34           充电控制寄存器2 R/W 41H 
    // 35           备用电池充电控制寄存器 R/W 22H  
    // 36           PEK参数设置寄存器 R/W 5DH 
    // 37           DCDC转换器工作频率设置寄存器 R/W 08H 
    // 38           电池充电低温报警设置寄存器 R/W A5H 
    // 39           电池充电高温报警设置寄存器 R/W 1FH 
    // 3A           APS低电Level1设置寄存器 R/W 68H 
    // 3B           APS低电Level2设置寄存器 R/W 5FH 
    // 3C           电池放电低温报警设置寄存器 R/W FCH 
    // 3D           电池放电高温报警设置寄存器 R/W 16H 
    // 80           DCDC工作模式设置寄存器 R/W E0H 
    // 82           ADC使能设置寄存器1 R/W 83H 
    // 83           ADC使能设置寄存器2 R/W 80H 
    // 84           ADC 采样率设置，TS pin控制寄存器 R/W 32H 
    // 85           GPIO [3:0]输入范围设置寄存器 R/W X0H 
    // 86           GPIO1 ADC IRQ上升沿门限设置 R/W FFH 
    // 87           GPIO1 ADC IRQ下降沿门限设置 R/W 00H 
    // 8A           定时器控制寄存器 R/W 00H 
    // 8B           VBUS监测设置寄存器 R/W 00H 
    // 8F           过温关机控制寄存器 R/W 01H 
};


struct AXP192_GpioCtl_RegSet:public AXP192_Prelude{

    // 90           GPIO0控制寄存器 R/W 07H 
    // 91           GPIO0 LDO模式输出电压设置寄存器 R/W A0H 
    // 92           GPIO1控制寄存器 R/W 07H 
    // 93           GPIO2控制寄存器 R/W 07H 
    // 94           GPIO[2:0]信号状态寄存器 R/W 00H 
    // 95           GPIO[4:3]功能控制寄存器 R/W 00H 
    // 96           GPIO[4:3]信号状态寄存器 R/W 00H 
    // 97           GPIO[2:0]下拉控制寄存器 R/W 00H 
    // 98           PWM1频率设置寄存器 R/W 00H 
    // 99           PWM1占空比设置寄存器1 R/W 16H 
    // 9A           PWM1占空比设置寄存器2 R/W 0BH
    // 9B           PWM2 频率设置寄存器
    // 9C           PWM2占空比设置寄存器1 R/W 16H 

};

struct AXP192_IntCtl_RegSet:public AXP192_Prelude{
// 40 IRQ使能控制寄存器1 R/W D8H
// 41 IRQ使能控制寄存器2 R/W FFH 
// 42 IRQ使能控制寄存器3 R/W 3BH 
// 43 IRQ使能控制寄存器4 R/W C1H 
// 4A IRQ使能控制寄存器5 R/W 00H
// 44 IRQ状态寄存器1 R/W 00H 
// 45 IRQ状态寄存器2 R/W 00H 
// 46 IRQ状态寄存器3 R/W 00H 
// 47 IRQ状态寄存器4 R/W 00H 
// 4D IRQ状态寄存器5 R/W 00H 
};

struct AXP192_Adc_RegSet:public AXP192_Prelude{
    
};

struct AXP192_Coulometre_RegSet:public AXP192_Prelude{

};

class AXP192 final: public AXP192_Prelude{

    explicit AXP192(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c.get(), addr)){}

    explicit AXP192(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){}

    explicit AXP192(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){}

    IResult<> set_dcdc1_voltage(const DcdcVoltage voltage){
        auto reg = RegCopy(pw_regs_.dcdc1_voltage_setting_reg);
        reg.dcdc1_voltage_setting = voltage.to_u8();
        return write_reg(reg);
    }
    IResult<> set_dcdc2_voltage(const DcdcVoltage voltage){
        auto reg = RegCopy(pw_regs_.dcdc2_voltage_setting_reg);
        reg.dcdc2_voltage_setting = voltage.to_u8();
        return write_reg(reg);
    }
    IResult<> set_dcdc3_voltage(const DcdcVoltage voltage){
        auto reg = RegCopy(pw_regs_.dcdc3_voltage_setting_reg);
        reg.dcdc3_voltage_setting = voltage.to_u8();
        return write_reg(reg);
    }
private:
    hal::I2cDrv i2c_drv_;
    AXP192_PowerCtl_Regset pw_regs_ = {};
    AXP192_GpioCtl_RegSet gp_regs_ = {};

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