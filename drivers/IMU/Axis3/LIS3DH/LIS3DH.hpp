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

    explicit LIS3DH(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, addr}){;}
    explicit LIS3DH(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit LIS3DH(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit LIS3DH(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit LIS3DH(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit LIS3DH(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        transport_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> clear_flag();

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<> sleep(){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.norm_mod_en = false; 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> wakeup(){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.norm_mod_en = true; 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> enable_drdy_pulse(const Enable en){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.drdy_pulse = en == EN; 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> block_when_update(const Enable en){
        auto reg = RegCopy(regs_.ctrl1_reg); 
        reg.drdy_pulse = en == EN; 
        return write_reg(reg);
    }


    [[nodiscard]] IResult<> set_filtering_strategy(const FilteringStrategy st){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.dsp_lp_type = bool(st); 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> set_bandwidth(const Bandwidth bw){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.dsp_bw_sel = bool(bw); 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> set_selftest_mode(const SelfTestMode mode){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.selftest_mode = uint8_t(mode); 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> set_int1_pp_or_od(const FormerLatter sel){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.pp_od_int1 = sel; 
        return write_reg(reg);
    }

    [[nodiscard]] IResult<> set_int2_pp_or_od(const FormerLatter sel){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.pp_od_int2 = sel; 
        return write_reg(reg);
    }
        
    [[nodiscard]] IResult<> enable_fifo(const Enable en){
        auto reg = RegCopy(regs_.ctrl4_reg); 
        reg.fifo_en = en == EN; 
        return write_reg(reg);}

    [[nodiscard]] IResult<> enable_spi_hw(const Enable en){
        auto reg = RegCopy(regs_.ctrl5_reg); 
        reg.fifo_spi_hs_on = en == EN; 
        return write_reg(reg);
    }

private:


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg);

    IResult<> read_reg(auto & reg);

    IResult<> verify_phy();

    using Regs = _LIS3DH_Regs;
    Regs regs_ = {};

    Transport transport_;
};

}

namespace ymd::drivers{

template<typename T>
LIS3DH::IResult<> LIS3DH::write_reg(const RegCopy<T> & reg){
    const auto res = transport_.write_reg(T::ADDRESS, reg.to_bits());
    if(res.is_err()) return res;
    reg.apply();
    return Ok();
}

template<typename T>    
LIS3DH::IResult<> LIS3DH::read_reg(T & reg){
    return LIS3DH::IResult<>(transport_.read_reg(T::ADDRESS, reg.as_bits_mut()));
}

LIS3DH::IResult<> LIS3DH::verify_phy(){
    return LIS3DH::IResult<>(transport_.validate());
}

}