# Yumud

## catalog

##### docu 文档
##### algo 数据算法
##### dsp 信号处理
##### types 各种类型

- [ ] IQ(IQMATH)
    - [x] 支持四则运算 大小比较 类型转换
    - [x] 支持超越函数
    - [ ] 支持std::超越函数
    - [ ] 使用模板重构

##### protocol 通信协议
##### drivers 设备驱动
##### scripts 脚本文件
##### sys 系统设定
##### thirdparty 第三方库
##### src 主要源代码
##### hal 物理抽象层
##### robots 机器人学相关代码



- [ ] IOStream(输入输出流)
    - [ ] `printf` `cout`(内建输出流操作) 
    - [x] `println` `print` `<<` (基本输入流操作)
    - [ ] `format` (字符串格式化) 


- [x] GPIO(IO相关代码)
    - [x] bitband(位带操作)
    - [x] Gpio(单个IO)
    - [x] Port(IO端口)
    - [x] VirtualPort(虚拟IO端口)(用于将多个片上与片外端口按顺序绑定至一个IO端口上)

- [ ] TIM(定时器相关代码)
    - [x] 中断与回调函数绑定

    - [ ] PWMChannel PWM输出概念
        - [x] AW9523Pwm 
        - [ ] GPIOPwm

    - [ ] CaptureChannel 输入捕获概念
        - [x] 
        - [ ] ExtiCapture 基于EXTI的输入捕获

    - [ ] PWMModule PWM输出端口

    - [ ] TimerOC(定时器输出)
        - [ ] 
        - [ ] Hrpwm(高精度PWM)
    
    - [ ] TimerIC(定时器输入捕获)
        - [ ] 



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

## ThirdParty
使用了
SSTL:静态STL以避免堆溢出
BETTER ENUM:枚举的反射库 主要用于枚举转字符串分析状态机状态
JSON：如其名 尚未使用


## 开发环境
vscode
eide
openocd
riscv-none-elf-gcc 12(wch特供版)

## 编译选项
os/o3
cpp >= cpp20