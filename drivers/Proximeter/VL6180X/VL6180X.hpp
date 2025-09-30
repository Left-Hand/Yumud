#pragma once

#include "details/vl6180x_prelude.hpp"
namespace ymd::drivers{

class VL6180X final:public VL6180X_Prelude{
public:

    explicit VL6180X(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit VL6180X(hal::I2cDrv && i2c_drv):
        phy_(std::move(i2c_drv)){;}
    explicit VL6180X(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_{hal::I2cDrv(i2c, addr)}{;}

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> configure_default();

    [[nodiscard]] IResult<> set_scaling(uint8_t new_scaling);
    [[nodiscard]] IResult<> start_range_continuous(uint16_t period);
    [[nodiscard]] IResult<> start_ambient_continuous(uint16_t period);
    [[nodiscard]] IResult<> start_interleaved_continuous(uint16_t period);
    [[nodiscard]] IResult<> stop_continuous();
    
    [[nodiscard]] IResult<uint16_t> read_ambient();
    

    [[nodiscard]] IResult<uint8_t> read_range_status();

    [[nodiscard]] IResult<> set_max_convergence_time(const uint8_t ms){
        return write_reg(RegAddress::SYSRANGE__MAX_CONVERGENCE_TIME, ms);
    }

    [[nodiscard]] IResult<>  set_inter_measurement_period(const uint8_t ms){
        return write_reg(RegAddress::SYSRANGE__INTERMEASUREMENT_PERIOD, ms);
    }

    [[nodiscard]] IResult<uint16_t> read_range_millimeters() { 
        if(const auto res = read_range(); 
            res.is_err()) return Err(res.unwrap_err());
        else return Ok(uint16_t(scaling) * res.unwrap());
    }

    [[nodiscard]] IResult<> invoke_read_range(){
        return write_reg(RegAddress::SYSRANGE__START, 0x01);
    }

    [[nodiscard]] IResult<> invoke_read_ambient(){
        return write_reg(RegAddress::SYSALS__START, 0x01);
    }
private:
    [[nodiscard]] IResult<> write_reg(RegAddress reg, uint8_t value){
        return phy_.write_reg<uint8_t>(std::bit_cast<uint16_t>(reg), value);
    }

    [[nodiscard]] IResult<> write_reg(uint16_t reg, uint8_t value){
        return phy_.write_reg<uint8_t>(std::bit_cast<uint16_t>(reg), value);
    }

    [[nodiscard]] IResult<> write_reg16_bit(RegAddress reg, uint16_t value){
        return phy_.write_reg<uint16_t>(std::bit_cast<uint16_t>(reg), value);
    }
    [[nodiscard]] IResult<> write_reg16_bit(const uint16_t reg, uint16_t value){
        return phy_.write_reg<uint16_t>(reg, value);
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(uint16_t reg, T & value){
        return phy_.read_reg<T>(std::bit_cast<uint16_t>(reg), value);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(RegAddress reg, T & value){
        return read_reg(std::bit_cast<uint16_t>(reg), value);
    }
    [[nodiscard]] IResult<> read_reg16_bit(RegAddress reg, uint16_t & value){
        return phy_.read_reg<uint16_t>(std::bit_cast<uint16_t>(reg), value);
    }

    [[nodiscard]] IResult<uint8_t> read_range();
private:
    VL6180X_Phy phy_;
    uint8_t scaling;
    int8_t ptp_offset;
};

}

namespace ymd{
DEF_DERIVE_DEBUG(drivers::VL6180X::Error_Kind );
}