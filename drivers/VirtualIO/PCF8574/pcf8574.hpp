#pragma once

//这个驱动已经完成
//这个驱动还未测试

// 这款面向两线制双向总线 (I2C) 的 8 位输入/输出 (I/O) 
// 扩展器设计为在 2.5V 至 6V VCC 下运行。
// PCF8574 器件通过 I2C 接口 [串行时钟 (SCL)、串行数
// 据 (SDA)] 为大多数微控制器系列提供通用远程 I/O 扩
// 展。
// 该器件具有一个 8 位准双向 I/O 端口 (P0–P7)，其中
// 包括具有高电流驱动能力的锁存输出，用于直接驱动 
// LED。每个准双向 I/O 都可以用作输入或输出（无需使
// 用数据方向控制信号）。在上电时，这些 I/O 处于高电
// 平。在该模式下，仅有一个连接到 VCC 的电流源处于
// 活动状态。


#include "device_defs.h"

namespace ymd::drivers{
class PCF8574{

public:
    explicit PCF8574(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit PCF8574(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    void write(const uint8_t _data){
        data = _data;
        i2c_drv_.write(data);
    }

    uint8_t read(){
        uint8_t ret = 0;
        i2c_drv_.read(ret);
        return ret;
    }

    void write_bit(const uint16_t index, const bool value){
        if(value) data |= (1 << index);
        else data &= (~(1 << index));
        i2c_drv_.write(data);
    }

    bool read_bit(const uint16_t index){
        return (read() & (1 << index));
    }
private:

    hal::I2cDrv i2c_drv_;

    uint8_t data;
};
}