#pragma once

#include "drv832x_prelude.hpp"
#include "core/container/closure/heapless_function.hpp"

namespace ymd::drivers{

class DRV8323H_Vtable final:public DRV832X_Prelude{
public:
    // _6x = GND,
    // _3x = 47K to GND,
    // _1x = HiZ,
    // Independent = VDD,
    static constexpr size_t SBO_SIZE = 8;

    HeaplessFunction<IResult<>(PwmMode), SBO_SIZE> set_pwm_mode;
    HeaplessFunction<IResult<>(IDriveP), SBO_SIZE> set_drive_hs;
};

template<typename Backend>
class DRV8323H final:
    public DRV832X_Prelude{
public:
    using Transport = DRV8323H_Vtable;
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
