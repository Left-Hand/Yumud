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

    // PAGE20

    // SINGLE_TAP 
    // 加速度值超过THRESH_TAP寄存器(地址0x1D)值，并且持
    // 续时间小于DUR寄存器(地址0x21)规定的时间范围的时
    // 候，SINGLE_TAP中断置位。

    // DOUBLE_TAP 
    // 两次加速度事件超过THRESH_TAP寄存器(地址0x1D)值，
    // 并且持续时间小于DUR寄存器(地址0x21)的规定时间范围
    // 的时候，DOUBLE_TAP中断置位。第二次敲击开始于
    // Latent寄存器(地址0x22)规定的时间之后，但在Window寄
    // 存器(0x23)规定时间内。详情见敲击检测部分。

    // Activity 
    // 加速度值大于THRESH_ACT寄存器(地址0x24)存储值时，
    // Activity(活动)中断置位，由任一轴参与，通过ACT_IN-ACT_CTL寄存器(0x27)置位。

    // Inactivity 
    // 加速度值小于THRESH_INACT寄存器(地址0x25)的存储值
    // 时，Inactivity(静止)位置位，所有轴参与，多于TIME_IN-ACT寄存器(地址 0x26)规定的时间，通过 ACT_IN-ACT_CTL寄存器(地址0x27)置位。TIME_INACT最大值为
    // 255秒。

    // FREE_FALL 
    // 加速度值小于THRESH_FF寄存器(地址0x28)的存储值时，
    // FREE_FALL置位，大于TIME_FF寄存器(地址0x29)所有轴
    // (逻辑与)所规定的时间。FREE_FALL中断不同于静止中断，
    // 因为：所有轴始终参与，并为逻辑“和”的形式，定时器周
    // 期小得多(最大值：1.28秒)，始终为直流耦合操作模式。

    //PAGE21

    // Overrun 
    // 当有新采样点更新了未被读取的前次采样点时，Overrun
    // 中断置位。Overrun功能与FIFO的工作模式有关。当FIFO
    // 工作在Bypass模式下，如果有新采样点更新了DATAX、
    // DATAY和DATAZ寄存器(地址0x32至0x37)里的数值，则
    // Overrun中断置位。在其他模式下，只有FIFO被存满时，
    // Overrun中断才会置位。读取FIFO内容时，Overrun位自动
    // 清零。

    // FIFO 
    // ADXL345包含嵌入式存储器管理系统(专利申请中)，采用
    // 32位FIFO，可将主机处理器负荷降至最低。缓冲分四种模
    // 式：旁路模式、FIFO模式、流模式和触发器模式(参见
    // FIFO模式)。在FIFO_CTL寄存器(地址0x38)内设置FIFO_-
    // MODE位(位[D7：D6])，可选择各模式。

    // 旁路模式
    // 旁路模式下，FIFO不可操作，因此，仍然为空。

    // FIFO模式
    // 在FIFO模式下，x、y、z轴的测量数据存储在FIFO中。当
    // FIFO中的采样数与FIFO_CTL寄存器(地址0x38)采样数位规
    // 定的数量相等时，水印中断置位。FIFO继续收集样本，直
    // 到填满(x、y和z轴测量的32位样本)，然后停止收集数据。
    // FIFO停止收集数据后，该器件继续工作，因此，FIFO填满
    // 时，敲击检测等功能可以使用。水印中断继续发生，直到
    // FIFO样本数少于FIFO_CTL寄存器的样本位存储值。

    // 流模式
    // 在流模式下，x、y、z轴的测量数据存储在FIFO中。当
    // FIFO中的采样数与FIFO_CTL寄存器(地址0x38)采样数位规
    // 定的数量相等时，水印中断置位。FIFO继续收集样本，保
    // 存从x、y和z轴收集的最新32位样本。新数据更新后，丢弃
    // 旧数据。水印中断继续发生，直到FIFO样本数少于
    // FIFO_CTL寄存器的样本位存储值。

    // 触发器模式
    // 触发器模式下，FIFO收集样本，保存从x、y和z轴收集的
    // 最新32位样本。触发事件发生后，中断被发送到INT1引脚
    // 或INT2引脚(取决于FIFO_CTL寄存器的触发位)，FIFO保
    // 持最后n个样本(其中n为FIFO_CTL寄存器样本位规定值)，
    // 然后在FIFO模式下运行，只有FIFO没有填满时，才会收集
    // 新样本。从触发事件发生到开始从FIFO读取数据，至少有
    // 5 μs延迟，允许FIFO丢弃和保留必要样本。触发器模式复位
    // 后，才能识别附加触发事件。要复位触发器模式，设置器
    // 件为旁路模式，然后再设置回触发器模式。请注意，应首
    // 先读取FIFO数据，因为器件旁路模式下，FIFO清零。


    // 从FIFO中读取数据
    // 从DATAX、DATAY和DATAZ寄存器(地址0x32至0x37)读
    // 取FIFO数据。当FIFO为FIFO模式、流模式或触发器模式
    // 时，DATAX，DATAY和DATAZ寄存器读取存储在FIFO中
    // 的数据。每次从FIFO读取数据，x、y和z轴的最早数据存
    // 入DATAX、DATAY和DATAZ寄存器。
    // 如果执行单字节读取操作，当前FIFO样本的剩余数据字节
    // 会丢失。因此，所有目标轴应以突发(或多字节)读取操作
    // 进行读取。为确保FIFO完全弹出(即新数据完全移动到
    // DATAX、DATAY和DATAZ寄存器)，读取数据寄存器结
    // 束后至FIFO重新读取或FIFO_STATUS寄存器(地址0x39)读
    // 取前，至少必须有5 μs延迟。从寄存器0x37至寄存器0x38
    // 的转变或 引脚变为高电平为标志，读取数据寄存器结束。
    // 对于1.6 MHz或更低频率下的SPI操作，传输的寄存器处理
    // 部分充分延迟，确保FIFO完全弹出。对于大于1.6 MHz频率
    // 下的SPI操作，有必要拉高 引脚确保5 μs的总延迟；否则，
    // 延迟会不充分。5 MHz操作的必要总延迟最多为3.4 μs。使
    // 用I2C模式时，不用担心这个问题，因为通信速率足够低，
    // 确保FIFO读取的充分延迟。

    // ADXL345具备自测功能，可同时有效测试机械系统和电子
    // 系统。自测功能使能时(通过DATA_FORMAT寄存器(地址
    // 0x31)的SELF_TEST位)，有静电力施加于机械传感器之
    // 上。与加速度同样的方式，静电力驱使力敏传感元件移
    // 动，且有助于器件体验加速度。增加的静电力导致x、y和z
    // 轴上的输出变化。因为静电力与VS^2成正比，所以输出随着VS
    // 而变化。该效应如图43所示。表14所示的比例因子可用
    // 来为不同的电源电压VS调整预期的自测输出限值。
    // ADXL345的自测功能也表现为双模行为。然而，由于双峰
    // 性，表1和表15至表18所示的限值对潜在的自检值都有
    // 效。在低于100 Hz或在1600 Hz的数据速率下，使用自测功
    // 能，可能产生超出这些限值的值。因此，器件必须为正常
    // 功率运行(BW_RATE寄存器的(地址0x2C) LOW_POWER位
    // = 0)，通过800 Hz或3200 Hz，置于100Hz的数据速率，以便
    // 自测功能正常运行。
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

        void set_double_click_latency_ms(const iq16 ms){
            static constexpr iq16 LSB_PER_MS = iq16(1 / 1.25);
            data = math::ceil_cast<uint8_t>(ms * LSB_PER_MS);
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