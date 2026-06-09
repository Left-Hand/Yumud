#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/STMicroIMU.hpp"
#include "lis3dh_prelude.hpp"

enum FormerLatter:bool{
    Former,
    Latter
};

namespace ymd::drivers{

class LIS3DH: public LIS3DH_Prelude{

public:

    explicit LIS3DH(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv{i2c, i2c_addr}){;}
    explicit LIS3DH(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit LIS3DH(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit LIS3DH(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit LIS3DH(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit LIS3DH(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv{spi, rank}){;}

    IResult<> init();
    IResult<> update();
    IResult<> validate();
    IResult<> reset();

    IResult<> clear_flag();

    IResult<math::Vec3<iq24>> get_acc();
    IResult<> sleep(){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.norm_mod_en = false; 
        return write_reg(reg);
    }

    IResult<> wakeup(){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.norm_mod_en = true; 
        return write_reg(reg);
    }

    IResult<> enable_drdy_pulse(const Enable en){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.drdy_pulse = (en == EN); 
        return write_reg(reg);
    }

    IResult<> block_when_update(const Enable en){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.drdy_pulse = (en == EN); 
        return write_reg(reg);
    }


    IResult<> set_filtering_strategy(const FilteringStrategy st){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.dsp_lp_type = bool(st); 
        return write_reg(reg);
    }

    IResult<> set_bandwidth(const Bandwidth bw){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.dsp_bw_sel = bool(bw); 
        return write_reg(reg);
    }

    IResult<> set_selftest_mode(const SelfTestMode mode){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.selftest_mode = uint8_t(mode); 
        return write_reg(reg);
    }

    IResult<> set_int1_pp_or_od(const FormerLatter sel){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.pp_od_int1 = sel; 
        return write_reg(reg);
    }

    IResult<> set_int2_pp_or_od(const FormerLatter sel){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.pp_od_int2 = sel; 
        return write_reg(reg);
    }
        
    IResult<> enable_fifo(const Enable en){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.fifo_en = (en == EN); 
        return write_reg(reg);}

    IResult<> enable_spi_hw(const Enable en){
        auto reg = RegCopy(regs_.ctrl5_reg); 
        reg.fifo_spi_hs_on = (en == EN); 
        return write_reg(reg);
    }

private:
    Transport transport_;

    using Regset = LIS3DH_Regset;
    Regset regs_ = {};


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = transport_.write_reg(
            static_cast<uint8_t>(T::REG_ADDR), reg.to_bits()
        );  res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = transport_.read_reg(
            static_cast<uint8_t>(T::REG_ADDR), reg.as_bits_mut()
        );  res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> verify_phy(){
        return LIS3DH::IResult<>(transport_.validate());
    }


};

}