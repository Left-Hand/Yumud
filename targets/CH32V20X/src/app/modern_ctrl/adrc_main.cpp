#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "core/clock/time.hpp"


using namespace ymd;

static constexpr auto UART_BAUD = 576000;

void adrc_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    const auto tau = 80.0_r;

    static dsp::Leso leso{dsp::Leso::Config{
        .b0 = 1,
        .w = 17.8_r,
        .fs = 1000
    }};

    static dsp::CommandShaper1 cs{{
        .kp = tau * tau,
        .kd = 2 * tau,
        .max_spd = 40.0_r,
        // .max_acc = 200.0_r,
        // .max_acc = 80.0_r,
        .max_acc = 100.0_r,
        .fs = 1000
    }};

    [[maybe_unused]]
    auto command_shaper_poller = [&](const auto t){

        const auto u = 10 * sign(sin(3 * t));
        const auto u0 = clock::micros();
        cs.update(u);
        const auto u1 = clock::micros();

        leso.update(cs.get()[0], u);


        DEBUG_PRINTLN(
            u,
            cs.get()[0],
            leso.get_disturbance(),
            u1 - u0
        );
    };


    while(true){

        command_shaper_poller(clock::time());


        clock::delay(2ms);
    }

}