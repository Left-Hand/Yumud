#pragma once

#include "../CoilDriver.hpp"
#include "types/range/range_t.hpp"
#include "hal/adc/analog_channel.hpp"

#include <optional>
#include <array>

struct PwmChannel;

#ifdef MP6540_DEBUG
#undef MP6540_DEBUG
#define MP6540_DEBUG(...) DEBUG_LOG(...)
#else 
#define MP6540_DEBUG(...)
#endif


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
        const real_t ratio_;
        real_t basis_ = real_t(0);
        size_t index_;

        MP6540CurrentChannel(AnalogInChannel & _ain, const real_t _ratio, const uint8_t _index):
            ain_(_ain), ratio_(_ratio), index_(_index) {}
        MP6540CurrentChannel(const MP6540CurrentChannel & other) = delete;
        MP6540CurrentChannel(MP6540CurrentChannel && other) = delete;

        real_t getRawVoltage() {return real_t(ain_);}
        void setBasis(const real_t _basis) {basis_ = _basis;}
        operator real_t() override{
            return real_t(ain_) * ratio_ - basis_;
        }
    };

    real_t volt_to_curr_ratio = 1;

    std::array<MP6540CurrentChannel, 3> chs{
        MP6540CurrentChannel(*ains_[0], volt_to_curr_ratio, 0),
        MP6540CurrentChannel(*ains_[1], volt_to_curr_ratio, 1),
        MP6540CurrentChannel(*ains_[2], volt_to_curr_ratio, 2)
    };

public:
    MP6540(PWM3_WP && pwms, AIN3_WP && ains);

    void init();
    
    void enable(const bool en = true);

    MP6540CurrentChannel & ch(const size_t index);

    void setSoRes(const real_t so_res_ohms);

    MP6540 & operator= (const UVW_Duty & duty) override;

};