#pragma once


#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/AnalogDeviceIMU.hpp"

// https://item.szlcsc.com/607233.html

namespace ymd::drivers{

struct ADXL345_Prelude{
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x1D);
    static constexpr uint8_t VALID_DEVICE_ID = 0xE5;

    enum class RegAddr:uint8_t{
        DeviceID = 0x00,
        TapThreshold = 0x1D,
        OffsetX = 0x1E,
        OffsetY = 0x1F,
        OffsetZ = 0x20,
        TapDuration = 0x21,
        TapLatency = 0x22,
        TapWindow = 0x23,
        ActivityThreshold = 0x24,
        InactivityThreshold = 0x25,
        InactivityTime = 0x26,
        AxisEnabler = 0x27,
        FreefallThreshold = 0x28,
        FreefallTime = 0x29,
        AxisControlForSingle2DoubleTap = 0x2A,
        SourceOfSingle2DoubleTap = 0x2B,
        DataRate = 0x2C,
        PowerCtl = 0x2D,
        InterruptEnabler = 0x2E,
        InterruptMap = 0x2F,
        InterruptSource = 0x30,
        DataFormat = 0x31,
        DataX0 = 0x32,
        DataX1 = 0x33,
        DataY0 = 0x34,
        DataY1 = 0x35,
        DataZ0 = 0x36,
        DataZ1 = 0x37,
        FifoCtrl = 0x37,
        FifoStatus = 0x38
    };

    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AccFs:uint8_t{
        _2G = 0,
        _4G = 1,
        _8G = 2,
        _16G = 3
    };

    enum class DataRate:uint8_t{
        _0_1Hz = 0,
        _0_2Hz, 
        _0_39Hz, 
        _0_78Hz,
        _1_56Hz,
        _6_25Hz,
        _12_5Hz,
        _25Hz,
        _50Hz,
        _100Hz,
        _200Hz,
        _400Hz,
        _800Hz,
        _1600Hz,
        _3200Hz
    };

    enum class SleepingDataRate:uint8_t{
        _8Hz = 0b00,
        _4Hz = 0b01,
        _2Hz = 0b10,
        _1Hz = 0b11,
    };

    enum class WakeupFreq:uint8_t{
        HZ8, HZ4, HZ2, HZ1
    };

    enum class MeasureRange:uint8_t{
        G2, G4, G8, G16
    };


    enum class FifoMode:uint8_t{
        // FIFO为旁路模式。
        Bypass = 0b00,
        // FIFO收集多达32个值，然后停止收集
        // 数据，只有FIFO未填满时，才收集新
        // 的数据。 1 0 流
        Fifo = 0b01, 
        // FIFO保存最后32个数据值。FIFO填满
        // 时，新数据覆盖最早的数据。
        Stream = 0b10, 
        // 通过触发位触发时，FIFO在触发事件
        // 前保存最后的数据样本，然后继续收
        // 集数据，直到填满。FIFO填满后，不
        // 再收集新的数据。
        Trigger = 0b11
    };



};

struct ADXL345_Regset final :public ADXL345_Prelude{
    struct R8_DeviceID:public Reg8<>{
        static constexpr auto address = RegAddr::DeviceID;
        uint8_t id;
    }DEF_R8(deviceid_reg)

    struct R8_TapThreshold:public Reg8<>{
        uint8_t data;
    };

    // DUR为8位寄存器，包含未签名时间值，表示必须是
    // THRESH_TAP阈值之上的事件才能称之为敲击事件的最大
    // 时间。比例因子为625 μs/LSB。值为0时，禁用单击/双击
    // 功能。
    struct R8_TapDuration:public Reg8<>{
        uint8_t data;
    };

    // Latent寄存器为8位寄存器，包含无符号时间值，表示从敲
    // 击事件检测到时间窗口(由Window寄存器定义)开始的等待
    // 时间，在此期间，能检测出可能的第二次敲击事件。比例
    // 因子为1.25 ms/LSB。值为0时，禁用双击功能。
    struct R8_TapLatency:public Reg8<>{
        static constexpr auto address = RegAddr::TapLatency;
        uint8_t data;

        void set_double_click_latency_ms(const q16 ms){
            static constexpr q16 LSB_PER_MS = q16(1 / 1.25);
            data = ceil_cast<uint8_t>(ms * LSB_PER_MS);
        }

        void disable_double_click(){
            data = 0;
        }
    };

    // Window寄存器为8位寄存器，包含未签名的时间值，表示
    // 延迟时间(由Latent寄存器确定)期满后的时间量，在此期
    // 间，能开始进行第二次有效敲击。比例因子为1.25 ms/LSB。
    // 值为0时，禁用双击功能。
    struct R8_TapWindow:public Reg8<>{
        uint8_t data;
    };

    struct R8_ActivityThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_InactivityThreshold:public Reg8<>{
        uint8_t data;
    };

    struct R8_InactivityTime:public Reg8<>{
        uint8_t z_dis;
    };

    struct R8_AxisEnabler:public Reg8<>{
        
        uint8_t inact_z :1;
        uint8_t inact_y :1;
        uint8_t inact_x :1;
        uint8_t inact_a_cor_dc:1;
        uint8_t act_x :1;
        uint8_t act_y :1;
        uint8_t act_z :1;
        uint8_t act_a_cor_dc:1;
        
    };

    struct R8_FreefallThreshold:public Reg8<>{
        // THRESH_FF寄存器为8位寄存器，保存未签名格式的阈
        // 值，用于自由落体检测。所有轴的加速度与THRESH_FF
        // 的值相比较，以确定是否有自由落体事件发生。比例因子
        // 为62.5 mg/LSB。请注意，如果自由落体中断被使能，值为
        // 0 mg可能导致工作异常。建议采用300 mg与600 mg (0x05至
        // 0x09)之间的值。
    };

    struct R8_FreefallTime:public Reg8<>{
        // TIME_FF寄存器为8位寄存器，存储未签名值，表示所有
        // 轴的值必须小于THRESH_FF的最小时间，以生成自由落
        // 体中断。比例因子为5 ms/LSB。如果使能自由落体中断，值
        // 为0时，可能导致工作异常。建议采用100 ms与350 ms (0x14
        // 至0x46)之间的值。
        uint8_t data;
    };

    struct R8_AxisControlForSingle2DoubleTap:public Reg8<>{
        
        uint8_t tapZenabled :1;
        uint8_t tapYenabled :1;
        uint8_t tapXenabled :1;
        uint8_t doubleCheck :1;
        uint8_t __resv__ :4;
        
    };

    struct R8_SourceOfSingle2DoubleTap:public Reg8<>{
        
        uint8_t tapZSource :1;
        uint8_t tapYSource :1;
        uint8_t tapXSource :1;
        uint8_t aSleep:1;
        uint8_t actZsource :1;
        uint8_t actYsource :1;
        uint8_t actXsource :1;
        uint8_t __resv__:1;
        
    };

    struct R8_DataRate:public Reg8<>{
        // 这些位能选择器件带宽和输出数据速率(详情见表7和表
        // 8)。默认值为0x0A，转换为100 Hz的输出数据速率。应选择
        // 适合所选通信协议和频率的输出数据速率。选择太高输出
        // 数据速率和低通信速度会导致采样丢弃。
        DataRate data_rate :4;

        // LOW_POWER位设置为0，选择正常操作，设置为1，选择
        // 低功率操作，而此时噪声有所增加(详情见功率模式部分)。
        uint8_t low_power :1;
        uint8_t __resv__ :3;
        
    };

    struct R8_PowerCtl:public Reg8<>{
        
        SleepingDataRate sleep_datarate :2;

        // 休眠位设置为0，将器件置于普通工作模式，设置为1，置
        // 于休眠模式。休眠模式抑制DATA_READY，停止对FIFO
        // 的数据传输，切换至唤醒位规定的采样速率。休眠模式
        // 下，只有活动功能可以使用。当DATA_READY中断被抑
        // 制，输出数据寄存器(寄存器0x32至寄存器0x37)仍然以唤
        // 醒位(D1：D0)设置的采样速率更新。
        // 休眠位清零后，建议将器件置于待机模式，然后复位为测
        // 量模式，随后写入。这样做是为了确保如果手动禁用睡眠
        // 模式，该器件适当偏置，否则，休眠位后的前几个数据样
        // 本清零后，可能会有额外的噪声，特别是该位清零后器件
        // 为休眠状态时。
        uint8_t sleep :1;

        // 测量位设置为0，将器件置于待机模式，设置为1，置于测
        // 量模式。ADXL345待机模式下，以最小功耗上电。
        uint8_t measure:1;

        // 设置链接位，AUTO_SLEEP位设置为1，自动休眠功能使
        // 能。该模式下，如果使能静止功能，检测出静止，ADXL345
        // 自动切换到休眠模式(即至少在TIME_INACT规定时间里，
        // 加速度值低于THRESH_INACT值)。如果活动功能也使
        // 能，ADXL345从活动检测后自动唤醒，以BW_RATE寄存
        // 器设置的输出数据速率重新运行。AUTO_SLEEP位设置为
        // 0，禁用自动切换至休眠模式。有关休眠模式的更多详
        // 情，参见休眠位部分。
        // 如果链接位未设置，AUTO_SLEEP特性禁用，设置AUTO_ 
        // SLEEP位不会影响器件运行。有关使用链接特性的更多详
        // 情，请参考链接位或链接模式部分。
        // AUTO_SLEEP位清零后，建议将器件置于待机模式，然后
        // 复位为测量模式，随后写入。这样做是为了确保如果手动
        // 禁用睡眠模式，该器件适当偏置，否则，AUTO_SLEEP位
        // 后的前几个数据样本清零后，可能会有额外的噪声，特别
        // 是该位清零后器件为休眠状态时。
        uint8_t auto_sleep:1;

        // 将有使能活动和静止功能的链接位设置为1，延迟活动功
        // 能开始，直到检测到静止。检测到活动后，静止检测开
        // 始，活动检测停止。该位串行链接活动和静止功能。此位
        // 设置为0时，静止功能和活动功能同时进行。其他信息见
        // 链接模式部分。
        // 链接位清零后，建议将器件置于待机模式，然后复位为测
        // 量模式，随后写入。这样做是为了确保如果手动禁用休眠
        // 模式，该器件适当偏置，否则，链接位后的前几个数据样
        // 本清零后，可能会有额外的噪声，特别是该位清零后器件
        // 为休眠状态时。
        uint8_t link:1;
        
    };


    struct _R8_InterruptMask:public Reg8<>{
        uint8_t overrun :1;
        uint8_t watermark :1;
        uint8_t freefall :1;
        uint8_t inactivity:1;
        uint8_t activity:1;
        uint8_t double_tap:1;
        uint8_t single_tap:1;
        uint8_t data_ready:1;
        
    };

    struct R8_InterruptEnabler:_R8_InterruptMask{
        // 寄存器设置位值为1，使能相应功能，生成中断，设置为0
        // 时，阻止这些功能产生中断。DATA_READY位、水印位
        // 和溢出位仅使能中断输出；这些功能总是处于使能状态。
        // 建议使能其输出前进行中断配置。
    };

    struct R8_InterruptMap:_R8_InterruptMask{
        // 寄存器任意位设置为0，发送各自中断到INT1引脚，设置
        // 为1，则发送到INT2引脚。给定引脚的所有选定中断都为
        // 逻辑“或”。
    };

    struct R8_InterruptSource:_R8_InterruptMask{
        static constexpr auto address = RegAddr::InterruptSource;
        // 寄存器位设置为1表示各自功能触发事件，值为0则表示没
        // 有相应的事件发生。不管INT_ENABLE寄存器设置如何，
        // 如果有相应的事件发生，总是设置DATA_READY位、水
        // 印位和溢出位，并通过读取DATAX、DATAY和DATAZ寄
        // 存器将数据清零。如FIFO部分FIFO模式所述，DATA_ 
        // READY和水印位可能需要多次读取。通过读取INT_ 
        // SOURCE寄存器，其他位和相应的中断清零。
    };

    struct R8_DataFormat:public Reg8<>{
        static constexpr auto address = RegAddr::DataFormat;
        AccFs acc_fs :2;

        // 对齐位设置为1，选择左对齐(MSB)模式，设置为0，选择
        // 右对齐模式，并带有符号扩展功能。
        uint8_t is_left_align:1;

        // 当此位值设置为1，该器件为全分辨率模式，输出分辨率
        // 随着范围位设置的g范围，以4 mg/LSB的比例因子而增加。
        // FULL_RES位设置为0时，该器件为10位模式，范围位决定
        // 最大g范围和比例因子。
        uint8_t full_resolution_en:1;
        uint8_t __resv__ :1; //hardware 0

        // INT_INVERT位值为0，设置中断至高电平有效，值为1，
        // 则设置至低电平有效。
        uint8_t interrupt_invert_en :1;
        uint8_t spi_3wire_else_4wire:1;

        // SELF_TEST位设置为1，自测力应用至传感器，造成输出
        // 数据转换。值为0时，禁用自测力。
        uint8_t self_test_en:1;
        
    }DEF_R8(data_format_reg)


    struct R8_FifoCtrl:public Reg8<>{
        static constexpr auto address = RegAddr::FifoCtrl;
        // 样本位功能
        // 旁路 无。
        // FIFO 指定触发水印中断需要的FIFO条目数。
        // 流 指定触发水印中断需要的FIFO条目数。
        // 触发器 指定触发事件之前在FIFO缓冲区要保留的
        // FIFO样本数。
        uint8_t samples:5;

        // 这些位的功能取决于选定的FIFO模式(见表23)。样本位设
        // 置值为 0时，不管选择哪种 FIFO模式，立即在 INT_ 
        // SOURCE寄存器设置水印状态位。触发器模式下，如果样
        // 本位值为0，可能会出现工作异常
        uint8_t trigger :1;
        FifoMode fifo_mode:2;
    };

    struct R8_FifoStatus:public Reg8<>{
        // 这些位报告FIFO存储的数据值的数量。通过DATAX、
        // DATAY和DATAZ寄存器，可从FIFO收集数据。FIFO应采
        // 取突发读取模式或多字节读取模式，因为FIFO的任意(单
        // 字节或多字节)读取后，每个FIFO水平清零。FIFO存储最
        // 多32项条目，相当于任何时间内最多有33项条目，因为器
        // 件的输出滤波器有一项附加条目。
        uint8_t entires:6;
        uint8_t __resv__ :1;

        // FIFO_TRIG位值为1表示有触发事件发生，值为0表示无
        // FIFO触发事件发生。
        uint8_t fifo_trig:1;
        
    }DEF_R8(fifo_status_reg)

};

}