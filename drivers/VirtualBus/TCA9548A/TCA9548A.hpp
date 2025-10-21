

#pragma once

//数据手册:
// https://www.ti.com.cn/cn/lit/ds/symlink/tca9548a.pdf

// TCA9548A 器件配有八个可通过 I2C 总线控制的双向
// 转换开关。SCL/SDA 上行对扩展到八个下行对，或者
// 通道。根据可编程控制寄存器的内容，可选择任一单独 
// SCn/SDn 通道或者通道组合。这些下游通道可用于解
// 决 I2C 目标地址冲突。例如，如果应用中需要八个完全
// 相同的数字温度传感器，则每个通道 (0-7) 可以连接一
// 个传感器。



#include "core/io/regs.hpp"
#include "core/utils/Option.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class TCA9548A{
public:

class TCA9548A_VirtualI2c final: public hal::I2c{
private:
    TCA9548A & host_;
    const uint8_t ch_;
    hal::HalResult lead(const hal::I2cSlaveAddrWithRw req){return host_.lead(req, ch_);}
    void trail(){return host_.trail(ch_);}
public:
    TCA9548A_VirtualI2c(TCA9548A & host, const uint8_t ch);

    hal::HalResult write(const uint32_t data) {return host_.write(data);}
    hal::HalResult read(uint8_t & data, const Ack ack) {return host_.read(data, ack);}
    hal::HalResult unlock_bus() {return host_.unlock_bus();}
    hal::HalResult set_baudrate(const uint32_t baud){return host_.set_baudrate(baud);}

    void lend(){;}
};


public:
    TCA9548A(hal::I2cDrv && i2c_drv):
        i2c_(i2c_drv.bus()), self_i2c_drv_(std::move(i2c_drv)){;}
    TCA9548A(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr):
        i2c_(i2c.deref()), self_i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    auto & operator [](const size_t ch){
        if(unlikely(ch >= 8)) while(true);
        return v_i2cs_[ch];
    }

    auto which() const {return last_ch_;}

    hal::HalResult validate() {
        return self_i2c_drv_.validate();
    }

private:
    hal::I2c & i2c_;
    hal::I2cDrv self_i2c_drv_;
    Option<uint8_t> last_ch_ = None;

    hal::HalResult switch_vbus(const uint8_t ch);
    hal::HalResult unlock_bus(){return i2c_.unlock_bus();}

    hal::HalResult lead(const hal::I2cSlaveAddrWithRw address, const uint8_t ch);

    void trail(const uint8_t ch);

    hal::HalResult set_baudrate(const uint32_t baud){return i2c_.set_baudrate(baud);}

    hal::HalResult write(const uint32_t data){
        return i2c_.write(data);
    }

    hal::HalResult read(uint8_t & data, const Ack ack){
        return i2c_.read(data, ack);
    }


    friend class TCA9548A_VirtualI2c;

    std::array<TCA9548A_VirtualI2c, 8> v_i2cs_{
        TCA9548A_VirtualI2c(*this, 0),
        TCA9548A_VirtualI2c(*this, 1),
        TCA9548A_VirtualI2c(*this, 2),
        TCA9548A_VirtualI2c(*this, 3),
        TCA9548A_VirtualI2c(*this, 4),
        TCA9548A_VirtualI2c(*this, 5),
        TCA9548A_VirtualI2c(*this, 6),
        TCA9548A_VirtualI2c(*this, 7),
    };
};

}