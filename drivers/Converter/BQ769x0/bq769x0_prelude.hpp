#pragma once

// 用于锂离子电池和磷酸盐电池的 BQ769x0 3 节至 15 节串联、电池监控器系列

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/conn/i2c/i2cdrv.hpp"

#include "core/utils/enum/strong_type_gradation.hpp"

namespace ymd::drivers{

struct BQ769x0_Prelude { 
enum class [[nodiscard]] RegAddr : uint8_t {    
    SysStat      = 0x00,      // 系统状态寄存器
    CellBal1     = 0x01,      // 电池均衡寄存器1（CB1-CB5）
    CellBal2     = 0x02,      // 电池均衡寄存器2（CB6-CB10）
    CellBal3     = 0x03,      // 电池均衡寄存器3（CB11-CB15）
    SysCtrl1     = 0x04,      // 系统控制寄存器1
    SysCtrl2     = 0x05,      // 系统控制寄存器2
    Protect1     = 0x06,      // 保护寄存器1（SCD配置）
    Protect2     = 0x07,      // 保护寄存器2（OCD配置）
    Protect3     = 0x08,      // 保护寄存器3（UV/OV延迟配置）
    OvTrip       = 0x09,      // 过压阈值寄存器
    UvTrip       = 0x0A,      // 欠压阈值寄存器
    CcCfg        = 0x0B,      // 库仑计配置寄存器
    Vc1Hi        = 0x0C,      // 电池1电压高位寄存器
    Vc1Lo        = 0x0D,      // 电池1电压低位寄存器
    Vc2Hi        = 0x0E,      // 电池2电压高位寄存器
    Vc2Lo        = 0x0F,      // 电池2电压低位寄存器
    Vc3Hi        = 0x10,      // 电池3电压高位寄存器
    Vc3Lo        = 0x11,      // 电池3电压低位寄存器
    Vc4Hi        = 0x12,      // 电池4电压高位寄存器
    Vc4Lo        = 0x13,      // 电池4电压低位寄存器
    Vc5Hi        = 0x14,      // 电池5电压高位寄存器
    Vc5Lo        = 0x15,      // 电池5电压低位寄存器
    Vc6Hi        = 0x16,      // 电池6电压高位寄存器
    Vc6Lo        = 0x17,      // 电池6电压低位寄存器
    Vc7Hi        = 0x18,      // 电池7电压高位寄存器
    Vc7Lo        = 0x19,      // 电池7电压低位寄存器
    Vc8Hi        = 0x1A,      // 电池8电压高位寄存器
    Vc8Lo        = 0x1B,      // 电池8电压低位寄存器
    Vc9Hi        = 0x1C,      // 电池9电压高位寄存器
    Vc9Lo        = 0x1D,      // 电池9电压低位寄存器
    Vc10Hi       = 0x1E,      // 电池10电压高位寄存器
    Vc10Lo       = 0x1F,      // 电池10电压低位寄存器
    Vc11Hi       = 0x20,      // 电池11电压高位寄存器
    Vc11Lo       = 0x21,      // 电池11电压低位寄存器
    Vc12Hi       = 0x22,      // 电池12电压高位寄存器
    Vc12Lo       = 0x23,      // 电池12电压低位寄存器
    Vc13Hi       = 0x24,      // 电池13电压高位寄存器
    Vc13Lo       = 0x25,      // 电池13电压低位寄存器
    Vc14Hi       = 0x26,      // 电池14电压高位寄存器
    Vc14Lo       = 0x27,      // 电池14电压低位寄存器
    Vc15Hi       = 0x28,      // 电池15电压高位寄存器
    Vc15Lo       = 0x29,      // 电池15电压低位寄存器
    BatHi        = 0x2A,      // 总电池电压高位寄存器
    BatLo        = 0x2B,      // 总电池电压低位寄存器
    Ts1Hi        = 0x2C,      // 温度1高位寄存器
    Ts1Lo        = 0x2D,      // 温度1低位寄存器
    Ts2Hi        = 0x2E,      // 温度2高位寄存器
    Ts2Lo        = 0x2F,      // 温度2低位寄存器
    Ts3Hi        = 0x30,      // 温度3高位寄存器
    Ts3Lo        = 0x31,      // 温度3低位寄存器
    CcHi         = 0x32,      // 库仑计计数高位寄存器
    CcLo         = 0x33,      // 库仑计计数低位寄存器
    AdcGain1     = 0x50,      // ADC增益寄存器1（高2位）
    AdcOffset    = 0x51,      // ADC偏移量寄存器
    AdcGain2     = 0x59       // ADC增益寄存器2（低3位）
};


/**
 * @brief SCD(放电短路) 延迟配置枚举 (PROTECT1: SCD_D1[4] SCD_D0[3])
 * 配置值对应延迟时间: 00=70μs; 01=100μs; 10=200μs; 11=400μs
 */
enum class [[nodiscard]] ScdDelay : uint8_t{
    _70us,   // 00b
    _100us,  // 01b
    _200us,  // 10b
    _400us   // 11b
};

/**
 * @brief SCD(放电短路) 阈值配置枚举 (PROTECT1: SCD_T2[2] SCD_T1[1] SCD_T0[0])
 * 量程区分: RSNS=0(低量程) / RSNS=1(高量程，阈值翻倍)
 */
enum class [[nodiscard]] ScdThreshold : uint8_t{
    _22mV,   // 000b | RSNS=0:22mV | RSNS=1:44mV
    _33mV,   // 001b | RSNS=0:33mV | RSNS=1:67mV
    _44mV,   // 010b | RSNS=0:44mV | RSNS=1:89mV
    _56mV,   // 011b | RSNS=0:56mV | RSNS=1:111mV
    _67mV,   // 100b | RSNS=0:67mV | RSNS=1:133mV
    _78mV,   // 101b | RSNS=0:78mV | RSNS=1:155mV
    _89mV,   // 110b | RSNS=0:89mV | RSNS=1:178mV
    _100mV   // 111b | RSNS=0:100mV | RSNS=1:200mV
};

/**
 * @brief OCD(放电过流) 延迟配置枚举 (PROTECT2: OCD_D2[6] OCD_D1[5] OCD_D0[4])
 * 配置值对应延迟时间: 000=8ms ~ 111=1280ms
 */
enum class [[nodiscard]] OcdDelay : uint8_t{
    _8ms,
    _20ms,
    _40ms,
    _80ms,
    _160ms,
    _320ms,
    _640ms,
    _1280ms
};

/**
 * @brief OCD(放电过流) 阈值配置枚举 (PROTECT2: OCD_T3[3] OCD_T2[2] OCD_T1[1] OCD_T0[0])
 * 量程区分: RSNS=0(低量程) / RSNS=1(高量程，阈值翻倍)
 */
enum class [[nodiscard]] OcdThreshold : uint8_t{
    _8mV,    // 0000b | RSNS=0:8mV | RSNS=1:17mV
    _11mV,   // 0001b | RSNS=0:11mV | RSNS=1:22mV
    _14mV,   // 0010b | RSNS=0:14mV | RSNS=1:28mV
    _17mV,   // 0011b | RSNS=0:17mV | RSNS=1:33mV
    _19mV,   // 0100b | RSNS=0:19mV | RSNS=1:39mV
    _22mV,   // 0101b | RSNS=0:22mV | RSNS=1:44mV
    _25mV,   // 0110b | RSNS=0:25mV | RSNS=1:50mV
    _28mV,   // 0111b | RSNS=0:28mV | RSNS=1:56mV
    _31mV,   // 1000b | RSNS=0:31mV | RSNS=1:61mV
    _33mV,   // 1001b | RSNS=0:33mV | RSNS=1:67mV
    _36mV,   // 1010b | RSNS=0:36mV | RSNS=1:72mV
    _39mV,   // 1011b | RSNS=0:39mV | RSNS=1:78mV
    _42mV,   // 1100b | RSNS=0:42mV | RSNS=1:83mV
    _44mV,   // 1101b | RSNS=0:44mV | RSNS=1:89mV
    _47mV,   // 1110b | RSNS=0:47mV | RSNS=1:94mV
    _50mV    // 1111b | RSNS=0:50mV | RSNS=1:100mV
};

/**
 * @brief UV(欠压) 延迟配置枚举 (PROTECT3: UV_D1[7] UV_D0[6])
 * 配置值对应延迟时间: 00=1s; 01=4s; 10=8s; 11=16s
 */
enum class [[nodiscard]] UvDelay : uint8_t{
    _1s,
    _4s,
    _8s,
    _16s
};

/**
 * @brief OV(过压) 延迟配置枚举 (PROTECT3: OV_D1[5] OV_D0[4])
 * 配置值对应延迟时间: 00=1s; 01=2s; 10=4s; 11=8s
 */
enum class [[nodiscard]] OvDelay : uint8_t{
    _1s,
    _2s,
    _4s,
    _8s
};

/**
 * @brief 芯片型号枚举 (适配不同寄存器支持)
 */
enum class [[nodiscard]] Bq769x0Package : uint8_t{
    BQ76920,  // 支持5串电池
    BQ76930,  // 支持10串电池
    BQ76940   // 支持15串电池
};

/**
 * @brief 温度源选择枚举 (SYS_CTRL1: TEMP_SEL[3])
 */
enum class [[nodiscard]] TempSource : uint8_t{
    InternalChipTemp,  // 0: 内部芯片温度
    ThermistorTemp     // 1: 外部热敏电阻温度
};

/**
 * @brief 库仑计工作模式枚举 (SYS_CTRL2: CC_EN[6] CC_ONESHOT[5])
 */
enum class [[nodiscard]] CoulombCounterMode : uint8_t{
    Disable,           // 00: 禁用
    OneShot,           // 01: 单次触发(250ms)
    Continuous         // 10: 连续采集
};
};


struct BQ769x0_Regset:public BQ769x0_Prelude{
//0x00 - SYS_STAT 系统状态寄存器 (R/W)
struct R8_SysStat : public Reg8<R8_SysStat> {
    static constexpr RegAddr REG_ADDR = RegAddr::SysStat;

    uint8_t ocd : 1; // 放电过流故障标志 (写1清零)
    uint8_t scd : 1; // 放电短路故障标志 (写1清零)
    uint8_t ov : 1; // 过压故障标志 (写1清零)
    uint8_t uv : 1; // 欠压故障标志 (写1清零)
    uint8_t ovrd_alert : 1; // ALERT引脚外部上拉标志 (写1清零)
    uint8_t device_xready : 1; // 芯片内部故障标志 (写1清零)
    uint8_t reserved6 : 1; // 保留位
    uint8_t cc_ready : 1; // 库仑计新读数就绪标志 (写1清零)
} DEF_R8(sys_stat_reg);

//0x01 - CELLBAL1 电池均衡寄存器1 (R/W)
struct R8_CellBal1 : public Reg8<R8_CellBal1> {
    static constexpr RegAddr REG_ADDR = RegAddr::CellBal1;

    uint8_t reserved0 : 1; // 保留位
    uint8_t cb1 : 1; // 电池1均衡使能
    uint8_t cb2 : 1; // 电池2均衡使能
    uint8_t cb3 : 1; // 电池3均衡使能
    uint8_t cb4 : 1; // 电池4均衡使能
    uint8_t cb5 : 1; // 电池5均衡使能
    uint8_t reserved6_7 : 2; // 保留位
} DEF_R8(cell_bal1_reg);

//0x02 - CELLBAL2 电池均衡寄存器2 (R/W) [BQ76930/40]
struct R8_CellBal2 : public Reg8<R8_CellBal2> {
    static constexpr RegAddr REG_ADDR = RegAddr::CellBal2;

    uint8_t reserved0 : 1; // 保留位
    uint8_t cb6 : 1; // 电池6均衡使能
    uint8_t cb7 : 1; // 电池7均衡使能
    uint8_t cb8 : 1; // 电池8均衡使能
    uint8_t cb9 : 1; // 电池9均衡使能
    uint8_t cb10 : 1; // 电池10均衡使能
    uint8_t reserved6_7 : 2; // 保留位
} DEF_R8(cell_bal2_reg);

//0x03 - CELLBAL3 电池均衡寄存器3 (R/W) [BQ76940]
struct R8_CellBal3 : public Reg8<R8_CellBal3> {
    static constexpr RegAddr REG_ADDR = RegAddr::CellBal3;

    uint8_t reserved0_1 : 2; // 保留位
    uint8_t cb11 : 1; // 电池11均衡使能
    uint8_t cb12 : 1; // 电池12均衡使能
    uint8_t cb13 : 1; // 电池13均衡使能
    uint8_t cb14 : 1; // 电池14均衡使能
    uint8_t cb15 : 1; // 电池15均衡使能
    uint8_t reserved7 : 1; // 保留位
} DEF_R8(cell_bal3_reg);

//0x04 - SYS_CTRL1 系统控制寄存器1 (R/W)
struct R8_SysCtrl1 : public Reg8<R8_SysCtrl1> {
    static constexpr RegAddr REG_ADDR = RegAddr::SysCtrl1;

    uint8_t shut_b : 1; // 关断控制位B
    uint8_t shut_a : 1; // 关断控制位A
    uint8_t reserved2 : 1; // 保留位 (禁止置1)
    TempSource temp_sel : 1; // 温度源选择: 0-内部温度 1-热敏电阻
    uint8_t adc_en : 1; // ADC使能: 0-禁用 1-启用
    uint8_t reserved5_6 : 2; // 保留位
    uint8_t load_present : 1; // 外部负载检测 (只读)
} DEF_R8(sys_ctrl1_reg);

//0x05 - SYS_CTRL2 系统控制寄存器2 (R/W)
struct R8_SysCtrl2 : public Reg8<R8_SysCtrl2> {
    static constexpr RegAddr REG_ADDR = RegAddr::SysCtrl2;

    uint8_t chg_on : 1; // 充电控制: 0-关 1-开
    uint8_t dsg_on : 1; // 放电控制: 0-关 1-开
    uint8_t reserved2_3 : 2; // 保留位
    uint8_t cc_oneshot : 1; // 库仑计单次触发 (配合cc_en使用)
    uint8_t cc_en : 1; // 库仑计连续使能
    uint8_t delay_dis : 1; // 保护延迟禁用: 0-正常 1-旁路
    uint8_t reserved7 : 1; // 保留位
} DEF_R8(sys_ctrl2_reg);

//0x06 - PROTECT1 保护寄存器1 (R/W)
struct R8_Protect1 : public Reg8<R8_Protect1> {
    static constexpr RegAddr REG_ADDR = RegAddr::Protect1;

    ScdThreshold scd_t : 3; // SCD阈值配置位 [2:0]
    ScdDelay scd_d : 2; // SCD延迟配置位 [4:3]
    uint8_t reserved5 : 1; // 保留位 (禁止置1)
    uint8_t reserved6 : 1; // 保留位
    uint8_t rsns : 1; // OCD/SCD阈值量程: 0-低量程 1-高量程
} DEF_R8(protect1_reg);

//0x07 - PROTECT2 保护寄存器2 (R/W)
struct R8_Protect2 : public Reg8<R8_Protect2> {
    static constexpr RegAddr REG_ADDR = RegAddr::Protect2;

    OcdThreshold ocd_t : 4; // OCD阈值配置位 [3:0]
    OcdDelay ocd_d : 3; // OCD延迟配置位 [6:4]
    uint8_t reserved7 : 1; // 保留位
} DEF_R8(protect2_reg);

//0x08 - PROTECT3 保护寄存器3 (R/W)
struct R8_Protect3 : public Reg8<R8_Protect3> {
    static constexpr RegAddr REG_ADDR = RegAddr::Protect3;

    uint8_t reserved0 : 1; // 保留位 (内部调试)
    uint8_t reserved1 : 1; // 保留位 (内部调试)
    uint8_t reserved2 : 1; // 保留位 (内部调试)
    uint8_t reserved3 : 1; // 保留位 (内部调试)
    OvDelay ov_d : 2; // OV延迟配置位 [5:4]
    UvDelay uv_d : 2; // UV延迟配置位 [7:6]
} DEF_R8(protect3_reg);

//0x09 - OV_TRIP 过压阈值寄存器 (R/W)
struct R8_OvTrip : public Reg8<R8_OvTrip> {
    static constexpr RegAddr REG_ADDR = RegAddr::OvTrip;

    uint8_t ov_t0 : 1; // 过压阈值位0
    uint8_t ov_t1 : 1; // 过压阈值位1
    uint8_t ov_t2 : 1; // 过压阈值位2
    uint8_t ov_t3 : 1; // 过压阈值位3
    uint8_t ov_t4 : 1; // 过压阈值位4
    uint8_t ov_t5 : 1; // 过压阈值位5
    uint8_t ov_t6 : 1; // 过压阈值位6
    uint8_t ov_t7 : 1; // 过压阈值位7
} DEF_R8(ov_trip_reg);

//0x0A - UV_TRIP 欠压阈值寄存器 (R/W)
struct R8_UvTrip : public Reg8<R8_UvTrip> {
    static constexpr RegAddr REG_ADDR = RegAddr::UvTrip;

    uint8_t uv_t0 : 1; // 欠压阈值位0
    uint8_t uv_t1 : 1; // 欠压阈值位1
    uint8_t uv_t2 : 1; // 欠压阈值位2
    uint8_t uv_t3 : 1; // 欠压阈值位3
    uint8_t uv_t4 : 1; // 欠压阈值位4
    uint8_t uv_t5 : 1; // 欠压阈值位5
    uint8_t uv_t6 : 1; // 欠压阈值位6
    uint8_t uv_t7 : 1; // 欠压阈值位7
} DEF_R8(uv_trip_reg);

//0x0B - CC_CFG 库仑计配置寄存器 (R/W)
struct R8_CcCfg : public Reg8<R8_CcCfg> {
    static constexpr RegAddr REG_ADDR = RegAddr::CcCfg;

    uint8_t cc_cfg0 : 1; // 库仑计配置位0
    uint8_t cc_cfg1 : 1; // 库仑计配置位1
    uint8_t cc_cfg2 : 1; // 库仑计配置位2
    uint8_t cc_cfg3 : 1; // 库仑计配置位3
    uint8_t cc_cfg4 : 1; // 库仑计配置位4
    uint8_t cc_cfg5 : 1; // 库仑计配置位5
    uint8_t reserved6_7 : 2; // 保留位
} DEF_R8(cc_cfg_reg);

//0x0C-0x29 - 电池电压寄存器 (只读，高低字节组合)
struct R16_CellVolt : public Reg16<R16_CellVolt> {
    // static constexpr RegAddr REG_ADDR_HI = RegAddr::CellVoltHi;

    // static constexpr RegAddr REG_ADDR_LO = RegAddr::CellVoltLo;

    uint16_t volt_val : 14; // 14位电池电压有效值
    uint16_t reserved14_15 : 2; // 保留位
} DEF_R16(cell_volt_reg);

//0x2A-0x2B - BAT_HI/LO 总电池电压寄存器 (只读)
struct R16_BatVolt : public Reg16<R16_BatVolt> {
    // static constexpr RegAddr REG_ADDR_HI = RegAddr::BatVoltHi;

    // static constexpr RegAddr REG_ADDR_LO = RegAddr::BatVoltLo;

    uint16_t volt_val : 16; // 16位总电压有效值
} DEF_R16(bat_volt_reg);

//0x2C-0x31 - 温度寄存器 (只读，高低字节组合)
struct R16_Temp : public Reg16<R16_Temp> {
    // static constexpr RegAddr REG_ADDR_HI = RegAddr::TempHi;

    // static constexpr RegAddr REG_ADDR_LO = RegAddr::TempLo;

    uint16_t temp_val : 14; // 14位温度有效值
    uint16_t reserved14_15 : 2; // 保留位
} DEF_R16(temp_reg);

//0x32-0x33 - CC_HI/LO 库仑计计数寄存器 (只读)
struct R16_CcCount : public Reg16<R16_CcCount> {
    // static constexpr RegAddr REG_ADDR_HI = RegAddr::CcCountHi;

    // static constexpr RegAddr REG_ADDR_LO = RegAddr::CcCountLo;

    uint16_t count_val : 16; // 16位库仑计计数值
} DEF_R16(cc_count_reg);

//0x50 - ADCGAIN1 ADC增益寄存器1 (只读)
struct R8_AdcGain1 : public Reg8<R8_AdcGain1> {
    static constexpr RegAddr REG_ADDR = RegAddr::AdcGain1;

    uint8_t reserved0_1 : 2; // 保留位
    uint8_t adc_gain3 : 1; // ADC增益位3
    uint8_t adc_gain4 : 1; // ADC增益位4
    uint8_t reserved4_7 : 4; // 保留位
} DEF_R8(adc_gain1_reg);

//0x51 - ADCOFFSET ADC偏移量寄存器 (只读)
struct R8_AdcOffset : public Reg8<R8_AdcOffset> {
    static constexpr RegAddr REG_ADDR = RegAddr::AdcOffset;

    uint8_t adc_offset0 : 1; // ADC偏移量位0
    uint8_t adc_offset1 : 1; // ADC偏移量位1
    uint8_t adc_offset2 : 1; // ADC偏移量位2
    uint8_t adc_offset3 : 1; // ADC偏移量位3
    uint8_t adc_offset4 : 1; // ADC偏移量位4
    uint8_t adc_offset5 : 1; // ADC偏移量位5
    uint8_t adc_offset6 : 1; // ADC偏移量位6
    uint8_t adc_offset7 : 1; // ADC偏移量位7 (符号位)
} DEF_R8(adc_offset_reg);

//0x59 - ADCGAIN2 ADC增益寄存器2 (只读)
struct R8_AdcGain2 : public Reg8<R8_AdcGain2> {
    static constexpr RegAddr REG_ADDR = RegAddr::AdcGain2;

    uint8_t reserved0_4 : 5; // 保留位
    uint8_t adc_gain0 : 1; // ADC增益位0
    uint8_t adc_gain1 : 1; // ADC增益位1
    uint8_t adc_gain2 : 1; // ADC增益位2
} DEF_R8(adc_gain2_reg)
};


}