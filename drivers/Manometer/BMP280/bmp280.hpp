
#pragma once

#include "details/BMP280_phy.hpp"

namespace ymd::drivers{

class BMP280 final:public BMP280_Prelude{
public:

    explicit BMP280(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, i2c_addr)){;}

    ~BMP280(){;}

    struct Config{
        DataRate datarate;
    };

    IResult<> init(const Config & cfg);

    IResult<> validate();

    IResult<> set_temprature_sample_mode(const TempratureSampleMode tempMode);

    IResult<> set_pressure_sample_mode(const PressureSampleMode pressureMode);

    IResult<> set_mode(const Mode mode);

    IResult<> set_datarate(const DataRate datarate);

    IResult<> set_filter_coefficient(const FilterCoefficient filterCoeff);

    IResult<> reset();

    IResult<bool> is_idle();

    IResult<> enable_3wire_spi(const Enable en);

    IResult<int32_t> get_pressure();



private:
    BMP280_Transport transport_;
    BMP280_Regset regs_ = {};
    Coeffs coeffs_;

    IResult<uint32_t> get_pressure_data();

    IResult<uint32_t> read_temp_data();


    IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return transport_.write_reg(addr, data);
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return transport_.read_reg(addr, data);
    }

    IResult<> read_bulk(
        const uint8_t addr, 
        std::span<int16_t> pbuf
    ){
        return transport_.read_bulk(addr, pbuf);
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return transport_.read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};

}