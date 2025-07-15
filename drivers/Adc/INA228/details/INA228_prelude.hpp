#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "concept/analog_channel.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd{
struct INA228_Prelude{
    using RegAddress = uint8_t;

    enum class AverageTimes:uint8_t{
        _1 = 0,
        _4 = 1,
        _16 = 2,
        _64 = 3,
        _128 = 4,
        _256 = 5,
        _512 = 6,
        _1024 = 7
    };

    enum class ConversionTime:uint8_t{
        _140us = 0, _204us, _332us, _588us, _1_1ms, _2_116_ms, _4_156ms, _8_244ms
    };
    
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x40 >> 1); // 16 bit
};

struct INA228_Regs:public INA228_Prelude{
    struct R16_Config:public Reg16<>{
        static constexpr RegAddress address = 0x00;

        // 3-0保留R 0h保留。始终读为 0。
        const uint16_t __resv1__:4 = 0;

        // 4 ADCRANGE R/W 0h IN+ 和 IN– 之间的分流满量程范围选择。
        // 0h = ±163.84mV
        // 1h = ± 40.96mV
        uint16_t ADCRANGE:1;

        // 5 TEMPCOMP R/W 0h启用外部分流器的温度补偿
        // 0h = 禁用分流温度补偿
        // 1h = 启用分流温度补偿
        uint16_t TEMPCOMP:1;

        // 13-6 CONVDLY R/W 0h以 2ms 的步长设置初始 ADC 转换的延迟。
        // 0h = 0s
        // 1h = 2ms
        // FFh = 510ms
        uint16_t CONVDLY:8;

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


    struct R16_AccConfig:public Reg16<>{
        static constexpr RegAddress address = 0x01;

        // 2-0 AVG R/W 0h选择 ADC 样片平均计数。平均值计算设置适用于所有有效输入。
        // 当 >0h 时，将在平均值计算完成后更新输出寄存器。
        // 0h = 1
        // 1h = 4
        // 2h = 16
        // 3h = 64
        // 4h = 128
        // 5h = 256
        // 6h = 512
        // 7h = 1024
        uint16_t AVG:3;

                // 5-3 VTCT R/W 5h设置温度测量的转换时间：
        // 0h = 50µs
        // 1h = 84µs
        // 2h = 150µs
        // 3h = 280µs
        // 4h = 540µs
        // 5h = 1052µs
        // 6h = 2074µs
        // 7h = 4120µs
        uint16_t VTCT:3;

                // 8-6 VSHCT R/W 5h设置分流电压测量的转换时间：
        // 0h = 50µs
        // 1h = 84µs
        // 2h = 150µs
        // 3h = 280µs
        // 4h = 540µs
        // 5h = 1052µs
        // 6h = 2074µs
        // 7h = 4120µs
        uint16_t VSHCT:3;

                // 11-9 VBUSCT R/W 5h设置总线电压测量的转换时间：
        // 0h = 50µs
        // 1h = 84µs
        // 2h = 150µs
        // 3h = 280µs
        // 4h = 540µs
        // 5h = 1052µs
        // 6h = 2074µs
        // 7h = 4120µs
        uint16_t VBUSCT:3;

        // MODE R/W Fh用户可以针对总线电压、分流电压或温度测量，设置 MODE 位以启用
        // 连续模式或触发模式。
        // 0h = 关断
        // 1h = 触发总线电压，单冲
        // 2h = 触发分流电压，单冲
        // 3h = 触发分流电压和总线电压，单冲
        // 4h = 触发温度，单冲
        // 5h = 触发温度和总线电压，单冲
        // 6h = 触发温度和分流电压，单冲
        // 7h = 触发总线电压、分流电压和温度，单冲
        // 8h = 关断
        // 9h = 仅连续总线电压
        // Ah = 仅连续分流电压
        // Bh = 连续分流和总线电压
        // Ch = 仅连续温度
        // Dh = 连续总线电压和温度
        // Eh = 连续温度和分流电压
        // Fh = 连续总线电压、分流电压和温度
        uint16_t SHUNT_MEAS_EN:1;
        uint16_t BUSBAR_MEAS_EN:1;
        uint16_t TEMP_MEAS_EN:1;
        uint16_t CONTMODE_EN:1;
    };

    struct R16_ShuntCal:public Reg16<>{
        static constexpr RegAddress address = 0x02;

        // 14-0 SHUNT_CAL R/W 1000h寄存器为器件提供一个转换常量值，表示用于计算电流值（安培）的
        // 分流电阻。
        // 这个寄存器还设置 CURRENT 寄存器的分辨率。
        // 节 8.1.2下的值计算。

        uint16_t SHUNT_CAL:15;
        uint16_t __RESV1__:1;
    };

    

    struct R8_ShuntTempco:public Reg8<>{
        static constexpr RegAddress address = 0x03;

        // 13-0
        // TEMPCO R/W 0h 分流器用于温度补偿校正的温度系数。以 +25°C 为基准进行计算。
        // 寄存器的满量程值为 16383ppm/℃。
        // 16 位寄存器提供 1ppm/℃/LSB 的分辨率
        // 0h = 0ppm/°C
        // 3FFFh = 16383ppm/°C
        uint16_t TEMPCO:14;
        uint16_t __RESV1__:2;
    };

    struct R24_reg_shunt_v:public Reg24<>{         
        static constexpr RegAddress address = 0x04;  // 24 bit, 312.5 - 78.125 nV / LSB (ADCRANGE)

        // 23-4
        // VSHUNT R 0h 分流输出上测得的差分电压。二进制补码值。
        // 转换因子：
        // 312.5nV/LSB（ADCRANGE = 0 时）
        // 78.125nV/LSB（ADCRANGE = 1 时）
        const uint32_t  __RESV0__:4 = 0;
        uint32_t VSHUNT:20;
    };


    struct R16_reg_bus_v:public Reg16<>{           
        static constexpr RegAddress address = 0x05;  // 24 bit, 195.3125 uV / LSB, 4 low bits == 0

        // 23-4
        // VBUS R 0h 总线电压输出。二进制补码，但始终为正。
        // 转换因子：195.3125µV/LSB
        // 3-0
        // 保留R 0h 保留。始终读为 0。

        const uint32_t __RESV0__:4 = 0;
        uint32_t VBUS:20;
    };


    struct R16_DieTemp:public Reg16<>{               
        static constexpr RegAddress address = 0x06;  // 16 bit

        // 15-0
        // DIETEMP R 0h 内部芯片温度测量。二进制补码值

        uint16_t DIETEMP:16;
    };


    struct R24_Current:public Reg24<>{               
        static constexpr RegAddress address = 0x07;  // 24 bit

        // 23-4
        // CURRENT R 0h 计算得出的电流输出（单位为安培）。二进制补码值。
        // 节 8.1.2 下的值说明。
        // 3-0
        // 保留R 0h 保留。始终读为 0。
        uint32_t __RESV1__:4;
        uint32_t CURRENT:20;
    };


    struct R24_Power:public Reg24<>{               
        static constexpr RegAddress address = 0x08;  // 24 bit

        // POWER R 0h 计算得出的功率输出。
        // 输出值（单位为瓦特）。
        // 无符号表示。正值。
        // 节 8.1.2 下的值说明。

        uint32_t __RESV1__:4;
        uint32_t POWER:20;
    };


    struct R64_Energy:public Reg64<>{               
        static constexpr RegAddress address = 0x09;  // 40 bit

        // ENERGY R 0h 计算得出的电能输出。
        // 输出值以焦耳为单位。无符号表示。正值。
        // 节 8.1.2 下的值说明。

        uint64_t ENERGY:40;
        uint64_t __RESV1__:24;
    };


    struct R64_Charge:public Reg64<>{               
        static constexpr RegAddress address = 0x0A;  // 40 bit

        // 39-0
        // CHARGE R 0h 计算得出的电荷输出。输出

        uint64_t CHARGE:40;
        uint64_t __RESV1__:24;
    };


    struct R16_Alert:public Reg16<>{            
        static constexpr RegAddress address = 0x0B;  // 16 bit

        // 0 MEMSTAT R/W 1h如果在器件修整存储器空间中检测到校验和错误，则该位设置为 0。
        // 0h = 存储器校验和错误
        // 1h = 正常运行
        uint16_t MEMSTAT:1;

        // 1h = 转换完成
        // 当 ALATCH=1 时，通过读取该寄存器或启动新的触发转换来清除该
        // 位。
        uint16_t DONE:1;

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
        // INA228
        // ZHCSN47A – JANUARY 2021 – REVISED MAY 2022 www.ti.com.cn
        // 26 Submit Document Feedback Copyright © 2022 Texas Instruments Incorporated
        // Product Folder Links: INA228
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

        // 10 CHARGEOF R 0h该位表示 CHARGE 寄存器的运行状况。
        // 如果 40 位 CHARGE 寄存器已溢出，则该位设置为 1。
        // 0h = 正常
        // 1h = 溢出
        // 读取 CHARGE 寄存器时清除。
        uint16_t CHARGEOF:1;

        // 11 ENERGYOF R 0h该位表示 ENERGY 寄存器的运行状况。
        // 如果 40 位 ENERGY 寄存器已溢出，则该位设置为 1。
        // 0h = 正常
        // 1h = 溢出
        // 读取 ENERGY 寄存器时清除。
        uint16_t ENERGYOF:1;

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


    struct R16_SOVL:public Reg16<>{            
        static constexpr RegAddress address = 0x0B;  // 16 bit,  shunt overvoltage threshold

        // 15-0 SOVL R/W 7FFFh设置用于比较值的阈值，以检测分流过压（过流保护）。二进制补码
        // 值。转换因子：5µV/LSB（ADCRANGE = 0 时）
        // 1.25µV/LSB（ADCRANGE = 1 时）。
        uint16_t SOVL;
    };


    struct R16_SUVL:public Reg16<>{            
        static constexpr RegAddress address = 0x0C;  // 16 bit,  shunt undervoltage threshold

        // 15-0 SUVL R/W 8000h设置用于比较值的阈值，以检测分流欠压（欠流保护）。二进制补码
        // 值。转换因子：5µV/LSB（ADCRANGE = 0 时）
        // 1.25µV/LSB（ADCRANGE = 1 时）。
        uint16_t SUVL;
    };


    struct R16_BOVL:public Reg16<>{            

        
        static constexpr RegAddress address = 0x0D;  // 16 bit,  bus overvoltage threshold

        // 15
        // 保留R 0h 保留。始终读为 0。
        // 14-0
        // BOVL R/W 7FFFh 设置用于比较值的阈值，以检测总线过压（过压保护）。无符号表
        // 示，仅限正值。转换因子：3.125mV/LSB。
        uint16_t BOVL:15;
        uint16_t __RESV__:1;
    };


    struct R16_BUVL:public Reg16<>{            
        static constexpr RegAddress address = 0x0E;  // 16 bit,  bus undervoltage threshold

        // 15
        // 保留R 0h 保留。始终读为 0。
        // 14-0
        // BUVL R/W 0h 设置用于比较值的阈值，以检测总线欠压（欠压
        uint16_t BUVL:15;
        uint16_t __RESV__:1;
    };
    
    struct R16_TempLimit:public Reg16<>{        
        static constexpr RegAddress address = 0x10;  // 16 bit,  temp over

        // 15-0
        // TOL R/W 7FFFh 设置用于比较值的阈值，以检测过热测量值。二进制补码值。
        // 在此字段中输入的值直接与 DIETEMP 寄存器中的值进行比较，以确
        // 定是否存在过热情况。转换因子：7.8125m°C/LSB
        uint16_t TOL:16;
    };


    struct R16_PowerLimit:public Reg16<>{         

        static constexpr RegAddress address = 0x11;  // 16 bit,  power over

        // 15-0
        // POL R/W FFFFh 设置用于比较值的阈值，以检测功率高于上限测量值。无符号表示，
        // 仅限正值。
        // 在此字段中输入的值直接与 POWER 寄存器中的值进行比较，以确定
        // 是否存在超出功率情况。转换因子：256 × 功率 LSB。
        uint16_t POL:16;
    };


    struct R16_ManfId:public Reg16<>{        
        static constexpr RegAddress address = 0x3E;

        // 15-0
        // MANFID R 5449h 以 ASCII 格式读回 TI。
        static constexpr uint16_t KEY = (int('T') << 8) | 'I';
        uint16_t MANFID;
    };


    struct R16_DieId:public Reg16<>{          
        static constexpr RegAddress address = 0x3F;

        // 15-4
        // DIEID
        // 3-0
        // R 228h 存储器件标识位。
        // REV_ID R 1h 器件修订版本标识。
        static constexpr uint16_t KEY = 0x2281;
        uint16_t DIEID;
    };
};

}