#pragma once

#include "tof10120_prelude.hpp"

namespace ymd::drivers{


class TOF10120 final:
    public TOF10120_Prelude{
public:
    explicit TOF10120(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit TOF10120(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit TOF10120(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        i2c_drv_(i2c, i2c_addr){;}

    TOF10120(const TOF10120 &) = delete;
    TOF10120(TOF10120 &&) = delete;
    ~TOF10120() = default;

    IResult<uint16_t> get_realtime_distance();
    IResult<uint16_t> get_filtered_distance();
    IResult<int16_t> get_distance_diff();
    IResult<RangeMode> get_range_mode();
    IResult<CommPort> get_comm_port();
private:
    hal::I2cDrv i2c_drv_;

    template<typename T>
    IResult<> write_reg(const RegAddr reg_addr, const T reg_val){
        if(const auto res = i2c_drv_.write_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, std::endian::little
        ); res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(const RegAddr reg_addr, T & reg_val){
        if(const auto res = i2c_drv_.read_reg(
            std::bit_cast<uint8_t>(reg_addr), 
            reg_val, std::endian::little
        ); res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
};

};