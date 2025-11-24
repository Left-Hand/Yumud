
#pragma once

#include "details/BMP280_phy.hpp"

namespace ymd::drivers{

class BMP280 final:public BMP280_Prelude{
public:

    explicit BMP280(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}

    ~BMP280(){;}

    struct Config{
        DataRate datarate;
    };

    [[nodiscard]] IResult<> init(const Config & cfg);

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> set_temprature_sample_mode(const TempratureSampleMode tempMode);

    [[nodiscard]] IResult<> set_pressure_sample_mode(const PressureSampleMode pressureMode);

    [[nodiscard]] IResult<> set_mode(const Mode mode);

    [[nodiscard]] IResult<> set_datarate(const DataRate data_rate);

    [[nodiscard]] IResult<> set_filter_coefficient(const FilterCoefficient filterCoeff);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<bool> is_idle();

    [[nodiscard]] IResult<> enable_3wire_spi(const Enable en);

    [[nodiscard]] IResult<int32_t> get_pressure();



private:
    BMP280_Phy phy_;
    BMP280_Regset regs_ = {};
    Coeffs coeffs_;

    [[nodiscard]] IResult<uint32_t> get_pressure_data();

    [[nodiscard]] IResult<uint32_t> read_temp_data();


    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_burst(
        const uint8_t addr, 
        std::span<int16_t> pbuf
    ){
        return phy_.read_burst(addr, pbuf);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(T::ADDRESS, reg.as_bits_mut());
    }
};

}