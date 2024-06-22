# Yumud

# 基于C++20的嵌入式开发框架

"用C写嵌入式太膈应了 面向对象多好"
编写着结构体以及调用初始化函数的我如此想在目睹了arduino缺乏对底层的精细化控制与HAL尚未摆脱真正的硬件抽象的我返回了标准库。但转念一想利用CPP的高级语法，我们可以让嵌入式从这些令人作呕的外设配置变成了调度抽象语义的游戏，才能够使我们编写出更快更清爽更加与现代设计模式接轨的代码,不是吗？


无需复杂的结构体配置点亮一盏led

```c++

int main(){
    //...prework()
    auto & bled = portC[13];
    bled.outpp();
    while(true){
        bled = !bled;//blink XD
        delay(200);
    }
}

```

又或是开启互补PWM快速验证一个BUCK

```C++
int main(){
    //...prework()

    // config timer at 36Khz
    timer1.init(36000);

    // get pwm for timer channel 1
    auto & pwm = timer1.oc(1);
    pwm.init();

    // set clamp to avoid overflow
    pwm.setClamp({0.1, 0.95});

    //to enable complementary for example
    auto & pwmn = timer1.ocn(1);//
    pwmn.init();

    // or just
    timer1.ocn(1).init();

    //init bdtr of timer1 with 200ns-deadzone by default
    timer1.initBdtr();

    //we can do this
    while(true){
        // t(time) is automatically changed by systick

        // we use overload so pwm can be assigned to a real_t
        pwm = 0.5 + 0.5 * sin(t);
    }

    //or...

    // use 0:0 pre:sub interrupt priority
    timer1.enableIt(TimerIT::Update, {0,0});

    //bind callback function
    timer1.bindCb(TimerIT::Update, [&](){pwm = 0.5 + 0.5 * sin(t);});
}

```

再或者通过CAN指令精细控制云台的角度

```C++

int main(){
    //...prework()

    can1.init(1000000);
    uart1.init(921600, CommMethod::DMA);
    UartHw & logger = uart1;

    Servo   servo   {timer1.ch(1)};
    Stepper stepper {timer1.ch(2);}
    Gimbal  gimbal  {servo, stepper};
    // automatically init all the things
    gimbal.init();

    while(true){
        static real_t angle;
        if(can1.available()){
            //read msg from can fifo
            auto & msg = can1.read();
            
            //using template to load message for remote
            msg.load(angle);

            //just print the angle
            logger.println("angle is:", angle);
        }

        gimbal.move_to(Vector2(1, 0).rotated(angle));
    }
}

```

当我们用对象构建起代码时 一切是如此自然且清晰
我们脱离了厂家的SDK这套框架可以被拓展至各类处理器中


## ⚠️声明

目前支持的芯片只有ch32(性价比之与皮实王yyds)

参考了HAL和arduino 但使用了抽象开销成本更小与更安全的方式编写

使用cpp20编写 请使用gcc12+进行编译
使用vscode下的eide搭建开发环境

厂方sdk库已改动
请使用用本仓库提供的sdk 位于sys/kernel下

大量使用iqmath在无浮点单元的情况下进行高速数学运算 请选择16位IQ数（调整宏定义GLOBAL_Q）否则可能会出现意想不到的问题

## 🌱参与贡献

算是用来练习git的项目, 第一次使用git不太熟悉 孩子自个写着玩的

由于目前项目架构尚不稳定 不要轻易fork！！！！！


## 🔨开发环境

#### GCC
由于本项目基于RISCV结构的微处理器（ARMCC or ARMCLANG不支持）故使用GCC作为编译工具
且本项目使用到了CPP20的特性进行开发 故GCC的版本不低于12
同时由于WCH为了支持快速中断 为GCC添加了魔改的中断关键字 故应使用WCH定制的GCC12

需要安装[MounRiver Studio](http://www.mounriver.com/)以获取我们所需要的厂家魔改版riscv-none-elf-gcc(买椟还珠属于是了)

#### VSCODE 
为了提供插件EIDE的宿主以及获得宇宙第一的编码体验(本人认为是的) 我们需要下载vscode并安装

#### EIDE

为了提供方便快捷开箱即用的环境体验 本项目选择vscode的插件eide进行开发 具体请移步
[eide](https://em-ide.com/zh-cn/docs/intro/)

#### OPENOCD

为了支持WCH-Link基于OPENOCD进行了定制化开发故需要安装WCH提供的OpenOCD(同样在MounRiver Studio的安装目录中可以找到)

#### 编译选项
os/o3
cpp20 + 


## 🍴目录

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


##### concept 外设的抽象基类

##### sys 系统
- [x] tasker 非抢占式任务驱动器

##### src 主要源代码
+ 宏定义头文件
+ example/testbench
+ main文件
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
    - [x] 任意通道多路信号采集
    - [ ] DMA绑定API
    - [x] 片上温度/参考电压采集
    - [ ] 虚拟ADCChannel类

- [x] DVP
    - [x] MT9V034摄像头驱动
    - [ ] OV2640摄像头驱动

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

