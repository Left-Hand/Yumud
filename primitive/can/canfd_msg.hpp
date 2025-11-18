#pragma once

#include "can_msg.hpp"

//这个文件描述了canfd的消息 canfd移除了远程帧

//这个类暂时无法实现
//canfd在硬件上基于DMA进行实现，目前没有合适的内存分配与托管器（在异步过程为内存实现Pin特征）
namespace ymd::hal{

}