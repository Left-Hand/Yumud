//这个驱动还未完成
//这个驱动还未测试

//VL53L5X 是意法半导体的一款具有宽视野的飞行时间 (ToF) 8x8多区测距传感器

// VL53L5CX是意法半导体FlightSense产品系列中最先进的飞行时间 (ToF) 多区测距传感器。
// 该产品采用微型可回流焊封装，集成了SPAD阵列、物理红外滤光片和衍射光学元件 (DOE)，
// 确保在各种环境照明条件下，搭配不同类型的盖片材料，均能够实现卓越的测距性能。

// 通过在垂直腔面发射激光器 (VCSEL) 上方使用DOE，将一个方形FoV投影到场景上。反射光随后由接收器透镜聚焦到SPAD阵列上。

// 不同于传统的IR传感器，VL53L5CX利用意法半导体最新一代ToF技术，能够测量目标的绝对距离，
// 而不受目标颜色或反射率的影响。该产品可提供高达400 cm的精确测距，并能以极快的速度 
// (60 Hz) 运行，是目前市场上速度最快的多区微型ToF传感器之一。

// 多区测距最多可覆盖8x8个区域，对角线视野可达65°（可通过软件调整）

// 得益于意法半导体的专利型直方图算法，VL53L5CX能够检测并区分FoV内的多个目标。
// 该算法还能有效抵御60 cm以上的盖片串扰。

#pragma once


#pragma once

#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class VL53L5CX final{ 
    
};
}