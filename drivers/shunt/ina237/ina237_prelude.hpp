#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/real.hpp"

#include "hal/conn/i2c/i2cdrv.hpp"

namespace ymd::drivers{

struct [[nodiscard]] INA237_Prelude{

    // A1 A0 辅助器件地址
    // GND GND 1000000
    // GND VS 1000001
    // GND SDA 1000010
    // GND SCL 1000011
    // VS GND 1000100
    // VS VS 1000101
    // VS SDA 1000110
    // VS SCL 1000111
    // SDA GND 1001000
    // SDA VS 1001001
    // SDA SDA 1001010
    // SDA SCL 1001011
    // SCL GND 1001100
    // SCL VS 1001101
    // SCL SDA 1001110
    // SCL SCL 1001111
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1000000);

    // ASCII 的 TI。
    static constexpr uint16_t VALID_MANU_ID = ('T') * 256 + 'I';

    // INA237
    static constexpr uint16_t VALID_CHIP_ID = 0x2260;

    enum class Error_Kind{
        ChipIdVerifyFailed,
        ManuIdVerifyFailed,
        SolveFailed
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] AverageTimes:uint16_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class [[nodiscard]] ConversionTime:uint16_t{
        _50us = 0, 
        _84us, 
        _150us, 
        _280us, 
        _540us, 
        _1052us, 
        _2074us,
        _4120us
    };

    using RegAddr = uint8_t;

    static constexpr iq16 VOLTAGE_LSB_MV = iq16(1.25);


    struct [[nodiscard]] ShuntValue final{
        using Self = ShuntValue;

        //1.25uv/lsb
        int32_t bits;

        static constexpr Self from_mv(const int32_t mv){
            return Self{int32_t(mv * (1000 / 1.25))};
        }
    };


    struct [[nodiscard]] ShuntVoltageCode final{
        using Self = ShuntVoltageCode;
        uint16_t bits;

        static constexpr Self from_shuntvalue_range0(const ShuntValue v){
            return Self{uint16_t(v.bits >> 2)};
        }

        static constexpr Self from_shuntvalue_range1(const ShuntValue v){
            return Self{uint16_t(v.bits)};
        }

        constexpr ShuntValue to_shuntvalue_range0() const {
            return ShuntValue{int32_t(bits << 2)};
        }

        constexpr ShuntValue to_shuntvalue_range1() const {
            return ShuntValue{int32_t(bits)};
        }
    };



    [[nodiscard]] static constexpr uint32_t bv_code_to_uv(uint16_t bv_code){ 
        return bv_code * 3125;
    }


    struct [[nodiscard]] BusbarVoltageCode final{
        // 二进制补码，但始终为正。
        // 3.125mV/LSB。

        using Self = BusbarVoltageCode;
        uint16_t bits;

        constexpr int32_t to_uv() const noexcept {
            return bv_code_to_uv(bits);
        }

    };

    [[nodiscard]] static constexpr uint16_t celsius_encode(const iq20 celsius){
        // 转换因子：125m°C/LSB。
        int16_t bits = int16_t(celsius * 8);
        return uint16_t(bits << 4);
    }

    static constexpr iq20 celsius_decode(const uint16_t bits){
        constexpr auto factor = uq20(0.125);
        const int16_t int_val = int16_t(bits) >> 4;
        return int32_t(int_val) * factor;
    }

    struct [[nodiscard]] TemperatureCode final{
        // 设置用于比较值的阈值，以检测过热测量值。二进制补码值。
        // 在此字段中输入的值直接与 DIETEMP 寄存器中的值进行比较，以确
        // 定是否存在过热情况。转换因子：125m°C/LSB。

        using Self = TemperatureCode;

        uint16_t bits;

        static constexpr Self from_celsius(const iq20 celsius){
            return Self{.bits = celsius_encode(celsius)};
        }

        constexpr iq20 to_celsius() const {
            return celsius_decode(bits);
        }
    };
};


struct INA237_Regs:public INA237_Prelude{
    struct R16_Config:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x00};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        // 3-0保留R 0h保留。始终读为 0。
        const uint16_t __resv1__:4 = 0;

        // 4 ADCRANGE R/W 0h IN+ 和 IN– 之间的分流满量程范围选择。
        // 0h = ±163.84mV
        // 1h = ± 40.96mV
        uint16_t ADCRANGE:1;

        // ina228 only
        // 5 TEMPCOMP R/W 0h启用外部分流器的温度补偿
        // 0h = 禁用分流温度补偿
        // 1h = 启用分流温度补偿
        uint16_t TEMPCOMP:1;

        // 13-6 CONVDLY R/W 0h以 2ms 的步长设置初始 ADC 转换的延迟。
        // 0h = 0s
        // 1h = 2ms
        // FFh = 510ms
        uint16_t CONVDLY:8;

        // ina228 only
        // 14 RSTACC R/W 0h将累积寄存器 ENERGY 和 CHARGE 的内容复位为 0
        // 0h = 正常运行
        // 1h = 将 Energy 和 Charge 寄存器清除为默认值
        uint16_t RSTACC:1;

        // 15 RST R/W 0h复位位。将该位设置为“1”会生成一个与上电复位相同的系统复位。
        // 将所有寄存器复位为默认值。
        // 0h = 正常运行
        // 1h = 系统复位将寄存器设置为默认值
        // 该位会自我清除。
        uint16_t RST:1;
    };


    struct R16_AdcConfig:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x01};
        static constexpr uint16_t RESET_VALUE = 0xfb68;

        // 2-0 AVG R/W 0h选择 ADC 样片平均计数。平均值计算设置适用于所有有效输入。
        // 当 >0h 时，将在平均值计算完成后更新输出寄存器。
        AverageTimes AVG:3;

        // 5-3 VTCT R/W 5h设置温度测量的转换时间：
        ConversionTime VTCT:3;

        // 8-6 VSHCT R/W 5h设置分流电压测量的转换时间：
        ConversionTime VSHCT:3;

        // 11-9 VBUSCT R/W 5h设置总线电压测量的转换时间：
        ConversionTime VBUSCT:3;

        // MODE R/W Fh用户可以针对总线电压、分流电压或温度测量，设置 MODE 位以启用
        // 连续模式或触发模式。
        uint16_t SHUNT_MEAS_EN:1;
        uint16_t BUSBAR_MEAS_EN:1;
        uint16_t TEMP_MEAS_EN:1;
        uint16_t CONTMODE_EN:1;
    };

    struct R16_ShuntCal:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};
        static constexpr uint16_t RESET_VALUE = 0x1000;

        // 14-0 SHUNT_CAL R/W 1000h寄存器为器件提供一个转换常量值，
        // 表示用于计算电流值（安培）的分流电阻。
        // 这个寄存器还设置 CURRENT 寄存器的分辨率。
        // 计算参考节 8.1.2 。

        uint16_t SHUNT_CAL:15;
        uint16_t __RESV1__:1;
    };


    struct R16_Vshunt:public Reg16{ 
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};  
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltageCode code;
    };


    struct R16_Vbus:public Reg16{   
        static constexpr RegAddr REG_ADDR = RegAddr{0x05};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusbarVoltageCode code;
    };


    struct R16_DieTemp:public Reg16{   
        static constexpr RegAddr REG_ADDR = RegAddr{0x06};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        TemperatureCode code;
    };


    struct R16_Current:public Reg16{   
        static constexpr RegAddr REG_ADDR = RegAddr{0x07};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        // 计算得出的电流输出（单位为安培）。二进制补码值。
        // 节 8.1.2 下的值说明。
        int16_t bits;
    };


    struct R24_Power:public Reg24{   
        static constexpr RegAddr REG_ADDR = RegAddr{0x08};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        // POWER R 0h 计算得出的功率输出。
        // 输出值（单位为瓦特）。
        // 无符号表示。正值。
        // 节 8.1.2 下的值说明。

        uint32_t bits;
    };


    struct R16_Alert:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0B};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        // 0 MEMSTAT R/W 1h如果在器件修整存储器空间中检测到校验和错误，则该位设置为 0。
        // 0h = 存储器校验和错误
        // 1h = 正常运行
        uint16_t MEMSTAT:1;

        // 1h = 转换完成
        // 当 ALATCH=1 时，通过读取该寄存器或启动新的触发转换来清除该
        // 位。
        uint16_t CVTDONE:1;

        // 2 POL R/W 0h如果功率测量值超过功率限制寄存器中的阈限值，则该位设置为
        // “1”。
        // 0h = 正常
        // 1h = 功率高于上限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        // 1 CNVRF R/W 0h如果转换完成，则该位设置为 1。
        // 0h = 正常
        uint16_t POL:1;

        // 3 BUSUL R/W 0h如果总线电压测量值低于总线电压低于下限寄存器中的阈限值，则该
        // 位设置为“1”。
        // 0h = 正常
        // 1h = 总线电压低于下限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        uint16_t BUSUL:1;

        // 4 BUSOL R/W 0h如果总线电压测量值超过总线电压高于上限寄存器中的阈限值，则该
        // 位设置为“1”。
        // 0h = 正常
        // 1h = 总线电压高于上限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        uint16_t BUSOL:1;


        // 5 SHNTUL R/W 0h如果分流电压测量值低于分流电压低于下限寄存器中的阈限值，则该
        // 位设置为“1”。
        // 0h = 正常
        // 1h = 分流电压低于下限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        // INA237
        // ZHCSN47A – JANUARY 2021 – REVISED MAY 2022 www.ti.com.cn
        // 26 Submit Document Feedback Copyright © 2022 Texas Instruments Incorporated
        // Product Folder Links: INA237
        // 表 7-16. DIAG_ALRT 寄存器字段说明 (continued)
        // 位字段类型复位说明
        uint16_t SHNTUL:1;

        // 6 SHNTOL R/W 0h如果分流电压测量值超过分流电压高于上限寄存器中的阈限值，则该
        // 位设置为“1”。
        // 0h = 正常
        // 1h = 分流电压高于上限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        uint16_t SHNTOL:1;

        // 7 TMPOL R/W 0h如果温度测量值超过温度高于上限寄存器中的阈限值，则该位设置为
        // “1”。
        // 0h = 正常
        // 1h = 温度高于上限事件
        // 当 ALATCH=1 时，通过读取该寄存器清除该位。
        uint16_t TMPOL:1;

        // 8保留R 0h保留。始终读为 0。
        uint16_t __RESV__:1;

        // 9 MATHOF R 0h如果算术运算导致一个溢出错误的话，该位被置为 1。
        // 它表示电流和功率值也许是无效的。
        // 0h = 正常
        // 1h = 溢出
        // 必须通过触发另一个转换或通过使用 RSTACC 位清除累加器来手动清
        // 除。
        uint16_t MATHOF:1;

        uint16_t __RESV2__:2;


        // 12 APOL R/W 0h警报极性位设定 ALERT 引脚极性。
        // 0h = 正常（低电平有效，开漏）
        // 1h = 反相（高电平有效，开漏）
        uint16_t APOL:1;

        // 13 SLOWALERT R/W 0h启用时，ALERT 功能置位为完成的平均值。
        // 这样可以灵活地将 ALERT 延迟到平均值之后。
        // 0h = 非平均 (ADC) 值上的 ALERT 比较
        // 1h = 平均值上的 ALERT 比较
        uint16_t SLOWALERT:1;

        // 14 CNVR R/W 0h将该位设置为高电平可配置在置位转换就绪标志（位 1）时要置位的 
        // ALERT 引脚，指示转换周期已完成。
        // 0h = 禁用 ALERT 引脚上的转换就绪标志
        // 1h = 启用 ALERT 引脚上的转换就绪标志
        uint16_t CNVR:1;

        // 15 ALATCH R/W 0h当警报锁存使能位设置为透明模式时，如果故障已被清除，则 ALERT 
        // 引脚和标志位复位为空闲状态。
        // 当警报锁存使能位设置为锁存模式时，ALERT 引脚和 ALERT 标志位
        // 在故障后保持有效，直到已读取 DIAG_ALRT 寄存器。
        // 0h = 透明
        // 1h = 锁存
        uint16_t ALATCH:1;
    };


    struct R16_SOVL:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0B}; 
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltageCode code;
    };


    struct R16_SUVL:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0C};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        ShuntVoltageCode code;
    };


    struct R16_BOVL:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0D};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusbarVoltageCode code;
    };


    struct R16_BUVL:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x0E};
        static constexpr uint16_t RESET_VALUE = 0x0000;

        BusbarVoltageCode code;
    };



    struct R16_TempLimit:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x10};
        static constexpr uint16_t RESET_VALUE = 0x7fff;

        TemperatureCode code;
    };



    struct R16_PowerLimit:public Reg16{ 

        static constexpr RegAddr REG_ADDR = RegAddr{0x11}; 
        static constexpr uint16_t RESET_VALUE = 0xffff;
        // 15-0
        // POL R/W FFFFh 设置用于比较值的阈值，以检测功率高于上限测量值。无符号表示，
        // 仅限正值。
        // 在此字段中输入的值直接与 POWER 寄存器中的值进行比较，以确定
        // 是否存在超出功率情况。转换因子：256 × 功率 LSB。
        uint16_t POL:16;
    };


    struct R16_ManfId:public Reg16{
        static constexpr RegAddr REG_ADDR = RegAddr{0x3E};

        // 15-0
        static constexpr uint16_t KEY = (int('T') << 8) | 'I';
        uint16_t MANFID;
    };

    VALIDATE_R16(R16_Config)
    VALIDATE_R16(R16_AdcConfig)
    VALIDATE_R16(R16_ShuntCal)
    VALIDATE_R16(R16_DieTemp)
    VALIDATE_R16(R16_Alert)
    VALIDATE_R16(R16_SOVL)
    VALIDATE_R16(R16_SUVL)
    VALIDATE_R16(R16_BOVL)
    VALIDATE_R16(R16_BUVL)
    VALIDATE_R16(R16_TempLimit)
    VALIDATE_R16(R16_PowerLimit)
    VALIDATE_R16(R16_ManfId)


    R16_Config      config_reg;
    R16_AdcConfig   adc_config_reg;
    R16_ShuntCal    shunt_cal_reg;
    R16_DieTemp     die_temp_reg;
    R16_Alert       alert_reg;

    template<typename T>
    static constexpr void reset_reg(T & reg){
        constexpr auto value = T::RESET_VALUE;
        reg.as_bits_mut() = value;
    }

    constexpr void reset_initial_value(){
        reset_reg(config_reg);
        reset_reg(adc_config_reg);
        reset_reg(shunt_cal_reg);
        reset_reg(die_temp_reg);
        reset_reg(alert_reg);
    }
};



}

