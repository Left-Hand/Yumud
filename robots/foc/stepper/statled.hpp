#ifndef __STEPPER_STATLED_HPP__

#define __STEPPER_STATLED_HPP__

#include "constants.hpp"

#include "drivers/CommonIO/Led/rgbLed.hpp"
#include "types/color/color_t.hpp"

using Sys::t;
using Color = Color_t<real_t>;

namespace StepperComponents{
class StatLed{
public:
    enum class Method:uint8_t{
        Sine = 0,
        Saw,
        Square,
        Triangle
    };
protected:

    using RunStatus = StepperEnums::RunStatus;
    using CtrlType = StepperEnums::CtrlType;

    RgbLedConcept & led;

    Color color_a;
    Color color_b;

    uint period;
    Method method;

    volatile RunStatus & run_status;
    volatile CtrlType & ctrl_type;
public:
    StatLed(RgbLedConcept & _led, volatile RunStatus & _run_status, volatile CtrlType & _ctrl_type) : led(_led), run_status(_run_status), ctrl_type(_ctrl_type) {;}

    void init(){
        led.init();
    }

    void setTranstit(const Color & _color_a, const Color & _color_b, const Method _method, const uint _period){
        color_a = _color_a;
        color_b = _color_b;
        method = _method;
        period = _period;
    }

    void run(){
        switch(run_status){
            case RunStatus::ACTIVE:
                switch(ctrl_type){
                    case CtrlType::CURRENT:
                        setTranstit(Color(0,1,0), Color(0,0,0), Method::Triangle, 400);
                        break;
                    case CtrlType::POSITION:
                        setTranstit(Color(1,0,0), Color(0,1,0), Method::Triangle, 400);
                        break;
                    case CtrlType::SPEED:
                        setTranstit(Color(0,0,1), Color(0,1,0), Method::Triangle, 400);
                        break;
                    case CtrlType::TRAPEZOID:
                        setTranstit(Color(1,0,1), Color(1,0,0), Method::Triangle, 400);
                        break;
                    case CtrlType::TEACH:
                        setTranstit(Color(1,1,1), Color(0,0,1), Method::Triangle, 400);
                        break;

                    default:
                        break;
                }
                break;
            case RunStatus::INACTIVE:
                setTranstit(Color(1,1,1), Color(0,0,0), Method::Triangle, 2000);
                break;
            case RunStatus::CALI:
                setTranstit(Color(0,0,1), Color(0,0,0), Method::Square, 200);
                break;
            case RunStatus::BEEP:
                setTranstit(Color(0,1,0), Color(0,0,0), Method::Square, 200);
                break;
            case RunStatus::ERROR:
            case RunStatus::WARN:
                setTranstit(Color(1,0,0), Color(0,0,0), Method::Square, 200);
            default:
                break;
        }

        real_t uni = t * real_t(100000 / period) * real_t(0.01);
        real_t ratio;
        switch(method){
            case Method::Saw:
                ratio = frac(uni);
                break;
            case Method::Sine:
                ratio = abs(real_t(0.5) * sin(uni * real_t(TAU)) + real_t(0.5));
                break;
            case Method::Square:
                ratio = sign(2 * frac(uni) - 1) * real_t(0.5) + real_t(0.5);
                break;
            case Method::Triangle:
                ratio = abs(2 * frac(uni) - 1);
                break;
        }

        Color color_mux = color_a.linear_interpolate(color_b, ratio);
        led = color_mux;
    }

    auto & operator = (const Color & color){
        led = color;
        return *this;
    }
};

}

#endif