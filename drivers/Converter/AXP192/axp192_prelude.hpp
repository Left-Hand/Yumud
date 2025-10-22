#pragma once

// 这个驱动正在开发中

// AXP192 是高度集成的电源系统管理芯片，针对单芯锂电池(锂离子或锂聚合物)且需要多路电源转换
// 输出的应用，提供简单易用而又可以灵活配置的完整电源解决方案，充分满足目前日益复杂的应用处理
// 器系统对于电源相对复杂而精确控制的要求。 
// AXP192 内部集成了一个自适应的 USB-Compatible 的充电器，3 路降压转换器(Buck DC-DC 
// converter)，4 路线性稳压器(LDO)，电压/电流/温度监视等多路12-Bit ADC。为保证电源系统安全稳定，
// AXP192 还整合了过/欠压(OVP/UVP)、过温(OTP)、过流(OCP)等保护电路。

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct AXP192_Prelude{
    enum class ErrorKind:uint8_t{
        NotReady,
        UnexpectedProductId,

        Dcdc1SetVoltageGreatThan_3_5V,
        Dcdc1SetVoltageLessThan_0_7V,
        Dcdc2SetVoltageGreatThan_2_275V,
        Dcdc2SetVoltageLessThan_0_7V,
        Dcdc3SetVoltageGreatThan_3_5V,
        Dcdc3SetVoltageLessThan_0_7V,

        Ldo2SetVoltageGreatThan_3_3V,
        Ldo2SetVoltageLessThan_1_8V,
        Ldo3SetVoltageGreatThan_3_3V,
        Ldo3SetVoltageLessThan_1_8V,


    };

    DEF_ERROR_SUMWITH_HALERROR(Error, ErrorKind)

    template<typename T = void>
    using IResult = Result<Error, T>;

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

    enum class DcDcSwitchingFrequency:uint8_t{
        // 每一级改变5%，默认值1.5MHz
        _1_5MHz     = 0b0000,
        _1_575MHz   = 0b0001,
        _1_65MHz    = 0b0010,
        _1_725MHz   = 0b0011,
        _1_8MHz     = 0b0100,
        _1_875MHz   = 0b0101,
        _1_95MHz    = 0b0110,
        _2_025MHz   = 0b0111,
        _2_1MHz     = 0b1000,
        _2_175MHz   = 0b1001,
        _2_25MHz    = 0b1010,
        _2_325MHz   = 0b1011,
        _2_4MHz     = 0b1100,
        _2_475MHz   = 0b1101,
        _2_55MHz    = 0b1110,
        _2_625MHz   = 0b1111
    };

    
};

struct AXP192_Regs:public AXP192_Prelude{ 
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



    struct R8_PowerInputState:public Reg8<>{
        uint8_t setup_source_is_acin_or_vbus:1;
        uint8_t is_acin_or_vbus_short_circuit:1;
        uint8_t battery_is_charging_either_discharge:1;
        uint8_t is_vbus_great_than_vhold:1;
        uint8_t is_vbus_present:1;
        uint8_t is_acin_available:1;
        uint8_t is_acin_present:1;
    };

    struct R8_PowerModeChargeState:public Reg8<>{
        uint8_t :1;
        uint8_t powerdown_strategy_is_b_either_a:1;
        uint8_t is_charging_current_less_than_expected:1;
        uint8_t is_on_battery_activate_mode:1;
        uint8_t :1;
        uint8_t is_battery_present:1;
        uint8_t is_charing:1;
        uint8_t is_overheat:1;
    };

    struct R8_UsbOtgVbusState:public Reg8<>{
        uint8_t is_session_end:1;
        uint8_t is_vbus_session_valid:1;
        uint8_t is_vbus_valid:1;
        uint8_t :5;
    };

    struct R8_ExtenDcDc2SwitchCtrl:public Reg8<>{
        uint8_t dcdc2_en:1;
        uint8_t :1;
        uint8_t ext_en:1;
        uint8_t :5;
    };

    struct R8_PowerOutputCtrl:public Reg8<>{
        uint8_t dcdc1_en:1;
        uint8_t dcdc3_en:1;
        uint8_t ldo2_en:1;
        uint8_t ldo3_en:1;
        uint8_t dcdc2_en:1;
        uint8_t :1;
        uint8_t ext_en:1;
        uint8_t :1;
    };



    
    struct R8_DcDc2VoltageSetting:public Reg8<>{
        uint8_t dcdc2_voltage_setting:7;
        uint8_t :1;


    static constexpr Result<uint8_t, Error> voltage_mv_to_u8(const uint32_t voltage_mv){
        // 0.7v-2.275v, 25mv/step
        if(voltage_mv < 700) return Err(Error::Dcdc2SetVoltageLessThan_0_7V);
        if(voltage_mv > 2275) return Err(Error::Dcdc2SetVoltageGreatThan_2_275V);
        return Ok(static_cast<uint8_t>(voltage_mv - 700) / 25);
    }
        constexpr Result<void, Error> set_output_voltage_mv(const uint32_t voltage_mv){
            if(const auto res = voltage_mv_to_u8(voltage_mv);
                res.is_err()) return Err(res.unwrap_err());
            else dcdc2_voltage_setting = res.unwrap();
        }
    };

    struct R8_DcDc1VoltageSetting:public Reg8<>{
        uint8_t dcdc1_voltage_setting:7;
        uint8_t :1;

        static constexpr Result<uint8_t, Error> voltage_mv_to_u8(const uint32_t voltage_mv){
            // 0.7v-2.275v, 25mv/step
            if(voltage_mv < 700) return Err(Error::Dcdc1SetVoltageLessThan_0_7V);
            if(voltage_mv > 2275) return Err(Error::Dcdc1SetVoltageGreatThan_3_5V);
            return Ok(static_cast<uint8_t>(voltage_mv - 700) / 25);
        }
        constexpr Result<void, Error> set_output_voltage_mv(const uint32_t voltage_mv){
            if(const auto res = voltage_mv_to_u8(voltage_mv);
                res.is_err()) return Err(res.unwrap_err());
            else dcdc1_voltage_setting = res.unwrap();
        }
    };


    struct R8_DcDc3VoltageSetting:public Reg8<>{
        uint8_t dcdc3_voltage_setting:7;
        uint8_t :1;

        static constexpr Result<uint8_t, Error> voltage_mv_to_u8(const uint32_t voltage_mv){
            // 0.7v-2.275v, 25mv/step
            if(voltage_mv < 700) return Err(Error::Dcdc3SetVoltageLessThan_0_7V);
            if(voltage_mv > 2275) return Err(Error::Dcdc3SetVoltageGreatThan_3_5V);
            return Ok(static_cast<uint8_t>(voltage_mv - 700) / 25);
        }
        constexpr Result<void, Error> set_output_voltage_mv(const uint32_t voltage_mv){
            if(const auto res = voltage_mv_to_u8(voltage_mv);
                res.is_err()) return Err(res.unwrap_err());
            else dcdc3_voltage_setting = res.unwrap();
        }
    };

    struct R8_Ldo23VoltageSetting:public Reg8<>{
        uint8_t ldo3_voltage_setting:4;
        uint8_t ldo2_voltage_setting:4;

        template<size_t I>
        static constexpr Result<uint8_t, Error> voltage_mv_to_u8(const uint32_t voltage_mv){
            if(voltage_mv < 1800) return []{
                if constexpr (I == 2) return Err(Error::Ldo2SetVoltageLessThan_1_8V);
                if constexpr (I == 3) return Err(Error::Ldo3SetVoltageLessThan_1_8V);
            }();

            if(voltage_mv > 3300) return []{
                if constexpr (I == 2) return Err(Error::Ldo2SetVoltageLessThan_3_3V);
                if constexpr (I == 3) return Err(Error::Ldo3SetVoltageLessThan_3_3V);
            }();

            return Ok(static_cast<uint8_t>(voltage_mv - 1800) / 100);
        }

        template<size_t I>
        constexpr Result<void, Error> set_output_voltage_mv(const uint32_t voltage_mv){
            if(const auto res = voltage_mv_to_u8<I>(voltage_mv);
                res.is_err()) return Err(res.unwrap_err());
            else []{
                if constexpr (I == 2) ldo2_voltage_setting = res.unwrap();
                if constexpr (I == 3) ldo3_voltage_setting = res.unwrap();
            }
        }
    };


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

    struct R8_VbusIpsoutRaouteManage:public Reg8<>{
        VbusCurrentLimit vbus_current_limit:1;
        uint8_t vbus_current_limit_en:1;
        uint8_t :1;
        VholdVoltage vhold_setting:3;
        uint8_t vhold_voltage_limit_en:1;
        uint8_t is_vbus_ipsout_en_nomatter_v_busen:1;
    };
};      

}