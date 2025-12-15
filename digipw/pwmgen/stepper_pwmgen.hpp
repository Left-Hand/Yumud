#pragma once

#include "hal/timer/bipolarity_abstract.hpp"
#include "digipw/prelude/abdq.hpp"

namespace ymd::digipw{

//AT8222
class StepperPwmGen{
public:
    explicit StepperPwmGen(
        hal::TimerOC & pwm_ap,
        hal::TimerOC & pwm_an,
        hal::TimerOC & pwm_bp,
        hal::TimerOC & pwm_bn
    ):
        channel_a_(pwm_ap, pwm_an),
        channel_b_(pwm_bp, pwm_bn)
    {;}

    void init_channels(){
        channel_a_.inverse(EN);
        channel_b_.inverse(DISEN);

        static constexpr hal::TimerOcPwmConfig pwm_noinv_cfg = []{
            auto config = hal::TimerOcPwmConfig::from_default();
            config.cvr_sync_en = EN;
            config.valid_level = HIGH;
            return config;
        }();

        static constexpr hal::TimerOcPwmConfig pwm_inv_cfg = []{
            auto config = hal::TimerOcPwmConfig::from_default();
            config.cvr_sync_en = EN;
            config.valid_level = LOW;
            return config;
        }();

        // static constexpr hal::TimerOcPwmConfig pwm_noinv_cfg = {
        //     .cvr_sync_en = EN,
        //     .valid_level = LOW
        // };

        // static constexpr hal::TimerOcPwmConfig pwm_inv_cfg = {
        //     .cvr_sync_en = EN,
        //     .valid_level = HIGH,
        // };

        // channel_a_.inverse(DISEN);
        // channel_b_.inverse(EN);


        
        channel_a_.pos_channel().init(pwm_noinv_cfg);
        channel_a_.neg_channel().init(pwm_noinv_cfg);
        channel_b_.pos_channel().init(pwm_inv_cfg);
        channel_b_.neg_channel().init(pwm_inv_cfg);

    }

    void set_dutycycle(const AlphaBetaCoord<iq16> alphabeta_dutycycle){
        channel_a_.set_dutycycle(alphabeta_dutycycle.alpha);
        channel_b_.set_dutycycle(alphabeta_dutycycle.beta);
    }
private:

    hal::BipolarityTimerOcPair channel_a_;
    hal::BipolarityTimerOcPair channel_b_;
};

}