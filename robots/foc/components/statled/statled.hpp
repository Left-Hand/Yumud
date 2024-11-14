#pragma once

#include "robots/foc/motor_utils.hpp"

#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "types/color/color_t.hpp"



namespace ymd::foc::StepperComponents{

using Sys::t;
using Color = ymd::Color_t<real_t>;


using namespace ymd::drivers;
class StatLed{
public:
    enum class Method:uint8_t{
        Sine = 0,
        Saw,
        Square,
        Triangle
    };
protected:

    using RunStatus = MotorUtils::RunStatus;
    using CtrlType = MotorUtils::CtrlType;

    RgbLedConcept & led;

    Color color_a;
    Color color_b;

    uint period;
    Method method;

    volatile RunStatus & run_status;
    volatile CtrlType & ctrl_type;
public:
    StatLed(RgbLedConcept & _led, volatile RunStatus & _run_status, volatile CtrlType & _ctrl_type) : led(_led), run_status(_run_status), ctrl_type(_ctrl_type) {;}

    // void init(){
    //     led.init();
    // }

    void setTranstit(const Color & _color_a, const Color & _color_b, const Method _method, const uint _period){
        color_a = _color_a;
        color_b = _color_b;
        method = _method;
        period = _period;
    }

    void run();

    auto & operator = (const Color & color){
        led = color;
        return *this;
    }
};

}