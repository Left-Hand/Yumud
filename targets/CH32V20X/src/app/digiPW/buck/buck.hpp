#pragma once

#include "../types/real.hpp"
#include "../hal/adc/analog_channel.hpp"
#include "../drivers/Actuator/Driver/CoilDriver.hpp"
#include <bit>

#include "../ctrl/current_ctrl.hpp"
#include "../ctrl/power_ctrl.hpp"
#include "../ctrl/voltage_ctrl.hpp"


class Buck{
public:
    enum class CtrlMode:uint8_t{
        CC,
        CV,
        CP
    };

    Buck(AnalogInChannel & _curr_ch, AnalogInChannel & _volt_ch, Coil2Driver & _driver):
        curr_ch_(_curr_ch),
        volt_ch_(_volt_ch),
        driver_(_driver)
    {;}

    void init();
    void tick();
    void run();
protected:
    AnalogInChannel & curr_ch_;
    AnalogInChannel & volt_ch_;
    Coil2Driver & driver_;


    CtrlMode ctrl_mode_ = CtrlMode::CC;
};

