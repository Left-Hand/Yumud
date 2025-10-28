

// HP203B 是高分辨率（0.1meter）压力传感器，带有 I²C 接口，
// 包括一个硅压阻压力元件和一个高分辨率 24 位 △∑ ADC。
// HP203B 提供高精度 24 位压力和温度数字输出，客户可以根据应用需要转换速度和高度，
// 所有内置计算采用了高速 4ＭＨz 的浮点运算，计算误差小，数据补偿是内部集成，通讯连接非常简单，
// 高度及温度上下限比较的可编程事件及中断输出控制。HP203B 传感器是采用不锈钢盖子表面封装和符合 RoHS 标准，
// 尺寸非常小，仅为 3.6x3.8mm,厚度为 1.2mm。

// 可以直接读取压力值，高度值，温度值

// https://wiki.lckfb.com/zh-hans/lspi/module/sensor/hp203b-barometric-height-sensor.html

#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct HP203B_Prelude{
static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x77); // HP203B I2C address

enum class State{
    Calibration,
    Sleep,
    Actions,
    Por,
};

enum class ChannelSelection:uint8_t{
    PressureAndTemperature = 0b00,
    Temperature = 0b10,
};

enum class Osr:uint8_t{
    _4096 = 0b000,
    _2048 = 0b001,
    _1024 = 0b010,
    _512 = 0b011,
    _256 = 0b100,
    _128 = 0b101,
    Default = _256
};

enum class Command:uint8_t{
    SoftReset  =0x06,
    ReadPressureAndTemperature  = 0x10,
    ReadAltitudeAndTemperature = 0x11,
    ReadPressure = 0x30,
    ReadAltitude = 0x31,
    ReadTemperature = 0x32,
    CalibrateAnalog = 0x28
};

struct AdcConvertCommand{
    ChannelSelection channel_selection:2;
    Osr osr:3;
    uint8_t __fixed__ = 0b010;
};


// 温度数据由 20 位 2 的补码格式组成，单位为摄氏度。温度的值由 24 位 OUT_T_MSB,OUT_T_CSB
// OUT_T_LSB 存储。最高 4 位的数据是无用，而最低有效 20 位代表温度的值。用户应当把这 20 位以 2 的补码的
// 二进制值转换成一个整数，然后整数除以 100 获得最终结果。

// 气压数据由 20 位 2 的补码格式组成，单位为帕。气压的值由 24 位 OUT_T_MSB,OUT_T_CSB OUT_T_LSB
// 存储。最高 4 位的数据是无用，而最低有效 20 位代表气压的值。用户应当把这 20 位以 2 的补码的二进制值转换
// 成一个整数，然后整数除以 100 获得最终结果。

// 高度数据由 20 位 2 的补码格式组成，单位为米。高度的值由 24 位 OUT_T_MSB,OUT_T_CSB OUT_T_LSB
// 存储。最高 4 位的数据是无用，而最低有效 20 位代表高度的值。用户应当把这 20 位以 2 的补码的二进制值转换
// 成一个整数，然后整数除以 100 获得最终结果。

struct [[nodiscard]] Data20{
    static constexpr Data20 from_raw_bytes(
        const uint8_t msb, const uint8_t csb, const uint8_t lsb
    ){
        const auto raw = (msb << 16) | (csb << 8) | lsb;
        return Data20{
            raw
        };
    }
    [[nodiscard]] q16 count() const {
        return q24(raw_val) / 100;
    }
private:
    constexpr Data20(uint32_t raw_val):raw_val(raw_val){}
    uint32_t raw_val;
};



};
}