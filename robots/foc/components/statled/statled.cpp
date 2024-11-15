#include "statled.hpp"

using namespace ymd;
using namespace ymd::foc::StepperComponents;


void StatLed::run(){
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

