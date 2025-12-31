#pragma once

// 适用于球形关节摇杆的磁传感器


// MLX90333 是一款 Triaxis® 位置传感器 IC，可通过测量和处理磁通密度矢量的 3 个空间分量（即 Bx、By 和 Bz），感应在其周围移动的任何磁体。

// 水平分量（Bx 和 By）可通过集磁点（IMC） 感应，而垂直分量 (Bz) 可通过传统霍尔板进行感应。

// MLX90333 具有 3D 磁力计模式，在该模式下，磁通密度的 3D 信息可通过 SPI 进行报告。

// MLX90333 还具有非接触式位置传感器模式，适用于旋转位置传感器（通轴磁体）、线性行程位置传感器（磁体位移与器件表面平行）及 3D/“操纵杆”位置传感器 。

// 处理后的位置信息最终会以成比例的模拟量输出或 PWM（脉宽调制）信号的形式进行报告。在 3D/“操纵杆”模式下，器件具有 2 个独立的输出。3 引脚 SPI（串行接口）模式也可用于将位置信息传输到主机控制器。

// 输出传输特性完全可编程（例如，偏移、增益、钳位电平、线性度、热漂移、滤波、范围...），从而可通过行尾校准满足任何指定要求。迈来芯编程单元 PTC-04 专门通过连接器端子 (Vdd-Vss-Out) 与器件通信并对器件进行校准。

// MLX90333 的目标应用为汽车和工业系统中常见的大量非接触式位置传感器应用，尤其适用于线性位移和“操纵杆”感应应用。

#include <tuple>

#include "core/io/regs.hpp"
#include "core/math/real.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{


struct MLX90333_Prelude{

};

struct MLX90333_Regs:public MLX90333_Prelude{

};

class MLX90333_Transport final:public MLX90333_Prelude{

};

}