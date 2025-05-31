
#pragma once

#include "details/BMP280_phy.hpp"

namespace ymd::drivers{

class BMP280 final:private BMP280_Regs{
public:
    using BMP280_Regs::Error;

    BMP280(hal::I2c & _i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(_i2c, addr)){;}
    ~BMP280(){;}

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_temprature_sample_mode(const TempratureSampleMode tempMode);

    [[nodiscard]] IResult<> set_pressure_sample_mode(const PressureSampleMode pressureMode);

    [[nodiscard]] IResult<> set_mode(const Mode mode);

    [[nodiscard]] IResult<> set_datarate(const DataRate dataRate);

    [[nodiscard]] IResult<> set_filter_coefficient(const FilterCoefficient filterCoeff);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<bool> is_idle();

    [[nodiscard]] IResult<> enable_spi3(const Enable en = EN);

    [[nodiscard]] IResult<int32_t> get_pressure();

    [[nodiscard]] IResult<> init();

private:
    BMP280_Phy phy_;
    [[nodiscard]] IResult<uint32_t> get_pressure_data();

    [[nodiscard]] IResult<uint32_t> read_temp_data();


    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_burst(
        const uint8_t addr, std::span<int16_t> pdata){
        return phy_.read_burst(addr, pdata);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }
};

}