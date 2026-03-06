#pragma once

#include "drv832x_prelude.hpp"

namespace ymd::drivers{


class DRV8323R final:public DRV832X_Prelude{
public:
    explicit DRV8323R(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit DRV8323R(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit DRV8323R(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv(spi, rank)){;}


    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);

    IResult<> set_peak_current(const PeakCurrent peak_current);
    IResult<> set_ocp_mode(const OcpMode ocp_mode);
    IResult<> set_gain(const Gain gain);
    IResult<> enable_pwm3(const Enable en);

    IResult<> set_drive_hs(const IDriveP drive_p, const IDriveN drive_n);
    IResult<> set_drive_ls(const IDriveP drive_p, const IDriveN drive_n);
    IResult<> set_drive_time(const PeakDriveTime ptime);

    IResult<Status1> get_status1();
    IResult<Status2> get_status2();
private:
    using Phy = DRV8323R_Transport;
    Phy transport_;
    DRV832X_Regs regs_;


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = transport_.write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    template<typename T>
    IResult<> read_reg(T & reg){
        return transport_.read_reg(T::REG_ADDR, reg.as_bits_mut());
    }
};



};
