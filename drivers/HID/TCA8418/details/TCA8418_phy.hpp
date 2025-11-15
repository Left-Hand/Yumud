#pragma once

#include "TCA8418_Prelude.hpp"

namespace ymd::drivers{


class TCA8418_Phy final:public TCA8418_Prelude{
public:
    TCA8418_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    Result<void, Error> write_reg(const uint8_t addr, const uint8_t data);
    Result<void, Error> read_reg(const uint8_t addr, uint8_t & data);

    Result<void, Error> write_reg(const auto & reg){
        return write_reg(reg.address, reg.as_bits());
    }

    Result<void, Error> read_reg(auto & reg){
        return read_reg(reg.address, reg.as_mut_bits());
    }
private:
    hal::I2cDrv i2c_drv_;
};

}