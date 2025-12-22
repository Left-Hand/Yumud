#pragma once

#include "mmc5603_prelude.hpp"
namespace ymd::drivers{

class MMC5603:
    public MagnetometerIntf,
    public MMC5603_Prelude{
public:

    explicit MMC5603(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit MMC5603(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit MMC5603(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_data_rate(const DataRate dr);

    [[nodiscard]] IResult<> set_band_width(const BandWidth bw);

    [[nodiscard]] IResult<> enable_continuous(const Enable en);

    [[nodiscard]] IResult<> inhibit_channels(bool x, bool y, bool z);

    [[nodiscard]] IResult<Vec3<iq24>> read_mag();

protected:

    hal::I2cDrv i2c_drv_;
    MMC5603_Regset regs_ = {};  


    [[nodiscard]] IResult<> read_burst(const RegAddr addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = i2c_drv_.write_reg(
            uint8_t(T::ADDRESS), 
            reg.to_bits(), std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = i2c_drv_.read_reg(
            uint8_t(reg.ADDRESS), 
            reg.as_bits_mut(), std::endian::little);
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }


    [[nodiscard]] IResult<> set_self_test_threshlds(uint8_t x, uint8_t y, uint8_t z);

};

}