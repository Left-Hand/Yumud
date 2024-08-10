# Yumud

# 基于C++20的嵌入式开发框架



"用C进行嵌入式开发太膈应了 面向对象多好"

外设天然就是一个个类，用C去开发就是给自己找不快。

编写着结构体以及调用初始化函数的我如此想在目睹了arduino缺乏对底层的精细化控制与HAL尚未摆脱真正的硬件抽象的我返回了标准库。但转念一想利用CPP的高级语法，我们可以让嵌入式开发从这些令人作呕的外设配置变成了调度抽象语义的游戏，才能够使我们编写出更快更清爽更加与现代设计模式接轨的代码,不是吗？

很多人说C++不适合用在嵌入式上，更别说STL了，实际上这样的观点大错特错，实际上我们完全能在嵌入式开发上享用最新的特性且随处使用STL。只要我们提高优化选项，禁用dynamic_cast，异常等本就不健康的开发方式，CPP的代码产生的二进制文件完全可以部署在嵌入式上。在本框架上，通过宏对于一些全局量进行裁剪，完全可以在32Kb不到的芯片上编写大量业务代码而不超过芯片的极限。


无需复杂的结构体配置点亮一盏led

```c++

int main(){
    //...prework()

    //通过引用获取GPIO
    //端口使用工厂模式返回GPIO，确保GPIO不会被随意狗仔
    auto & led = portC[13];

    //将led设置为输出模式，并以高电平初始化
    led.outpp(HIGH);
    while(true){

        //通过重载GPIO的布尔运算符以写入与读取，使得GPIO能够直接被布尔量赋值
        led = !led;
        delay(200);
    }
}

```

又或是开启互补PWM快速验证PWM输出

```C++
int main(){
    //...prework()

    //将定时器①以36Khz初始化 使用字面量常量表达式来计算
    timer1.init(36_KHz);

    //使用工厂模式来获取定时器①的输出比较通道①
    auto & pwm = timer1.oc(1);
    pwm.init();

    {
        //获取互补PWM通道并初始化
        auto & pwmn = timer1.ocn(1);
        pwmn.init();

        //或者也可以直接
        timer1.ocn(1).init();
    }

    //将刹车寄存器初始化 默认为200ns
    timer1.initBdtr();

    //进入主循环
    while(true){
        // t(时间) 是一个全局变量 将在systick中被自动更改

        // 重载了PWM被数值赋值的函数 使得对它的赋值能直接改变占空比
        pwm = 0.5 + 0.5 * sin(t);
    }

    //或者也可以直接这样做

    //以0:0的中断优先级将定时器①的更新中断使能
    //使用枚举类以确保安全 
    timer1.enableIt(TimerIT::Update, {0,0});

    //使用lambda函数建立对应中断请求的回调函数
    timer1.bindCb(TimerIT::Update, [&](){pwm = 0.5 + 0.5 * sin(t);});
}

```

再或者通过CAN指令精细控制云台的角度

```C++

int main(){
    //...prework()

    //将CAN1以1Mbps的波特率初始化
    can1.init(1_MHz);

    //将串口一以921600的波特率初始化 默任TX和RX都使用DMA进行数据收发
    uart1.init(921600);

    //OutputStream是串口的基类 可以实现数据的打印
    OutputStream & logger = uart1;

    //建立舵机与步进电机变量
    SG90   servo   {timer1.ch(1)};
    DRV8825 stepper {timer1.ch(2);}

    //通过引用的方式建立is-a的关系 其中多继承确保只要是两个继承自角度可控类的对象均可以传入云台中
    Gimbal  gimbal  {servo, stepper};
    gimbal.init();

    real_t angle;
    while(true){
        if(can1.available()){
            //从CAN的接收FIFO中读取报文
            const auto & msg = can1.read();
            
            //使用模板以确保报文可以转换为任意类型的数据
            msg.load(angle);

            //直接就能将数据打印到串口
            logger.println("angle is:", angle);
        }

        //内部有多种向量数据类型(多维向量与四元数等)，为机器人学奠定基础
        gimbal.moveTo(Vector2(1, 0).rotated(angle));
    }
}

```

当我们用面向对象的方式构建起代码时 一切是如此自然且清晰。
我们脱离了厂家的SDK，在业务代码中就不应该和具体平台绑定。


## ⚠️声明

目前支持的芯片只有ch32(性价比之与皮实王yyds)，正在考虑添加其他厂商的芯片

参考了HAL和arduino 但使用了抽象开销成本更小与更安全的方式编写

使用cpp20编写 请使用gcc12+进行编译
使用vscode下的eide搭建开发环境

厂方sdk库已改动
请使用用本仓库提供的sdk 位于sys/sdk下

大量使用iqmath在无浮点单元的情况下进行高速数学运算 请选择16位IQ数（调整宏定义GLOBAL_Q）否则可能会出现意想不到的问题

## 🌱参与贡献

算是用来练习git的项目, 第一次使用git不太熟悉 孩子自个写着玩的
本框架全由作者一人编写，可能参考了其他代码。
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
请使用Os/O3/Ofast进行编译，以确保代码的尺寸和运行速度被尽可能优化


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

- [ ] 线性时不变系统
- [x] 查找表
- [ ] 振荡器
- [x] pll 锁相环
- [ ] sogi

##### types 各种类型

- [x] basis 三维旋转矩阵(godot)
- [x] buffer 缓冲结构
    - [x] ringbuffer 环形缓冲

- [x] color RGBA颜色表述
- [x] complex 复数
- [x] image 图像类
    - [x] font 字体类
        - [x] 英文字体
        - [x] 中文字体
    - [ ] cursor 光标类
    - [x] packedImage 压缩二值化图片
    - [x] painter 绘图算法驱动

- [x] IQ(IQMATH)
    - [x] 支持四则运算 大小比较 类型转换
    - [x] 支持超越函数
    - [x] 支持std::超越函数
    - [x] 支持concept
    - [ ] 使用模板重构

- [ ] matrix 矩阵类
- [ ] polar 极坐标类
- [ ] float16 16位浮点数
- [x] quat 四元数类(godot)
- [x] range 区间类
- [x] rect2 矩形区域类
- [x] string 字符串类(arduino)
- [x] vector2 二维向量类(godot)
- [x] vector3 三维向量类(godot)
- [x] rgb 各类颜色结构体
- [x] uint24_t 24位数据

##### nvcv2 图像处理框架
- [x] 颜色空间转换
    - [x] 布尔运算 

- [x] 形态学
    - [x] 自适应阈值化
    - [x] CANNY
    - [x] 卷积核
    - [x] 洪水填充算法

    - [x] 边线提取
    - [x] 开闭运算
    - [x] 仿射变换
    - [x] 模板匹配
    - [x] Mnist深度学习识别
    - [x] apriltag识别
    - [ ] 霍夫变换



##### drivers 设备驱动

- [x] 执行器
    - [x] 线圈
    - [x] MOS驱动
    - [x] 云台
    - [x] 舵机

- [x] ADC
    - [x] HX711
    - [x] INA226
    - [x] SGM58031

- [x] DAC
    - [x] TM8211

- [x] DDS
    - [x] AD5933
    - [x] AD7607
    - [x] AD9833
    - [x] AD9854
    - [x] AD9910
    - [x] AD9959
    - [x] SI5351

- [x] 摄像头
    - [x] MT9V034

- [x] 屏幕
    - [x] SSD1306(OLED)
    - [x] ST7789(tft)

- [x] 编码器
    - [x] 磁编码器
        - [x] AS5600
        - [x] MA730
        - [x] MT6701
        - [x] MT6816

    - [x] AB编码器
    - [x] 里程计

- [x] HID设备
    - [x] PS2手柄

- [x] IMU
    - [x] 六轴
        - [ ] ADXL345
        - [x] MPU6050
    - [x] 地磁
        - [x] HMC5883L
        - [x] QMC5883L

- [x] 环境光传感器
    - [x] BH1750
    - [x] TCS34725

- [x] 气压计
    - [x] BMP280

- [x] 存储器
    - [x] EEPROM(AT24)
    - [ ] FLASH(W25)
    - [ ] SD卡

- [x] 调制器
    - [x] DSHOT
    - [x] NEC

- [x] 测距器
    - [x] VL53L0X

- [x] 一般IO
    - [x] LED
    - [x] 按键

- [x] 虚拟IO
    - [x] AS9523
    - [ ] HC138
    - [ ] HC165
    - [x] HC595
    - [x] PCA9695
    - [x] PCF8574
    - [x] TTP229

- [x] 无线
    - [x] CH9141
    - [x] HC12
    - [x] LT8920


##### concept 外设的抽象基类

##### sys 系统
- [x] clock 时钟
- [x] tasker 非抢占式任务驱动器
- [ ] IOStream(输入输出流)
    - [ ] `printf` `cout`(内建输出流操作) 
    - [x] `println` `print` `prints`  `<<` (基本输出流操作)
        - [x] 直接将类型格式化输出
        - [x] 将各输出类型重载输出
        - [x] 重载了容器的输出
    - [ ] `format` (字符串格式化) 

##### src 主要源代码

+ 宏定义头文件
+ example/testbench
+ main文件

##### hal 物理抽象层

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
    - [ ] 基于模板的自定义过滤器

- [ ] Adc
    - [x] 任意通道单路信号采集
    - [x] 任意通道多路信号采集
    - [ ] DMA绑定API
    - [x] 片上温度/参考电压采集
    - [x] 虚拟ADCChannel类

- [x] DVP
    - [x] MT9V034摄像头驱动

- [x] CRC
- [x] OPA
- [x] NVIC
- [x] EXTI

- [x] FLASH
    - [x] 快速FLASH读写

- [x] TIM(定时器相关代码)
    - [x] 中断与回调函数绑定
    - [x] 编码器模式
    - [ ] PWMModule PWM输出集线器

    - [x] PWMChannel PWM输出概念
        - [x] AW9523Pwm 基于AW9523的pwm
        - [x] GPIOPwm 使用GPIO配合定时触发模拟pwm
        - [x] TimerOC(定时器输出)
            - [x] TimerOC

    - [ ] CaptureChannel 输入捕获概念 
        - [x] ExtiCapture 基于EXTI的输入捕获
        - [ ] TimerIC(定时器输入捕获)

- [ ] TCP/UDP
- [ ] USB
 - [x] USBFS USBFS虚拟串口驱动
- [ ] BLE


##### robots 机器人学相关代码

- [x] `stepper` 步进电机FOC算法
    - [x] 电流环
    - [x] 力矩环
    - [x] 速度环
        - [x] 低速速度环(0rpm~240rpm)
        - [x] 中速速度环(300rpm~2400rpm)
        - [x] 高速速度环(2400rpm~8000rpm)
    - [x] 位置环
        - [x] 高精度位置环
        - [x] 常规位置环
        - [x] 梯形加减速速度环

    - [x] 自校准算法
    - [ ] 自检查算法

    - [x] 串口驱动
    - [x] CAN驱动
    - [x] 数据存档

    - [ ] 高级插值规划
        - [ ]直线规划
        - [ ]正弦规划
        - [ ]S形规划
- [ ] `bldc` 无刷电机FOC算法

- [x] `buck.hpp` BUCK驱动
- [x] `pmdc.hpp` 直流有刷电机驱动算法
- [ ] `spread_cycle.hpp` 对SpeadCycle斩波驱动的低劣模仿

