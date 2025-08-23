#pragma once

#include "gt911_prelude.hpp"
#include "core/container/inline_vector.hpp"


namespace ymd::drivers{

struct GT911:public Gt911_Prelude{ 
public:
    template<typename T = void>
    using IResult = Result<T, Error>;

    using TouchPoints = InlineVector<TouchPoint, MAX_NUM_TOUCHPOINTS>;

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