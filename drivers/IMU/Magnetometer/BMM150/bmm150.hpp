#pragma once

#include "bmm150_prelude.hpp"

namespace ymd::drivers{

class BMM150:
    public BMM150_Prelude{
public:
    explicit BMM150(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, addr)){;}

    struct Config{

    };

    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<iq24>> read_mag();
private:

    BoschImu_Transport transport_;
    BMM150_Regset regs_ = {};


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = transport_.write_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.to_bits()
        );
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return transport_.read_reg(
            std::bit_cast<uint8_t>(T::ADDRESS), 
            reg.as_bits_mut()
        );
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<int16_t> pbuf){
        return transport_.read_burst(addr, pbuf);
    }
};


}