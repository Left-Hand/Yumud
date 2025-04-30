#pragma once

namespace ymd::digipw::zvs_buck_details{

static constexpr double MOS_CISS_NF = 10;
static constexpr double IND_UH = 370;
static constexpr double OUT_CAP_UF = 370;

template<typename T>
struct utils{
    struct Inductor { T uH; };
    struct Voltage { T V; };
    struct Current { T A; };
    struct Capacitor { T uF; };

    struct SlewDownParams{
        T ind_uh;
        T sw_volt;
    };

    // @return current slewrate(A / us)
    static constexpr T calc_current_slewrate_down(const SlewDownParams & params){
        return -params.sw_volt / params.ind_uh;
    }
    

    struct SlewUpParams{
        T ind_uh;
        T sw_volt;
        T busbar_volt;
    };

    // @return current slewrate(A / us)
    static constexpr T calc_current_slewrate_up(const SlewUpParams & params){
        return (busbar_volt - sw_volt) / ind_uh;
    }
    
    struct DeadZoneParams{
        T deadzone_volt;
        T deadzone_current;
    };

    static constexpr T calc_deadzone_ns(){
        //TODO
        return 0;
    }

    struct PeriodParams{
        T ind_uh;
        T sw_volt;
        T busbar_volt;
        T max_curr;
        T min_curr;
    };

    static constexpr T calc_period_us(const PeriodParams & params){
        const auto up_slewrate = calc_current_slewrate_up({
            .ind_uh = params.ind_uh,
            .sw_volt = params.sw_volt,
            .busbar_volt = params.busbar_volt,
        });

        const auto down_slewrate = calc_current_slewrate_down({
            .ind_uh = params.ind_uh,
            .sw_volt = params.sw_volt,

        });

        const auto up_time_us = (max_curr - min_curr) / up_slewrate;
        const auto down_time_us = (min_curr - max_curr) / down_slewrate;
        return up_time_us + down_time_us;
    }
}


}