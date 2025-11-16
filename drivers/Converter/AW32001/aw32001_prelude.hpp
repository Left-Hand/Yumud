#pragma once

// 这个驱动还在开发中

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

#include "core/io/regs.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "core/utils/strong_type_gradation.hpp"

namespace ymd::drivers{

struct AW32001_Prelude{
    enum class Error_Kind:uint8_t{
        NotReady,
        UnexpectedProductId,
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34);


    // VIN_DPM voltage regulation setting: 
    enum class VIN_DPM : uint8_t {
        _3_88V = 0b0000,
        _3_96V = 0b0001,
        _4_04V = 0b0010,
        _4_12V = 0b0011,
        _4_20V = 0b0100,
        _4_28V = 0b0101,
        _4_36V = 0b0110,
        _4_44V = 0b0111,
        _4_52V = 0b1000, //default
        _4_60V = 0b1001,
        _4_68V = 0b1010,
        _4_76V = 0b1011,
        _4_84V = 0b1100,
        _4_92V = 0b1101,
        _5_00V = 0b1110,
        _5_08V = 0b1111,

        Default = _4_52V
    };

    //Input current limit setting:
    enum class IIN_LIM:uint8_t{
        _50mA = 0b0000,
        _80mA = 0b0001,
        _110mA,
        _140mA,
        _170mA,
        _200mA,
        _230mA,
        _260mA,
        _290mA,
        _320mA,
        _350mA,
        _380mA,
        _410mA,
        _440mA,
        _470mA,
        _500mA,

        Default = _50mA
    };

    enum class PullIntLowPeriod:uint8_t{
        _8s = 0b00,
        _12s = 0b01,
        _16s = 0b10,
        _20s = 0b11
    };

    enum class Q2LastOffTime:uint8_t{
        _2s = 0b0,
        _4s = 0b1,
    };

    enum class BatterUvloThreshold:uint8_t{
        _2_43V = 0b000, 
        _2_49V = 0b001, 
        _2_58V = 0b010, 
        _2_67V = 0b011, 
        _2_76V = 0b100,
        _2_85V = 0b101,
        _2_94V = 0b110,
        _3_03V = 0b111,
    };

    enum class WatchdogTime:uint8_t{
        Disabled = 0b00,
        _40s = 0b01,
        _120s = 0b10,
        _160s = 0b11, //default

        Default = _160s
    };

    enum class FastChargeTime:uint8_t{
        _3hrs = 0b00,
        _5hrs = 0b01, //default
        _8hrs = 0b10,
        _12hrs = 0b11,

        Default = _5hrs
    };

    enum class ThermalRegulationThreshold:uint8_t{
        _60C = 0b00,
        _80C = 0b01,
        _100C = 0b10,
        _120C = 0b11, //default

        Default = _120C
    };

    enum class ChargeState:uint8_t{
        NotCharging = 0b00,
        PreCharge = 0b01,
        Charge = 0b10,
        ChargeDone = 0b11,
    };

    enum class DeglitchingTimes:uint8_t{
        _1 = 0b00,
        _2 = 0b01,
        _4 = 0b10,
        _8 = 0b11,
    };

    DEF_U8_STRONG_TYPE_GRADATION(ChargeCurrent, 
        from_ma, uint32_t, 0, 512, 8);
    DEF_U8_STRONG_TYPE_GRADATION(DischargeCurrent, 
        from_ma, uint32_t, 0, 3200, 200);
    DEF_U8_STRONG_TYPE_GRADATION(TerminationCurrent, 
        from_ma, uint32_t, 1, 31, 2);
    DEF_U8_STRONG_TYPE_GRADATION(PrechargeCurrent, 
        from_ma, uint32_t, 1, 31, 2);
    DEF_U8_STRONG_TYPE_GRADATION(ChargeVoltage, 
        from_volt, iq16, 3.600, 4.545, 0.015);
    DEF_U8_STRONG_TYPE_GRADATION(VsysReg, 
        from_volt, iq16, 4.200, 4.95, 0.05);


    enum class RegAddr:uint8_t{
        InputSourceControl = 0x00,
        PowerOnConfiguration = 0x01,
        ChargeCurrentControl = 0x02,
        DischargeCurrentControl = 0x03,
        ChargeVoltage = 0x04,
        TimerControl = 0x05,
        MainControl = 0x06,
        SystemVoltage = 0x07,
        SystemStatus = 0x08,
        Fault = 0x09,
        ChipId = 0x0A,
        IndividualCharge = 0x0B,
        AdditionalFunctionControl = 0x0C,
        AdditionalFunctionControl1 = 0x22,
    };
};


struct AW32001_Regset:public AW32001_Prelude{ 

// Address: 00H, Reset State: 1000 1111.
struct R8_InputSourceControl:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::InputSourceControl;
    VIN_DPM vin_dpm:4;
    IIN_LIM iin_lim:4;
}DEF_R8(intput_source_control_reg)

// Address: 01H, Reset State1010 1100.
struct R8_PowerOnConfiguration:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::PowerOnConfiguration;
    BatterUvloThreshold vbat_uvlo:3;
    uint8_t disen_charge:1;
    uint8_t en_hiz:1;
    Q2LastOffTime t_rst_dur:1;
    PullIntLowPeriod t_rst_dgl:2;
}DEF_R8(power_on_configuration_reg)

struct R8_ChargeCurrentControl:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::ChargeCurrentControl;
    uint8_t ichg:6;
    uint8_t wd_tmr_rst:1;
    uint8_t reg_rst:1;
}DEF_R8(charge_current_control_reg)

struct R8_DischargeCurrentControl:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::DischargeCurrentControl;
    uint8_t iterm:4;
    uint8_t idischg:4;
}DEF_R8(discharge_current_control_reg)

struct R8_ChargeVoltage:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::ChargeVoltage;
    // Battery recharge threshold (below VBAT_REG):
    // 0: 100mV; 1: 200mV (default).
    uint8_t vrech:1;

    // Pre-charge to Fast charge threshold:
    // 0:2.8V; 1: 3.0V (default).
    uint8_t vbat_pre:1;

    uint8_t vbat_reg:6;
}DEF_R8(charge_voltage_reg)

struct R8_TimerControl:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::TimerControl;
    uint8_t term_tmr:1;
    FastChargeTime chg_time:2;
    uint8_t en_timer:1;
    uint8_t en_term:1;

    // Watchdog setting:
    // 00: disable timer 01: 40s
    // 10: 80s 11: 160s (default)
    // If Bit[6:5]=00, then watchdog timer is disable no matter
    // Bit 7 is set or no
    WatchdogTime watchdog:2;
    uint8_t en_wd_dischg:1;
}DEF_R8(timer_control_reg)

struct R8_MainControl:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::MainControl;
    uint8_t batovp_int_en:1;
    uint8_t ntc_int_en:1;
    uint8_t chg_status_int_en:1;
    uint8_t eoc_status_int_en:1;

    uint8_t pg_int_en:1;
    uint8_t fet_disen:1;
    uint8_t tmr2x_en:1;
    uint8_t ntc_en:1;
}DEF_R8(main_control_reg)

struct R8_SystemVoltage:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::SystemVoltage;
    uint8_t vsys_reg:6;
    ThermalRegulationThreshold tj_reg:2;
}DEF_R8(system_voltage_reg)

struct R8_SystemStatus:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::SystemStatus;
    uint8_t therm_stat:1;
    uint8_t pg_stat:1; // power good / power fail
    uint8_t ppm_stat:1; // in ppm / no ppm
    ChargeState chg_stat:2;
    uint8_t __resv__:2;
    uint8_t watchdog_fault:1; //watchdog timer expiration
}DEF_R8(system_status_reg)

struct R8_Fault:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::Fault;
    uint8_t ntc_cold:1;
    uint8_t ntc_hot:1;
    uint8_t stmr_fault:1;
    uint8_t bat_ovp:1;
    
    uint8_t therm_fault:1;
    uint8_t vin_fault:1;
    DeglitchingTimes en_shipping_dgl:2;
}DEF_R8(fault_reg)

struct R8_ChipId:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::ChipId;
    static constexpr uint8_t KEY = 0b01001001;
    uint8_t chip_id;
}DEF_R8(chip_id_reg)

struct R8_IndividualCharge:public Reg8<>{
    static constexpr auto ADDRESS = RegAddr::IndividualCharge;
    // VIN plug:In deglitch time of shipping mode out：
    // 0：2s; 1：100ms(default)
    uint8_t en_shipmd_0p1s:1;

    uint8_t ipre:4;

    // 0: IPRE is set by REG03H[4:1] (default);
    // 1: IPRE is set by REG0B[4:1]
    uint8_t en_ipre_set:1;
    uint8_t __resv__:1;

    // 1: reduce the current value of REG02H[3:0]
    // configuration to 1/4;
    // 0: keep the current value of REG02H[5:0]
    // configuration. (default)
    uint8_t en_ichg_divd:1;
}DEF_R8(individual_charge_reg)

struct R8_AdditionalFunctionControl:public Reg8<>{
    // The delay time after VSYS is Reset:
    // 0: 0s (Default); 1: 2s.
    uint8_t rstdlay:1;

    // NTC current mode configure:
    // 0: Disable NTC current mode(Default);
    // 1: Enable NTC current mode
    uint8_t en_10k_ntc:1;

    // The function of disabling INT PIN during SHIPPING mode:
    // 0: Nominal INT PIN function(Default);
    // 1: Disable INT PIN function during SHIPPING mode
    uint8_t dis_ship_int:1;

    // 0: Pre-charge timeout is 1h (default);
    // 1: Pre-charge timeout is 2h
    uint8_t preto:1;

    uint8_t __resv__:2;

    uint8_t itermdeg:1;

    // Increase input current limit:
    // 0: The input current limit is set by REG00H[3:0] (default) ;
    // 1: If REG00H[3:0]=1111,this bit increase the input current
    // limit to 550mA, otherwise, it is useless
    uint8_t en0p55:1;
}DEF_R8(additional_function_control_reg)

struct R8_AdditionalFunctionControl1:public Reg8<>{
    uint8_t __resv__:3;

    // INT 100ms exit shipping mode.
    // 0: 2s (default); 
    // 1:100ms
    uint8_t int_100ms:1;
    uint8_t __resv2__:4;
}DEF_R8(additional_function_control1_reg)


};


}