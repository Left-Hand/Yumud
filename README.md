# Yumud

# 基于C++20的超轻量级嵌入式开发框架 快速实现个位级成本MCU的降本增效


随着科技与技术的不断进步，价格个位数的MCU和崭新的C++20冲击着现有的设施。在硬件上，价格超1元便已经是32位MCU的天下，没有任何必要死守8位的MCU。在软件上，用C进行开发已经落伍了，它几乎没有任何安全或是抽象可言，现在是面向对象与函数式编程的时代。硬件已经日新月异，可嵌入式的主流话语权却还在C89上，这不禁让人啼笑皆非———传统的面向过程与全局变量乱飞的思维显然已经不适用于编写出安全高效可复用性强的代码。但我们却不得不承认嵌入式环境确实难寻成熟可靠的嵌入式C++框架（请忘了Arduino,它是蒙了一层C++皮的C/Java）。基于这样的想法，我从大二(2023年末)开始打造一个完备的嵌入式框架，并借助它获得了不少国家级别的奖项，现在我将它开源于此。在今天，借助C++20的强大特性，我们可以轻松编写安全，高效复用性强的代码。让嵌入式开发从繁琐枯燥的外设配置变成了调度抽象语义的游戏，才能够使我们编写出更快更清爽更加与现代设计模式接轨的代码,不是吗。

很多人说C++不适合用在嵌入式上，更别说STL了。实际上这样的观点大错特错，能得出这样的结论也受限于PC上编写代码的惯性，诸如iostream，异常等并非零成本开销的设施会在一开始就占用数十上百KB的内存空间，因而他们一旦不慎用到了这些设施就很容易得出这样荒谬且错误的结论。我们需要做的，仅仅只是择取其中零成本抽象的C++子集。在长期与C++的接触中，我也总结了更适合嵌入式开发的C++子集，这是在编写本库的过程中得到大量实践验证的：

1. 尽可能提高优化选项，将默认的O0提升到最起码O2的标准，同时开启lto优化：默认的O0优化选项会占用大量内存空间，且为了保持函数的调用关系与代码一致，导致很多调用和计算无法内敛或乱序执行，同时 lto优化能在最后在全局合并减少重复的代码，使用两者能极大减少二进制文件体积。也正因此，依赖使用Og进行程序追踪从来不是本库鼓励的开发方式，因为它让压缩数倍二进制空间的优化难以正常开展，正确的开发流应当是严谨推导自己的代码，或是利用本库提供的错误调试工具快速定位问题。

2. 禁用异常，禁用dynamic_cast：异常和rtti等特性是C++中被诟病已久的非零成本抽象特性，更不要说它上来就占用数十k二进制文件而不适合嵌入式开发的原罪。在本库中，异常被禁用，本库对异常安全不做任何处理。更合理的做法可以参考借鉴rust的Result，Optional等工具，通过在结果中附加执行情况来为调用方提供错误信息。本框架中实现了rust的Result类，以帮助开发者进行更轻量现代化的错误处理。

3. 手动重写newlib：很多嵌入式开发者在开发之初就早早地重写write函数，开启u_printf_float链接选项，殊不知这也是一项开销相当大的举动，newlib中提供的c风格设施一方面没有对MCU提供足够的轻量化设计，同时也很难被编译器内联与编译期求值。本框架自主实现的一系列字符串转换函数以及与之对应的流输出类，能够在可读性更好的同时大幅度减少printf等设施的占用。

4. 多使用模板和内联：很多C开发者总是抱着课本上模板会增大二进制文件体积的教条去批判C++,而事实上模板很难造成可观的体积膨胀，恰恰相反，编译器在得到模板提供的类型信息后能够减少很多无用的推迟到运行期判决的代码，使得代码能够更好的内联压缩。例如SDK中通常提供传递配置结构体去配置外设的方式，或是获取中断标志位的函数，实际上这些函数内部做了大量不必要的判断，反而加大了二进制文件体积和运行期的负担，通过将部分函数用模板改写，部分代码设置能内联到直接对寄存器进行位修改的地步，既快又轻量。在未来的计划中，为了减少SDK调用，同时减少适配不同产商芯片的移植陈本，将逐渐通过结构体位域，编译时求值与模板元函数来实现对外设的配置与映射。

5. 避免浮点改用定点：浮点对于编写桌面应用或是使用带有FPU单元的开发者来说触手可得，但大量的芯片却只提供了整数相关的硬件计算单元，而支持高性能的的芯片并不是作者的兴趣所在，在极限低成本的平台发挥超强的效能才是成就所在。故本库提供了内置的定点数支持，参考了德州仪器开源的iqmath库并对它做了改进，使得定点数被改写为一个重载了四则运算的模板类，我们能像用浮点数一样没有语法噪声地使用定点数并获得超高速的超越函数计算，其性能表现甚至在具有浮点单元的芯片上同样优于浮点计算。

6. STL可用：STL当然可用，但STL可用不代表我们可以盲目使用，请尽可能避免动态分配的容器。首先，零成本开销的库文件是完全可以使用的，诸如<type_traits><ranges><algorithm>等库，他们在提供抽象的同时几乎不会引入额外的占用。其次<functional><quene><vector><unordered_map>等库同样可用，但在使用容器前思考背后的动态分配是否有必要，否则尽量使用<array><span>等在栈上进行内存分配或是只引用不持有的容器。最后<iostream><exception>等库应当避免使用，他们会引入大量我们不需要的函数，占用大量体积。

成本从来都是我最看重的一点，在资源丰富的硬件平台上做低效率的crud从来不是一个成熟的开发者应该引以为傲的资本，本框架将始终保持对人民币2元以下的单片机的支持并将其作为核心的优化动力。在本框架上，完全可以在超低成本的芯片上编写大量业务代码而不超过芯片的极限，实现低性能MCU的四两拨千斤，快速实现降本增效。例如，通过本框架可以在短时间内部署大量低成本节点进行组网并实现传感，控制，计算等组件的模块化。真正使得嵌入式变成一门优雅的技术。

下面是一些案例的效果：
| 作用 | 描述 |开销 | 性能 |
|---------|---------|---------|----|
| 步进电机FOC驱动器 | 基于CH32V203(对标F103)芯片实现了完整的Can节点及FOC控制 | 36KB / 64KB | 电流环/速度环/位置环均能以40khz+运行|
| 机器人视觉主控板 | 基于CH32V307(对标F407)芯片实现了完整的Can主机以及视觉/AI/行为树的运行 | 93KB / 192KB | 能以60fps运行视觉算法，自动完成对目标的搜索 |
| 九轴传感节点 | 基于CH32V203(对标F103)芯片实现了完整的Can节点及串口命令行 | 25KB / 32KB | 微秒级解算 |

本框架的突出特点：

1. 实现了完全的跨平台的抽象
2. 提供了debug，日志相关的轻量强力工具，与stl保持良好兼容
3. 使用了模板化的定点数类，编写定点代码不再是问题
4. 提供了大量外设的抽象层，同时尽可能保证他们是易用且中断安全的(添加相关仲裁机制)
5. 提供了大量嵌入式常用设备的支持，开箱即用的一众SPI和I2C设备
6. 提供了大量的机器人学，图像处理相关的中间层
7. 在支持传统的虚函数,CRTP多态的同时也支持类似Rust的Trait特征表述以实现鸭子类型
8. （计划）使用模板元进行硬件平台的快速配置
9. （计划）提供异步去中心分布通信框架

固然承认，时至今日Arduino和HAL的设计占有相当大的话语权，但我还是与其对比，以更好地呈现本框架的优势。

本库与主流开发框架的比较：

vs arduino：

1. 提供了对外设每一分细节的精细控制
2. 提供了统一的设备管理层 不需要再用宏定义区分各个平台
2. 提供了统一的抽象层 所有同类外设均有对应的抽象类
3. 提供了对常用总线的原生支持，而不是全局单例
4. C++化程度更高，减少Java风格C++的枯燥体验
5. 大量使用泛型，以实现代码的复用与泛化
6. 大量使用stl(参考odrive)

vs HAL：

1. 做到了真正的硬件抽象，对不同厂家的芯片都提供的近乎相同的api，不再被厂家裹挟
2. 应用层与库函数完全隔离，在应用层完全不需要和厂方SDK进行交互
3. 没有丑陋的HAL前缀表示函数的命名空间
4. 用多态取或lambda取代函数指针的使用，减少了心智负担
5. 所有外设通过用户自定义宏或是模板元裁剪而不是工程文件管理 提供了更高的自由度
6. 将自动化生成代码与用户业务代码隔离



下面是一些简单的演示(不一定与具体代码同步，具体请参考src/app或src/testbench下的文件)

无需复杂的结构体配置点亮一盏led
   
```c++

int main(){
    //...preinit()

    //通过引用获取GPIO
    //端口使用工厂模式返回GPIO，确保GPIO不会被随意构造
    auto led = portC[13];

    //将led设置为输出模式，并以高电平初始化
    led.outpp(HIGH);

    while(true){

        //通过重载GPIO的布尔运算符以写入与读取，使得GPIO能够直接被布尔量赋值
        led = !led;
        delay(200);
    }
}

```

又或是开启互补PWM快速验证SPWM输出

```C++
int main(){
    //...preinit()

    //将定时器①以36Khz初始化 使用字面量常量表达式来计算
    timer1.init(36_KHz);

    //使用工厂模式来获取定时器①的输出比较通道①
    auto pwm = timer1.oc(1);
    pwm.init();

    {
        //获取互补PWM通道并初始化
        auto pwmn = timer1.ocn(1);
        pwmn.init();

        //或者也可以直接
        // timer1.ocn(1).init();
    }

    //将刹车寄存器初始化 默认为200ns
    timer1.initBdtr();

    //进入主循环
    while(true){
        // time (时间) 是一个获取高精度秒数(微秒级)的函数 将在systick中被自动更改
        const auto t = time();
        // 重载了PWM被数值赋值的函数 使得对它的赋值能直接改变占空比
        pwm = 0.5 + 0.5 * sin(t);
    }

    //或者也可以直接这样做

    //以0:0的中断优先级将定时器①的更新中断使能
    //使用枚举类以确保安全 
    // timer1.enableIt(TimerIT::Update, {0,0});

    //使用lambda函数建立对应中断请求的回调函数
    // timer1.bindCb(TimerIT::Update, [&](){pwm = 0.5 + 0.5 * sin(t);});
}

```

再或者通过CAN指令精细控制云台的角度

```C++

int main(){
    //...preinit()

    //将CAN1以1Mbps的波特率初始化
    can1.init(1_MHz);

    //将串口一以921600的波特率初始化 默认TX和RX都使用DMA进行数据收发
    uart1.init(921600);

    //OutputStream是输出流类 可以实现数据的格式化输出
    //这里将串口的字符打印特征传递给logger 让logger编码的字符从uart1输出
    OutputStream logger = {&uart1};

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
            //使用python风格的print输出而不需要formatter
            logger.println("angle is:", angle);
        }

        //内部有多种向量数据类型(多维向量与四元数等)，为机器人学奠定基础
        gimbal.moveTo(Vector2(1, 0).rotated(angle));
    }
}

```


## ⚠️声明

目前受到完整支持的芯片只有CH32V2/CH32V3系列，其他平台的MCU的支持计划在项目API稳定后加入支持，加入非SXX32风格的芯片乃至对于Linux的支持

使用C++20编写，请使用gcc12及以上的版本进行编译，否则可能出现编译错误
使用Vscode下的eide搭建开发环境，计划将于项目成熟后逐步迁移到Cmake中

厂方sdk库已改动
请使用用本仓库提供的sdk 位于sdk目录下下


## 🌱参与贡献

算是本人用来练习git的项目, 第一次使用git不太熟悉，欢迎大佬提出宝贵意见。
本框架全由作者一人贡献，参考或引入了部分开源代码。
由于目前项目架构尚不稳定 不要轻易fork！！！！！

## 🔨开发环境

#### GCC
由于本项目使用GCC进行开发（对各平台实现最大化开源），且目前暂无计划打算增加对Clang系列编译器的支持（ARMCC/ARMCLANG不支持）
且本项目使用到了完整的C++20的特性进行开发，故GCC的版本不低于12

#### CH32系列单片机开发：

同时由于CH32系列单片机使用的RiscV V4系列内部为了支持快速中断，为GCC添加了客制化的中断关键字，故应使用WCH定制的GCC12（需要安装[MounRiver Studio](http://www.mounriver.com/)以获取我们所需要的厂家客制化的riscv-none-elf-gcc）

#### VSCODE 
为了提供插件EIDE的宿主以及获得更好的编码体验 需要下载vscode并安装

#### EIDE

为了提供方便快捷开箱即用的环境体验 本项目选择vscode的插件eide进行开发 具体请移步
[eide](https://em-ide.com/zh-cn/docs/intro/)

#### OPENOCD

为了支持WCH-Link基于OPENOCD进行了定制化开发故需要安装WCH提供的OpenOCD(同样在MounRiver Studio的安装目录中可以找到)

#### 编译选项
请使用Os/O3/Ofast进行编译，以确保代码的尺寸和运行速度被尽可能优化, 否则FLASH占用不足以支持使用


## 🍴目录

##### docu 文档

##### algo 数据结构与算法
- [x] random 伪随机数发生器
- [x] hash 哈希函数
- [x] interpolation 插值
    - [x] 线性插值
    - [x] 三次插值
    - [x] 四次插值
    - [x] 多项式插值
    - [x] 弧形插值

- [ ] encrypt 加密解密
    - [ ] SHA256编码解码
    - [ ] AES编码解码
    - [ ] BASE64编码解码
    - [x] CRC编码解码

- [ ] astar A*寻路算法

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
    - [ ] 拓展卡尔曼滤波器
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
- [x] aabb 三维包围盒(godot)
- [x] Arc2D 二维弧形元素
- [x] basis 三维旋转矩阵(godot)
- [ ] Bezier2D 二维贝塞尔曲线


- [x] buffer 缓冲结构
    - [x] fifo 环形缓冲
    - [x] lifo 栈

- [x] Color RGBA颜色表述(godot)
- [x] Complex 复数
- [ ] Curve2D 二维曲线类
- [x] Image 图像类
    - [x] Font 字体类
        - [x] 英文字体
        - [x] 中文字体
    - [x] PackedImage 压缩二值化图片
    - [x] Painter 绘图算法驱动

- [ ] Line2D 二维直线类
- [ ] matrix 矩阵
    - [x] matrix_dynamic 动态矩阵
    - [ ] matrix_static 静态矩阵
        - [x] 加减乘法
        - [x] 求转置/逆
        - [ ] 求行列式 
        - [ ] 方法库

- [x] plane 三维平面类(godot)
- [ ] Point2D 二维点类
- [ ] polar 极坐标类
- [x] quat 四元数类(godot)
- [x] transform3d 三维位姿矩阵(godot)


- [ ] dh 连接件DH参数

- [x] range 区间类
- [x] Ray2D 二维射线类
- [x] rect2 矩形区域类

- [x] Segment2D 二维线段类
- [x] Tranform2D 二维变换矩阵类
- [x] Tranform3D 三维变换矩阵类

- [x] vector2 二维向量类(godot)
- [x] vector3 三维向量类(godot)


- [x] rgb 各类颜色结构体

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
        - [x] 单端线圈
        - [x] 双端线圈
        - [x] 三端线圈

    - [x] Bridge 栅极驱动或电机驱动器
        - [x] AT8222
        - [ ] DRV8301
        - [ ] DRV8313
        - [x] EG2103
        - [x] EG2104
        - [ ] EG2133
        - [x] MP1907
        - [x] MP6540
        - [x] TB67H450

    - [x] 变换器
        - [x] DRV2605L
        - [x] SC8721
        - [x] SC8815
        - [x] MP5980
        - [x] MP2980
        - [ ] SC6570

    - [x] SVPWM
        - [x] SVPWM
        - [x] SVPWM2
        - [x] SVPWM3
    
    - [ ] 云台
    - [x] 舵机

- [x] ADC
    - [x] ADS112C04
    - [x] ADS1115
    - [x] HX711
    - [x] INA226
    - [x] INA3221
    - [x] SGM58031

- [x] DAC
    - [x] TM8211

- [ ] 音频
    - [x] JQ8900

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
    - [ ] OV2640

- [x] 屏幕
    - [x] SSD1306(OLED)
    - [ ] STN7567
    - [x] ST7789(tft)

- [x] 编码器
    - [x] 磁编码器
        - [x] AS5047
        - [x] AS5600
        - [x] MA730
        - [x] MT6701
        - [x] MT6816
        - [x] MT6835

    - [x] AB编码器
    - [ ] PWM编码器
    - [ ] 模拟编码器
    - [x] 里程计

- [x] HID设备
    - [x] PS2手柄
    - [ ] FT6336

- [x] IMU
    - [ ] 三轴
        - [ ] LISDW12 
    - [x] 六轴
        - [x] ADXL345  
        - [x] BMI088  
        - [x] BMI160
        - [x] BMI270
        - [x] ICM42605
        - [x] ICM42688
        - [x] MPU6050
    - [x] 地磁
        - [x] AK8975
        - [x] BMM150
        - [x] HMC5883L
        - [x] IST8310
        - [x] MMC5603
        - [x] QMC5883L

- [x] 环境光传感器
    - [x] BH1750
    - [x] TCS34725

- [x] 气压计
    - [x] BMP280
    - [ ] BMP180

- [x] 存储器
    - [x] EEPROM(AT24)
    - [ ] FLASH(W25)
    - [ ] SD卡
 
- [x] 调制器
    - [x] DSHOT
    - [x] NEC

- [x] 空间感知
    - [ ] LDS14
    - [x] PAJ7620
    - [x] VL53L0X
    - [x] PMW3901
    - [ ] SR04 
    - [ ] VL6180X

- [ ] 识别器
    - [ ] GM80X
    - [x] U13T

- [x] 一般IO
    - [x] LED
        - [x] 模拟LED
    - [x] 按键
        - [ ] 按键矩阵


- [x] 虚拟IO
    - [x] AS9523
    - [ ] HC138
    - [ ] HC165
    - [x] HC595
    - [x] MCP23016
    - [x] NCA9555
    - [x] PCA9695
    - [x] PCF8574
    - [x] PCF8575
    - [x] TTP229

- [x] 无线
    - [x] CH9141
    - [x] HC12
    - [x] LT8920
    - [x] LT8960
    - [ ] LT8960
    - [ ] Si24R1
    - [ ] XL2400

##### hwspec 硬件规格

- [ ] ch32
    - [ ] ch32v003 寄存器布局
    - [ ] ch32v203 寄存器布局
    - [ ] ch32l103 寄存器布局
    - [ ] ch32x035 寄存器布局
- [ ] py32
- [ ] stm32
- [ ] gd32
- [ ] chip 用于萃取不同外设的模板元工具

##### concept 外设的抽象基类

##### sys 系统
- [ ] arch 架构
    - [ ] arm
        - [ ] cm3
        - [ ] cm4
    - [ ] riscv
        - [ ] qingkeV3 
        - [ ] qingkeV4 

- [x] constants 编译期常量
    - [x] concepts c++20概念约束拓展 
    - [x] enums 内置枚举类型
    - [x] uints 单位转换

- [x] clock 时钟
    - [x] 毫秒 微秒 纳秒级时间戳
    - [x] Systick回调函数
    - [x] 生成精确启动秒数
    - [ ] chrono支持


- [x] debug
    - [x] assert
    - [x] panic
    - [x] 错误输出流
    - [ ] 错误颜色输出
    - [ ] 错误日志输出
- [ ] file 文件系统
- [x] io
    - [x] 寄存器
- [ ] os 操作系统
    - [x] tasker 非抢占式任务驱动器
- [ ] 错误处理
    - [x] Result(rust)
- [x] polymorphism 多态
    - [x] proxy3
    - [ ] metaclass
    - [ ] traits

- [x] Stream(输入输出流)
    - [x] `printf` `cout`(内建输出流操作) 
    - [x] `println` `print` `prints`  `<<` (基本输出流操作)
        - [x] 直接将类型格式化输出
        - [x] 将各输出类型重载输出
        - [x] 重载了容器的输出
        - [x] 添加对std::hex, std::setpos, std::setprecision等函数的支持
    - [x] StringStream(静态打印)
    - [x] snprintf

- [x] String 字符串类(arduino)
    - [x] string (字符串主体)
    - [x] string_view (字符串视图)
    - [x] string_utils (字符串工具)
        - [x] 超轻量级xtoa(数字转换到字符串)
        - [x] 超轻量级atox(字符串转换到数字)
- [ ] utils
    - [x] setget 属性访问
    - [ ] any 类型擦除
    - [x] rustlike
        - [x] result rust风格错误处理
        - [x] optional rust风格结果处理
    - [x] hashfunc 哈希函数
    - [ ] Variant 内建动态变量
- [x] math 数学类型库
    - [x] IQ(IQMATH)
        - [x] 支持四则运算 大小比较 类型转换
        - [x] 支持超越函数
            - [x] sin cos tan atan atan2 acos exp log
        - [x] 支持std::超越函数
        - [x] 支持concept
        - [x] 使用模板重构
        - [x] 可选的浮点数杜绝机制
        - [x] 支持全平台的iqmath


    - [ ] float 各类浮点数
        - [x] fp32 
        - [x] bf16
        - [ ] fp16
        - [ ] fp8
            - [ ] fp8e4m3
            - [ ] fp8e5m2

    - [ ] int 整数
        - [ ] int(rust)

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

- [x] UART(已验证1500000波特率的长时间压力测试)
    - [x] 对基本输入输出流的支持
    - [x] 环形缓冲区支持
    - [x] DMA/中断支持
    - [x] LinBus
    - [ ] 智能卡

- [x] SPI(已验证144Mhz的长时间压力测试)
    - [x] 一般数据收发
    - [ ] DMA数据收发
    - [x] 虚拟片选集线器
    - [x] 软件SPI

- [x] I2C(以验证5.4Mhz的软件I2C)
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

- [x] CAN(控制局域网总线)
    - [x] 信箱及FIFO驱动
    - [x] 输入输出环形缓冲区
    - [x] 标准ID与拓展ID下Msg发送接收
    - [x] 标准ID下的过滤器
    - [x] 基于模板的自定义过滤器
    - [ ] CANFD

- [ ] Adc
    - [x] 任意通道单路信号采集
    - [x] 任意通道多路信号采集
    - [ ] DMA绑定API
    - [x] 片上温度/参考电压采集
    - [x] 虚拟ADCChannel类

- [x] DVP
    - [x] DVP总线驱动

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

    - [x] PwmIntf PWM输出概念
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


##### robots 机器人与电力学相关代码

- [ ] FOC
    - [x] 步进电机FOC算法
        - [x] 电流环
        - [x] 力矩环
        - [x] 速度环
            - [x] 低速速度环(0rpm~240rpm)
            - [x] 中速速度环(300rpm~2400rpm)
            - [x] 高速速度环(2400rpm~8000rpm)
        - [x] 位置环
            - [x] 高精度位置环
            - [x] 常规位置环

        - [x] 自校准算法
        - [ ] 自检查算法

        - [x] 基于串口的RPC
        - [x] 基于CAN的RPC
        - [x] 数据存档

        - [x] 高级插值规划
            - [x] 直线规划
            - [x] 正弦规划
            - [x] S形规划

    - [ ] 无刷电机FOC算法

- [ ] 正逆解
    - [x] Scara正逆解
    - [x] 轮腿正逆解
    - [x] 交叉臂正逆解
    - [ ] 六轴正逆解

- [ ] DJI RoboMaster相关驱动
    - [x] M2006
    - [x] M3508
    - [x] M6020
    - [x] DR16

- [ ] 变换器
    - [ ] `halfbridge` 通用半桥驱动 
    - [x] `buck.hpp` BUCK驱动
    - [x] `pmdc.hpp` 直流有刷电机驱动算法
    - [ ] `spread_cycle.hpp` 对SpeadCycle斩波驱动的低劣模仿

- [ ] 机械
    - [x] 平动滑台描述
    - [ ] 转动关节描述

- [ ] rpc rpc框架
    - [x] 串口 RPC
    - [x] CAN RPC

- [ ] AI
    - [x] 神经元
    - [ ] 网络
    - [ ] 复合网络
    - [ ] 训练器
    - [ ] 损失函数

- [ ] 光栅器
    - [ ] 任务队列
    - [ ] 光栅化器

- [ ] 策略
    - [x] 动作池
        - [x] 运动队列
        - [x] 单个运动
        - [x] 组合运动

    - [ ] 行为树
        - [ ] 复合节点
            - [x] 选择节点
            - [x] 顺序节点
            - [x] 全通节点

        - [x] 装饰器

    - [ ] FSM

