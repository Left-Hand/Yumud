#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/fp/matchit.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "drivers/Encoder/odometer.hpp"

#include "algo/interpolation/cubic.hpp"

#include "robots/rpc/rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "drivers/Encoder/Encoder.hpp"
#include "drivers/Encoder/AnalogEncoder.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"

using namespace ymd;

//AT8222
class StepperSVPWM{
public:

    hal::GenericTimer & timer_;

    static void init_channel(hal::TimerOC & oc){
        oc.init({.valid_level = LOW});
    }

    void init(const uint32_t freq){
        timer_.init({freq});
    }
private:
    static constexpr std::pair<real_t, real_t> split_duty_in_pair(const real_t duty){
        if(duty > 0){
            return {real_t(0), frac(duty)};
        }else{
            return {frac(-duty), real_t(0)};
        }
    }
};
