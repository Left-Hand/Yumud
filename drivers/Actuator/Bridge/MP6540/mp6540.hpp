#pragma once

#include "../CoilDriver.hpp"
#include "types/range/range_t.hpp"
#include "hal/adc/analog_channel.hpp"

#include <optional>
#include <array>


#ifdef MP6540_DEBUG
#undef MP6540_DEBUG
#define MP6540_DEBUG(...) DEBUG_LOG(...)
#else 
#define MP6540_DEBUG(...)
#endif

struct PwmChannel;

namespace ymd::drivers{



class MP6540:public Coil3Driver{
protected:
    using PWM3 = std::array<PwmChannel *, 3>;
    using AIN3 = std::array<AnalogInChannel *, 3>;

    using PWM3_WP = std::array<std::reference_wrapper<PwmChannel>, 3>;
    using AIN3_WP = std::array<std::reference_wrapper<AnalogInChannel>, 3>;
    
    using EN3 = std::array<GpioConcept *, 3>;

    PWM3 pwms_;
    AIN3 ains_;
    EN3 ens_ = {nullptr, nullptr, nullptr};

    struct MP6540CurrentChannel:public AnalogInChannel{
        AnalogInChannel & ain_;
        real_t ratio_ = 0;
        real_t bias_ = 0;

        MP6540CurrentChannel(AnalogInChannel & _ain):
            ain_(_ain){}
        MP6540CurrentChannel(const MP6540CurrentChannel & other) = delete;
        MP6540CurrentChannel(MP6540CurrentChannel && other) = delete;

        real_t getRawVoltage() {return real_t(ain_);}
        void setBias(const real_t _basis) {bias_ = _basis;}
        operator real_t() override{
            return - (real_t(ain_) * ratio_ - bias_);
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
    
    void enable(const bool en = true);

    MP6540CurrentChannel & ch(const size_t index);

    void setSoRes(const uint so_res_ohms);
    void setBias(const real_t b0, const real_t b1, const real_t b2);

    MP6540 & operator= (const UVW_Duty & duty) override;

};

};