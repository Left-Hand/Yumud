#pragma once

//这个驱动还未完成

//HT16K33是一款真I2C的Led矩阵扫描/按键扫描芯片 同时带有中断引脚

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct HT16K33_Collections{

};

struct HT16K33_Regs:public HT16K33_Collections{

};

struct HT16K33_Phy final:public HT16K33_Collections{

};


class HT16K33:public HT16K33_Regs{
public:
private:
    using Phy = HT16K33_Phy;
    Phy phy_;
};
}