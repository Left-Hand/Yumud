#pragma once

#include "l3g4200d_prelude.hpp"


namespace ymd::drivers{

class L3G4200D:public L3G4200D_Prelude{
public:
    explicit L3G4200D(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR)
        :i2c_drv_(hal::I2cDrv(i2c, addr)){}
    explicit L3G4200D(hal::I2cDrv && i2c_drv):
        i2c_drv_(i2c_drv){}

    struct Config{
        Dps dps;
        OdrBw odrbw;
    };

    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);

    IResult<Dps> get_dps();
    IResult<OdrBw> get_odrbw();

    IResult<> calibrate(uint8_t samples);
    IResult<> update();

    IResult<math::Vec3<iq24>> read_acc();
    IResult<uint8_t> read_temperature();
private:
    using Regs = L3G4200D_Regs;
    Regs regs_ = {};
    hal::I2cDrv i2c_drv_;

    IResult<> write_reg(uint8_t reg_addr, uint8_t reg_val){
        if(const auto res = i2c_drv_.write_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_reg(uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = i2c_drv_.read_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(uint8_t reg_addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(reg_addr, pbuf, std::endian::little);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

}

