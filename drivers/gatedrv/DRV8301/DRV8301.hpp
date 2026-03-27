#pragma once

#include "drv8301_prelude.hpp"
#include "core/io/regs.hpp"

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


    IResult<> init();
    IResult<> set_peak_current(   const PeakCurrent peak_current);
    IResult<> set_ocp_mode(       const OcpMode ocp_mode);
    IResult<> set_octw_mode(      const OctwMode octw_mode);
    IResult<> set_gain(           const Gain gain);
    IResult<> set_oc_ad_table(    const OcAdTable oc_ad_table);
    IResult<> enable_pwm3(        const Enable en);
private:
    hal::SpiDrv spi_drv_;
    DRV8301_Regset regs_ = {};

    IResult<> _write_reg(const RegAddr addr, const uint16_t reg);
    IResult<> _read_reg(const RegAddr addr, uint16_t & reg);

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = _write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }
    
    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = _read_reg(reg.address, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

};