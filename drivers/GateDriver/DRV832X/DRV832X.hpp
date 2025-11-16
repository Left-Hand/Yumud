#pragma once

#include "drv832x_prelude.hpp"

namespace ymd::drivers{


class DRV8323R final:
    public DRV832X_Regs{
public:
    explicit DRV8323R(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit DRV8323R(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit DRV8323R(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
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
        if(const auto res = phy_.write_reg(T::ADDRESS, reg.as_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(T::ADDRESS, reg.as_mut_bits());
    }
};


class DRV8323H_Phy final:public DRV832X_Prelude{
public:
    struct Params{
        hal::Gpio & gain_gpio;
        hal::Gpio & vds_gpio;
        hal::Gpio & idrive_gpio;
        hal::Gpio & mode_gpio;
    };

    DRV8323H_Phy(const Params & params):
        gain_gpio_(params.gain_gpio),
        vds_gpio_(params.vds_gpio),
        idrive_gpio_(params.idrive_gpio),
        mode_gpio_(params.mode_gpio){;}

    void set_pwm_mode(const PwmMode mode){
        // _6x = GND,
        // _3x = 47K to GND,
        // _1x = HiZ,
        // Independent = VDD,

        switch(mode){
            case PwmMode::_6x:
                mode_gpio_.outpp(LOW);
                break;
            case PwmMode::_3x:
                mode_gpio_.inpd();
                break;
            case PwmMode::_1x:
                mode_gpio_.inflt();
                break;
                // mode_gpio_.outpp(HIGH);
                break;
            case  PwmMode::Independent:
                mode_gpio_.outpp(HIGH);
                break;
        }
    }

    void set_idrive(const IDriveP drive){
        // switch(drive){
        //     case IDriveP::
        // }
        idrive_gpio_.inflt();
    }

private:
    hal::Gpio & gain_gpio_;
    hal::Gpio & vds_gpio_;
    hal::Gpio & idrive_gpio_;
    hal::Gpio & mode_gpio_;
};


class DRV8323H final:
    public DRV832X_Prelude{
public:
    static constexpr auto name = "DRV8323H";
    template<typename ... Args>
    explicit DRV8323H(Args && ... args):
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
