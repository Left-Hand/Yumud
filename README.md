# Yumud

## catalog

##### docu 文档

##### algo 数据算法
- [x] random 伪随机数发生器
- [x] hash 函数
##### dsp 信号处理

- [x] 编译期超越函数计算

- [ ] 控制器
    - [x] PID 控制器
    - [ ] 模糊PID控制器
    - [ ] LQR 控制器
    - [ ] MPC 控制器
- [ ] fft(未测试)

- [x] 滤波器
    - [x] 卡尔曼滤波器
    - [x] 低通滤波器
    - [x] 高通滤波器
    - [x] 施密特触发器
    - [x] 抖动滤波器
    - [x] 毛刺滤波器
- [x] cordic 三角运算单元

- [x] 观测器
    - [x] 一次观测器

- [ ] 线性连续时间系统
- [x] 查找表
- [ ] 振荡器
- [x] pll 锁相环

##### types 各种类型

- [x] basis 三维旋转矩阵(godot)
- [x] buffer 缓冲结构
    - [x] ringbuffer 环形缓冲
    - [ ] pingpongbuffer 乒乓缓冲
    - [ ] stack 堆栈缓冲

- [x] color RGBA颜色表述
- [x] complex 复数
- [x] image 图像类
    - [x] font 字体类
    - [ ] cursor 光标类
    - [x] packedImage 压缩二值化图片
    - [x] painter 绘图算法驱动

- [x] IQ(IQMATH)
    - [x] 支持四则运算 大小比较 类型转换
    - [x] 支持超越函数
    - [ ] 支持std::超越函数
    - [ ] 使用模板重构

- [ ] matrix 矩阵类
- [ ] polar 极坐标类
- [x] quat 四元数类(godot)
- [x] range 区间类
- [x] rect2 矩形区域类
- [x] string 字符串类(arduino)
- [x] vector2 二维向量类(godot)
- [x] vector3 三维向量类(godot)
- [x] rgb 各类颜色结构体
- [x] uint24_t 24位数据

##### protocol 通信协议

- [ ] `CANSF` 自研轻量CAN驱动
- [ ] `CANOPEN` 大量落地通用CAN框架

##### drivers 设备驱动

日后谈

##### scripts 脚本文件
##### sys 系统设定

- [x] tasker 非抢占式任务驱动器

##### thirdparty 第三方库

1. SSTL:静态STL以避免堆溢出
2. BETTER ENUM:枚举的反射库 主要用于枚举转字符串分析状态机状态
3. JSON：如其名 尚未使用

##### src 主要源代码
##### hal 物理抽象层


- [ ] IOStream(输入输出流)
    - [ ] `printf` `cout`(内建输出流操作) 
    - [x] `println` `print` `<<` (基本输入流操作)
    - [ ] `format` (字符串格式化) 


- [x] GPIO(IO相关代码)
    - [x] bitband(位带操作)
    - [x] Gpio(单个IO)
    - [x] Port(IO端口)
    - [x] VirtualPort(虚拟IO端口)(用于将多个片上与片外端口按顺序绑定至一个IO端口上)

- [x] UART(已验证921600波特率的长时间压力测试)
    - [x] 对基本输入输出流的支持
    - [x] 环形缓冲区支持
    - [x] DMA/中断支持
    - [ ] LinBus/智能卡

- [x] SPI(已验证72Mhz的长时间压力测试)
    - [x] 一般数据收发
    - [ ] DMA数据收发
    - [x] 虚拟片选集线器
    - [x] 软件SPI

- [x] I2C(以验证3.4Mhz的软件I2C)
    - [x] 软件I2C
    - [x] 硬件I2C
    - [x] 一般数据收发
    - [ ] DMA数据收发
    - [ ] SMbus

- [ ] I2S(以验证软件I2S只发)
    - [x] 软件I2S只发
    - [x] TM8211
    - [ ] 硬件I2S
    - [ ] I2S读取

- [x] CAN(机器人局域网总线)
    - [x] 信箱及FIFO驱动
    - [x] 输入输出环形缓冲区
    - [x] 标准ID与拓展ID下Msg发送接收
    - [x] 标准ID下的过滤器

- [ ] Adc
    - [x] 任意通道单路信号采集
    - [x] 任意通道多路信号采集(DMA)
    - [ ] 片上温度采集
    - [ ] 虚拟ADCChannel类

- [x] DVP
    - [x] MT9V034摄像头驱动

- [x] CRC
- [x] OPA
- [x] NVIC
- [x] EXTI

- [x] FLASH
    - [ ] 普通FLASH读写(以弃用)
    - [x] 快速FLASH读写

- [x] TIM(定时器相关代码)
    - [x] 中断与回调函数绑定
    - [x] 编码器模式
    - [ ] PWMModule PWM输出集线器

    - [x] PWMChannel PWM输出概念
        - [x] AW9523Pwm 基于AW9523的pwm
        - [x] GPIOPwm 使用GPIO配合定时触发模拟pwm

        - [ ] TimerOC(定时器输出)
            - [x] TimerOC
            - [ ] Hrpwm(高精度PWM)

    - [ ] CaptureChannel 输入捕获概念 
        - [x] ExtiCapture 基于EXTI的输入捕获
        - [ ] TimerIC(定时器输入捕获)

- [ ] TCP/UDP
- [ ] USB
- [ ] BLE


##### robots 机器人学相关代码

- [ ] 步进电机FOC算法

- [x] `buck.hpp` BUCK驱动
- [x] `pmdc.hpp` 直流有刷电机驱动算法
- [ ] `spread_cycle.hpp` 对SpeadCycle斩波驱动的低劣模仿


## info

项目架构不稳定 不要轻易fork！！！！！

算是用来练习git的项目,孩子自个写着玩的

用c写嵌入式太膈应了 面向对象多好

目前支持的芯片只有ch32(性价比之王yyds)

参考了HAL和arduino 但使用了抽象开销成本更小与更安全的方式编写

使用cpp20编写 请使用gcc12+进行编译
使用vscode下的eide搭建开发环境

## SDK
厂方sdk库已改动
请使用用本仓库提供的sdk 位于sys/kernel下
大量使用iqmath在无浮点单元的情况下进行高速数学运算 请选择16位IQ数（调整宏定义GLOBAL_Q）否则可能会出现意想不到的问题


## 开发环境
vscode
eide
openocd
riscv-none-elf-gcc 12(wch特供版)

## 编译选项
os/o3
cpp >= cpp20