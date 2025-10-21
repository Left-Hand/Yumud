#pragma once

//这个驱动已经完成
//这个驱动已经通过测试

#include "ads7830_prelude.hpp"

namespace ymd::drivers{

struct ADS7830 final:
    public ADS7830_Prelude{
public:
    using Phy = ADS7830_Phy;

    explicit ADS7830(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<ConvData> read_pos_channel(const ChannelSelection nth);
    [[nodiscard]] IResult<ConvData> read_channel(const PairSelection ch);

    void set_pwdn_sel(const PowerDownSel sel){
        pwdn_sel_ = sel;
    }
private:
    Phy phy_;
    PowerDownSel pwdn_sel_ = PowerDownSel::RefOn_AdcOn;
};

}