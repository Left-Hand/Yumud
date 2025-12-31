#pragma once

#include "bmp085_prelude.hpp"

// 参考来源

// 无许可证声明
// https://github.com/adafruit/Adafruit-BMP085-Library/blob/master/BMP085.cpp

//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。
namespace ymd::drivers{

class BMP085 :public BMP085_Prelude{
public:
    explicit BMP085(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit BMP085(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit BMP085(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    struct Config{
        Mode mode;
    };

    [[nodiscard]] IResult<> init(const Config & config);

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<Coeffs> get_coeffs();

    [[nodiscard]] IResult<uint16_t> read_raw_temperature();

    [[nodiscard]] IResult<uint32_t> read_raw_pressure();

private:
    hal::I2cDrv i2c_drv_;
    Mode mode_;

    [[nodiscard]] IResult<> read8(RegAddr reg_addr, uint8_t & data) {
        if(const auto res = i2c_drv_.read_reg(std::bit_cast<uint8_t>(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    requires (sizeof(T) == 2)
    [[nodiscard]] IResult<> read16(RegAddr reg_addr, T & data) {
        if(const auto res = i2c_drv_.read_reg(std::bit_cast<uint8_t>(reg_addr), data, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> write8(RegAddr reg_addr, uint8_t data) {
        if(const auto res = i2c_drv_.write_reg(std::bit_cast<uint8_t>(reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }



    Coeffs coeffs_ = Coeffs::from_default();

};


}