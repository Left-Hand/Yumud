#pragma once

// 锐评：贵到七八块钱了，我还不如暂时先把自己的库搞好，比这种专用芯片便宜可定制度还高

// CH9434 是一款 SPI 转四串口转接芯片，提供四组全双工的 9 线异步串口，用于单片机/嵌入式系
// 统扩展异步串口。CH9434 包含四个兼容 16C550 的异步串口，最高支持 4Mbps 波特率通讯。最多支持
// 25 路 GPIO，提供半双工收发自动切换引脚 TNOW。



#include "details/CH9431_phy.hpp"

namespace ymd::drivers{

class CH9431 final{

};
}