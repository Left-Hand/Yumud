#pragma once

//这个驱动已经完成
//这个驱动还未测试

// Apache-2.0 license
// https://github.com/embedded-drivers/sitronix-touch

#include "st1615_prelude.hpp"

namespace ymd::drivers{

struct ST1615 final:public ST1615_Prelude{

    explicit ST1615(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<GestureInfo> get_gesture_info();

    [[nodiscard]] IResult<Option<Point>> get_point(uint8_t nth);

    [[nodiscard]] IResult<uint16_t> get_sensor_count();

    [[nodiscard]] IResult<Capabilities> get_capabilities();

    [[nodiscard]] IResult<>  blocking_until_normal_status();

private:
    hal::I2cDrv i2c_drv_;
    Capabilities capabilities_;

    [[nodiscard]] IResult<uint8_t> read_reg8(uint8_t reg){
        std::array<uint8_t, 2>buf = {0, 1};
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok(buf[0]);
    }

    [[nodiscard]] IResult<> read_burst(uint8_t reg, std::span<uint8_t> buf){ 
        if(const auto res = i2c_drv_.read_burst<uint8_t>(reg, buf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}
