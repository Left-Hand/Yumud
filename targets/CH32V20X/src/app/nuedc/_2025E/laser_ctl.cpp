#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/utils/streamed_string_splitter.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "src/testbench/tb.h"
#include "core/math/realmath.hpp"
#include "core/sync/timer.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/nodes/msg_factory.hpp"
#include "robots/nodes/node_role.hpp"

#include "types/vectors/vector2/Vector2.hpp"

using namespace ymd;
using namespace ymd::robots;
// using namespace ymd::hal;


#define DBG_UART hal::uart2
static constexpr uint32_t TIM_FREQ = 5000;
static constexpr uint32_t ISR_FREQ = TIM_FREQ / 2;

enum class Command:uint8_t{
    On,
    Off
};

// static constexpr


void laser_ctl_main(){
    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();
    DEBUG_PRINTLN("powerup");

    auto & can = hal::can1;
    can.init({
        .baudrate = hal::CanBaudrate::_1M,
        .mode = hal::CanMode::Normal,
        .remap = 0
    });

    can[0].mask(
        {
            .id = hal::CanStdIdMask{0x200, hal::CanRemoteSpec::Any}, 
            .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
        },{
            .id = hal::CanStdIdMask{0x000, hal::CanRemoteSpec::Any}, 
            // .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
            .mask = hal::CanStdIdMask::from_accept_all()
        }
    );


    auto & led = hal::portB[8];
    led.outpp(HIGH);

    auto set_led = [&](const bool l){
        // led.write(BoolLevel::from(l));
        // DEBUG_PRINTLN("led", l);
        if(l) led.set();
        else led.clr();
    };


    // while(true){
    //     set_led(1);
    //     clock::delay(200ms);
    //     set_led(0);
    //     clock::delay(200ms);
    // }




    can[0].mask(
        {
            .id = hal::CanStdIdMask{0x200, hal::CanRemoteSpec::Any}, 
            .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
        },{
            .id = hal::CanStdIdMask{0x000, hal::CanRemoteSpec::Any}, 
            // .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
            .mask = hal::CanStdIdMask::from_accept_all()
        }
    );


    [[maybe_unused]] auto & mode1_gpio   = hal::portB[1];
    [[maybe_unused]] auto & phase_gpio   = hal::portA[7];
    phase_gpio.outpp();


    hal::timer3.init({TIM_FREQ, hal::TimerCountMode::CenterAlignedUpTrig});
    auto & pwm = hal::timer3.oc<1>();
    pwm.init({});

    bool duty_is_forward = false;

    auto set_duty = [&](real_t duty){
        duty = CLAMP2(duty, 0.99_r);
        duty_is_forward = duty > 0.0_r;
        phase_gpio = BoolLevel::from(duty_is_forward);
        pwm.set_duty(ABS(duty));
    };


    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
            rpc::make_function("dty", [&](const real_t duty){set_duty(duty);}),
            rpc::make_function("led", [&](const bool l){set_led(l);})


        );

        repl_server.invoke(list);
    };

    [[maybe_unused]] auto blink_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(200ms);
        timer.invoke_if([&]{
            static BoolLevel last_state = LOW;
            set_led(last_state.to_bool());
            last_state = ~last_state;
        });
    };

    static constexpr auto CAN_ID_TURNON = 
        robots::comb_role_and_cmd<Command>(NodeRole::Laser,Command::On);

    static constexpr auto CAN_ID_TURNOFF = 
        robots::comb_role_and_cmd<Command>(NodeRole::Laser,Command::Off);


    [[maybe_unused]] auto can_service = [&]{
        if(can.available() == 0){
            // DEBUG_PRINTLN("no msg", can.get_last_fault());
            return;
        }
        const auto msg = can.read();
        DEBUG_PRINTLN(msg);
        if(not msg.is_standard()) return;
        const auto id = msg.stdid().unwrap();

        switch(id.to_u11()){
            case CAN_ID_TURNON.to_u11():
                set_duty(0.99_r);
                break;
            case CAN_ID_TURNOFF.to_u11():
                set_duty(0.0_r);
                break;
            default:

                break;
        }
    };


    set_duty(1);

    while(true){
        repl_service();
        blink_service();
        can_service();
        static auto timer = async::RepeatTimer::from_duration(5ms);
        timer.invoke_if([&]{
            // DEBUG_PRINTLN("laser_ctl_main", clock::time());
        });
    }
}
