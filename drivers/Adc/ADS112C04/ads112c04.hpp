#pragma once

#include "ads112c04_prelude.hpp"

namespace ymd::drivers{


class ADS112C04 final:
    public ADS112C04_Regs{
public:

    explicit ADS112C04(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit ADS112C04(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit ADS112C04(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};


    struct Config{

    };

    IResult<> init();

    IResult<> validate();

    IResult<> set_mux(const Mux mux);

    IResult<> set_gain(const Gain gain);

    IResult<> enable_turbo(const Enable en);

    IResult<bool> is_done();

    IResult<> set_idac(const IDAC idac);

    IResult<> set_datarate(const DataRate datarate);
private:
    hal::I2cDrv i2c_drv_;

    IResult<> read_data(uint16_t & data){
        if(const auto res = i2c_drv_.read_reg(
                std::bit_cast<uint8_t>(Command::READ_DATA), data, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(const RegAddr reg_addr, uint8_t & data){
        if(const auto res = i2c_drv_.read_reg(uint8_t(
                std::bit_cast<uint8_t>(Command::READ_REG) + reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> write_reg(const RegAddr reg_addr, const uint8_t data){
        if(const auto res = i2c_drv_.write_reg(uint8_t(
                std::bit_cast<uint8_t>(Command::WRITE_REG) + reg_addr), data);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};

}