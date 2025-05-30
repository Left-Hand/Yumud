#pragma once

#include "../CoilDriver.hpp"
#include "types/regions/range2/range2.hpp"

#include "concept/analog_channel.hpp"
#include "hal/gpio/gpio_intf.hpp"

#include <optional>
#include <array>



struct PwmIntf;

namespace ymd::drivers{



class MP6540:public Coil3DriverIntf{
protected:
    using PWM3 = std::array<hal::PwmIntf *, 3>;
    using AIN3 = std::array<hal::AnalogInIntf *, 3>;

    using PWM3_WP = std::array<std::reference_wrapper<hal::PwmIntf>, 3>;
    using AIN3_WP = std::array<std::reference_wrapper<hal::AnalogInIntf>, 3>;
    
    using EN3 = std::array<hal::GpioIntf *, 3>;

    PWM3 pwms_;
    AIN3 ains_;
    EN3 ens_ = {nullptr, nullptr, nullptr};

    struct MP6540CurrentChannel:public hal::AnalogInIntf{
        hal::AnalogInIntf & ain_;
        real_t ratio_ = real_t(0);

        MP6540CurrentChannel(hal::AnalogInIntf & _ain):
            ain_(_ain){}
        MP6540CurrentChannel(const MP6540CurrentChannel & other) = delete;
        MP6540CurrentChannel(MP6540CurrentChannel && other) = delete;

        void setRatio(const real_t ratio){ratio_ = ratio;}
        operator real_t() override{
            return - (real_t(ain_) * ratio_);
        }
    };

    // real_t volt_to_curr_ratio = 1;

    std::array<MP6540CurrentChannel, 3> chs{
        MP6540CurrentChannel(*ains_[0]),
        MP6540CurrentChannel(*ains_[1]),
        MP6540CurrentChannel(*ains_[2])
    };

public:
    MP6540(PWM3_WP && pwms, AIN3_WP && ains);

    void init();
    
    void enable(const Enable en = EN);

    MP6540CurrentChannel & ch(const size_t index);

    void setSoRes(const uint so_res_ohms);
    // void setBias(const real_t b0, const real_t b1, const real_t b2);

    MP6540 & operator= (const UVW_Duty & duty) override;

};

};