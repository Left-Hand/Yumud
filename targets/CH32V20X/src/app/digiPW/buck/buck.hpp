#pragma once

#include "sys/math/real.hpp"
#include "hal/adc/analog_channel.hpp"
#include "drivers/Actuator/Bridge/CoilDriver.hpp"
#include <bit>

#include "../ctrl/current_ctrl.hpp"
#include "../ctrl/power_ctrl.hpp"
#include "../ctrl/voltage_ctrl.hpp"

namespace ymd::digipw{

class BuckController {
public:
    enum class CtrlMode:uint8_t{
        CC,
        CV,
        CP
    };

    BuckController(){;}

    void reset();
protected:
    CtrlMode ctrl_mode_ = CtrlMode::CC;
};

class BuckConverter {
protected:
    using Coil2Driver = ymd::drivers::Coil2Driver;
    BuckController ctrl_;
    AnalogInChannel & curr_ch_;
    AnalogInChannel & volt_ch_;
    Coil2Driver & driver_;
public:
    BuckConverter(AnalogInChannel & _curr_ch, AnalogInChannel & _volt_ch, Coil2Driver & _driver):
        ctrl_(),
        curr_ch_(_curr_ch),
        volt_ch_(_volt_ch),
        driver_(_driver)
    {;}

    void init();
    void tick();
    void run();

};

}