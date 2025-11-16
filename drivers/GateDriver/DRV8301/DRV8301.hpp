#pragma once

#include "drv8301_prelude.hpp"

namespace ymd::drivers{


class DRV8301 final:
    public DRV8301_Prelude{
public:
    static constexpr auto NAME = "DRV8301";
    explicit DRV8301(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit DRV8301(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit DRV8301(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> set_peak_current(   const PeakCurrent peak_current);
    [[nodiscard]] IResult<> set_ocp_mode(       const OcpMode ocp_mode);
    [[nodiscard]] IResult<> set_octw_mode(      const OctwMode octw_mode);
    [[nodiscard]] IResult<> set_gain(           const Gain gain);
    [[nodiscard]] IResult<> set_oc_ad_table(    const OcAdTable oc_ad_table);
    [[nodiscard]] IResult<> enable_pwm3(        const Enable en);
private:
    hal::SpiDrv spi_drv_;
    DRV8301_Regset regs_ = {};

    [[nodiscard]] IResult<> _write_reg(const RegAddr addr, const uint16_t reg);
    [[nodiscard]] IResult<> _read_reg(const RegAddr addr, uint16_t & reg);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = _write_reg(T::ADDRESS, reg.as_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = _read_reg(reg.address, reg.as_bits());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

};