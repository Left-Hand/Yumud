#pragma once

// GT911 是专为7”~8”设计的新一代5点电容触控方案，拥有26个驱动通道和14个感
// 应通道，以满足更高的touch精度要求。 
// GT911 可同时识别5个触摸点位的实时准确位置，移动轨迹及触摸面积。并可根据主
// 控需要，读取相应点数的触摸信息。

// https://aitendo3.sakura.ne.jp/aitendo_data/product_img/ic/touch/GT911/GT911%20Datasheet_20130319.pdf

#include "gt911_prelude.hpp"
#include "core/container/inline_vector.hpp"


namespace ymd::drivers{

struct GT9XX final:public GT9XX_Prelude{ 
public:
    template<typename T = void>
    using IResult = Result<T, Error>;

    using TouchPoints = HeaplessVector<TouchPoint, MAX_NUM_TOUCHPOINTS>;

    explicit GT9XX(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit GT9XX(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    //验证设备是否有效
    IResult<> validate();

    //对设备进行初始化
    IResult<> init();

    //获取指定序号的可能的触摸点 
    //对于多个触摸点, 请使用更高效的实现get_touch_points
    IResult<Option<TouchPoint>> get_touch_point(Nth nth);

    //获取多个触摸点
    IResult<TouchPoints> get_touch_points();

    //获取触摸点的数量
    IResult<size_t> get_num_touch_points();
private:
    hal::I2cDrv i2c_drv_;

    IResult<TouchPoint> get_touch_point_unchecked(const Nth nth);
    IResult<> clear_status();

    IResult<> write(const uint16_t addr, const uint8_t val){
        if(const auto res = i2c_drv_.write_reg(addr, val, MSB);
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    };

    IResult<> read(const uint16_t addr, const std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf, MSB);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };

    IResult<> write_command(const uint16_t addr, const Command cmd){
        // const auto buf[1] = {std::bit_cast<uint8_t>(cmd)};

        if(const auto res = write(addr, std::bit_cast<uint8_t>(cmd));
            res.is_err()) return Err(res.unwrap_err());

        return Ok();
    };
};
}