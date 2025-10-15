#pragma once

#include "drv832x_prelude.hpp"

namespace ymd::drivers{


class DRV8323R final:
    public DRV832X_Regs{
public:
    DRV8323R(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    DRV8323R(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    DRV8323R(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        phy_(hal::SpiDrv(spi, rank)){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> reconf(const Config & cfg);

    [[nodiscard]] IResult<> set_peak_current(const PeakCurrent peak_current);
    [[nodiscard]] IResult<> set_ocp_mode(const OcpMode ocp_mode);
    [[nodiscard]] IResult<> set_gain(const Gain gain);
    [[nodiscard]] IResult<> enable_pwm3(const Enable en);

    [[nodiscard]] IResult<> set_drive_hs(const IDriveP drive_p, const IDriveN drive_n);
    [[nodiscard]] IResult<> set_drive_ls(const IDriveP drive_p, const IDriveN drive_n);
    [[nodiscard]] IResult<> set_drive_time(const PeakDriveTime ptime);

    [[nodiscard]] IResult<R16_Status1> get_status1();
    [[nodiscard]] IResult<R16_Status2> get_status2();
private:
    using Phy = DRV8323R_Phy;
    Phy phy_;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = phy_.write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(reg.address, reg.as_ref());
    }
};


class DRV8323H final:
    public DRV832X_Prelude{
public:

    template<typename ... Args>
    DRV8323H(Args && ... args):
        phy_(std::forward<Args>(args)...){;}


    [[nodiscard]] IResult<> init(const Config & cfg);
    [[nodiscard]] IResult<> reconf(const Config & cfg);

    [[nodiscard]] IResult<> set_peak_current(const PeakCurrent peak_current);
    [[nodiscard]] IResult<> set_ocp_mode(const OcpMode ocp_mode);
    [[nodiscard]] IResult<> set_gain(const Gain gain);
    [[nodiscard]] IResult<> enable_pwm3(const Enable en);

    [[nodiscard]] IResult<> set_drive_hs(const IDriveP drive_p, const IDriveN drive_n);
    [[nodiscard]] IResult<> set_drive_ls(const IDriveP drive_p, const IDriveN drive_n);
    [[nodiscard]] IResult<> set_drive_time(const PeakDriveTime ptime);

private:
    using Phy = DRV8323H_Phy;
    Phy phy_;
};


};
