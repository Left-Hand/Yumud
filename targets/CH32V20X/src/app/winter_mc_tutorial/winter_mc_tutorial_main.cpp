#include "src/testbench/tb.h"

#include <atomic>

#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/sumtype.hpp"
#include "core/clock/time.hpp"
#include "core/string/conv/strconv2.hpp"
#include "core/utils/combo_counter.hpp"
#include "core/utils/default.hpp"
#include "core/async/delayed_semphr.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "algebra/vectors/quat.hpp"

#include "robots/gesture/comp_est.hpp"
#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/sensored/calibrate_table.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/motor_ctrl/elecrad_compsator.hpp"
#include "dsp/controller/adrc/linear/leso2o.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"

using namespace ymd;

//电机控制频率
static constexpr size_t MC_FREQ = 500;

//限制最大的占空比
static constexpr auto PWM_DUTYCYCLE_LIMIT = 0.85_iq16;

//限制占空比每次递增的大小
static constexpr auto PWM_DUTYCYCLE_DELTA_LIMIT = 6.2_iq16 / MC_FREQ;

//这里我测出来它每圈产生900个计数
static constexpr size_t CNT_PER_TURN = 900;

static constexpr iq16 rotor_cnt_to_position(const int32_t cnt){
    constexpr uint64_t FACTOR = (1ull << (32 + 16)) / CNT_PER_TURN;
    return iq16::from_bits(static_cast<int32_t>((static_cast<int64_t>(cnt) * FACTOR) >> 32));
};

void winter_mc_tutorial_main(){
    auto & DEBUG_UART = hal::usart2;

    auto bsp_init_debug = [&]{
        DEBUG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(576000),
            .tx_strategy = CommStrategy::Blocking
        });

        DEBUGGER.retarget(&DEBUG_UART);
        DEBUGGER.no_brackets(EN);
        DEBUGGER.set_eps(3);
        DEBUGGER.force_sync(EN);
        // DEBUGGER.no_fieldname(EN);
        DEBUGGER.no_fieldname(DISEN);
    };

    bsp_init_debug();


    auto & encoder_timer = hal::timer3;

    encoder_timer.init_as_encoder();
    encoder_timer.set_remap(hal::TIM3_REMAP_A6_A7_B0_B1);


    auto & pwmgen_timer = hal::timer1;

    pwmgen_timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        .count_freq = hal::NearestFreq(20_KHz),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().alter_to_pins({
        hal::TimerChannelSelection::CH1,
        hal::TimerChannelSelection::CH2,
    }).unwrap();

    auto & pwm_out = pwmgen_timer.oc<1>();
    pwm_out.init(Default);
    pwm_out.set_dutycycle(0.3_uq16);
    pwmgen_timer.start();

    auto forward_dir_pin = hal::PB<4>();
    auto backward_dir_pin = hal::PB<5>();

    forward_dir_pin.set_mode(hal::GpioMode::OutPP);
    backward_dir_pin.set_mode(hal::GpioMode::OutPP);
    
    forward_dir_pin.set_high();
    backward_dir_pin.set_low();

    auto & isr_timer = hal::timer2;

    isr_timer.init({
        .remap = hal::TIM2_REMAP_A15_B3_A2_A3,
        .count_freq = hal::NearestFreq(MC_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();

    isr_timer.register_nvic<hal::TimerIT::Update>({0, 0}, EN);
    isr_timer.enable_interrupt<hal::TimerIT::Update>(EN);


    auto set_pwm_dutycycle = [&](const iq16 dutycycle) -> void{
        if(dutycycle > 0){
            backward_dir_pin.set_low();
            forward_dir_pin.set_high();
            pwm_out.set_dutycycle(uq16(dutycycle));
        }else{
            forward_dir_pin.set_low();
            backward_dir_pin.set_high();
            pwm_out.set_dutycycle(uq16(-dutycycle));
        }
    };

    auto get_encoder_cnt = [&] -> uint16_t{
        return static_cast<uint16_t>(encoder_timer.cnt());
    };

    auto get_rotor_cnt = [&](const uint16_t this_cnt) -> int32_t{
        constexpr int32_t CYCLIC_CNT = 65536;
        static uint16_t last_cnt = 0;
        static int32_t accumulated_cnt = 0;

        int32_t delta_cnt = static_cast<int32_t>(this_cnt) - static_cast<int32_t>(last_cnt);

        if(delta_cnt > CYCLIC_CNT / 2){
            delta_cnt -= CYCLIC_CNT;
        }else if(delta_cnt < (-CYCLIC_CNT / 2)){
            delta_cnt += CYCLIC_CNT;
        }
        last_cnt = this_cnt;

        accumulated_cnt += delta_cnt;
        return accumulated_cnt;
    };

    using ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;
    using state2o = dsp::SecondOrderState<iq16>;
    auto rotor_ltd = ltd2o{ltd2o::Config{.fs = MC_FREQ, .r = 80}.try_into_coeffs().unwrap()};

    state2o meas_rotor_state_var = {0, 0};
    iq16 meas_rotor_x1 = 0;
    iq16 meas_rotor_x2 = 0;

    iq16 target_rotor_x1 = 0;
    iq16 target_rotor_x2 = 0;
    iq16 kp = 4.0_iq16;
    iq16 kd = 0.22_iq16;
    iq16 kf = 0.225_iq16;

    iq16 ramp_speed = 2;
    iq16 pwm_dutycycle = 0;

    enum class DemoManipulateSource:uint8_t{
        None = 0,
        Ramp = 1,
        Sine = 2,
        Square = 3,
    };

    DemoManipulateSource demo_manipulate_source = DemoManipulateSource::None;

    repl::ReplServer repl_server = {
        &DEBUG_UART, &DEBUG_UART
    };

    repl_server.set_outen(DISEN);

    auto repl_list = script::make_list( "list",
        script::make_function("rst", [](){sys::reset();}),
        script::make_function("outen", [&](){repl_server.set_outen(EN);}),
        script::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
        script::make_mut_property("x1", &target_rotor_x1),
        script::make_mut_property("x2", &target_rotor_x2),
        script::make_mut_property("kp", &kp),
        script::make_mut_property("kd", &kd),
        script::make_mut_property("kf", &kf),
        script::make_mut_property("rs", &ramp_speed),
        script::make_mut_property("dm", reinterpret_cast<uint8_t *>(&demo_manipulate_source))
    );

    auto motor_isr = [&]{
        //获取当前的时间
        [[maybe_unused]] const auto now_secs = clock::seconds();

        switch(demo_manipulate_source){
            case DemoManipulateSource::None:{
                break;
            }

            case DemoManipulateSource::Ramp:{
                target_rotor_x1 += target_rotor_x2 * uq32::from_rcp(MC_FREQ);
                target_rotor_x2 = ramp_speed;
                break;
            }

            case DemoManipulateSource::Sine:{
                target_rotor_x1 = 3.0_iq16 * math::sin(now_secs * 1.0_iq16);
                target_rotor_x2 = 3.0_iq16 * math::cos(now_secs * 1.0_iq16);
                break;
            }

            case DemoManipulateSource::Square:{
                target_rotor_x1 = math::sin(now_secs * 2.0_iq16) > 0 ? 2 : -2;
                target_rotor_x2 = 0;
            }
        }

        //#region 更新电机运动状态变量
        const auto meas_rotor_cnt = get_rotor_cnt(get_encoder_cnt());
        const auto meas_rotor_x1_unfilted = rotor_cnt_to_position(meas_rotor_cnt);

        meas_rotor_state_var = rotor_ltd.iterate(meas_rotor_state_var, {meas_rotor_x1_unfilted, 0});
        meas_rotor_x1 = math::fixed_downcast<16>(meas_rotor_state_var.x1);
        meas_rotor_x2 = meas_rotor_state_var.x2;
        //#endregion

        //计算位置误
        const auto e1 = target_rotor_x1 - meas_rotor_x1;

        //计算速度误差
        const auto e2 = target_rotor_x2 - meas_rotor_x2;

        //计算期望的占空比
        const auto desired_pwm_dutycycle = 
            (e1 * kp) + 
            (e2 * kd) + 
            (target_rotor_x2 * kf);

        //计算钳位与限速后更新的占空比
        pwm_dutycycle = STEP_TO(
            pwm_dutycycle, 
            CLAMP2(desired_pwm_dutycycle, PWM_DUTYCYCLE_LIMIT), 
            PWM_DUTYCYCLE_DELTA_LIMIT);

        //将占空比应用到输出
        set_pwm_dutycycle(pwm_dutycycle);
    };

    isr_timer.set_event_callback([&](const hal::TimerEvent ev){
        switch(ev){
            case hal::TimerEvent::Update:
                motor_isr();
                break;
            default:
                break;
        };
    });


    isr_timer.start();

    while(true){
        repl_server.invoke(repl_list);


        DEBUG_PRINTLN(
            target_rotor_x1,
            target_rotor_x2,
            meas_rotor_x1,
            meas_rotor_x2,
            pwm_dutycycle
        );

        clock::delay(1ms);
    }
}