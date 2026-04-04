#pragma once

#include "details/vl6180x_prelude.hpp"

namespace ymd::drivers{

class VL6180X final:public VL6180X_Prelude{
public:

    explicit VL6180X(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit VL6180X(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit VL6180X(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_{hal::I2cDrv(i2c, i2c_addr)}{;}

    IResult<> validate();

    IResult<> init();

    IResult<> configure_default();

    IResult<> set_scaling(uint8_t new_scaling);
    IResult<> start_range_continuous(uint16_t period);
    IResult<> start_ambient_continuous(uint16_t period);
    IResult<> start_interleaved_continuous(uint16_t period);
    IResult<> stop_continuous();
    
    IResult<uint16_t> read_ambient();
    

    IResult<uint8_t> read_range_status();

    IResult<> set_max_convergence_time(const uint8_t ms){
        return write_reg(RegAddr::SYSRANGE__MAX_CONVERGENCE_TIME, ms);
    }

    IResult<>  set_inter_measurement_period(const uint8_t ms){
        return write_reg(RegAddr::SYSRANGE__INTERMEASUREMENT_PERIOD, ms);
    }

    IResult<uint16_t> read_range_millimeters() { 
        if(const auto res = read_range(); 
            res.is_err()) return Err(res.unwrap_err());
        else return Ok(uint16_t(scaling) * res.unwrap());
    }

    IResult<> invoke_read_range(){
        return write_reg<uint8_t>(RegAddr::SYSRANGE__START, 0x01);
    }

    IResult<> invoke_read_ambient(){
        return write_reg<uint8_t>(RegAddr::SYSALS__START, 0x01);
    }
private:
    template<typename T>
    requires (sizeof(T) <= 2)
    IResult<> write_reg(RegAddr reg_addr, T reg_val){
        return transport_.write_reg<T>(std::bit_cast<uint16_t>(reg_addr), reg_val);
    }
    
    template<typename T>
    requires (sizeof(T) <= 2)
    IResult<> read_reg(RegAddr reg_addr, T reg_val){
        return transport_.read_reg<T>(std::bit_cast<uint16_t>(reg_addr), reg_val);
    }
    IResult<uint8_t> read_range();
private:
    VL6180X_Transport transport_;
    uint8_t scaling;
    int8_t ptp_offset;
};

}
