#pragma once

// CH9431 是一款控制局域网总线（Controller Area Network，CAN）协议控制器，支持CAN V2.0B
// 技术规范；支持发送和接收标准、扩展数据帧以及远程帧。CH9431 芯片提供两个接收屏蔽寄存器和
// 六个接收过滤寄存器，过滤掉不需要的报文，减少主单片机（MCU）的资源占用。 
// CH9431 通过标准串行外设接口（Searial Peripheral Interface，SPI）和主机设备进行通讯，
// 可为主机设备扩展CAN功能，连接到外部CAN总线。 

#include "details/CH9431_phy.hpp"
#include "hal/bus/can/can_msg.hpp"

namespace ymd::drivers{

class CH9431 final:public CH9431_Prelude{
public:

    IResult<> write(const hal::CanMsg & msg);
    IResult<hal::CanMsg> read();
    IResult<> reset(){
        return phy_.reset_device();
    }
private:
    CH9431_Phy phy_;
};
}