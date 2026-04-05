#pragma once

#include "drv832x_prelude.hpp"

namespace ymd::drivers{

class DRV8323H_TransportIntf final:public DRV832X_Prelude{
public:
    // _6x = GND,
    // _3x = 47K to GND,
    // _1x = HiZ,
    // Independent = VDD,
    virtual void set_pwm_mode(const PwmMode mode) = 0;
    virtual void set_idrive(const IDriveP drive) = 0;
};


class DRV8323H final:
    public DRV832X_Prelude{
public:
    static constexpr auto name = "DRV8323H";
    using Transport = DRV8323H_TransportIntf;
    explicit DRV8323H(Transport & transport):
        transport_(transport){;}


    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);

    IResult<> set_peak_current(const PeakCurrent peak_current);
    IResult<> set_ocp_mode(const OcpMode ocp_mode);
    IResult<> set_gain(const Gain gain);
    IResult<> enable_pwm3(const Enable en);

    IResult<> set_drive_hs(const IDriveP drive_p, const IDriveN drive_n);
    IResult<> set_drive_ls(const IDriveP drive_p, const IDriveN drive_n);
    IResult<> set_drive_time(const PeakDriveTime ptime);

private:

    Transport & transport_;
};


};
