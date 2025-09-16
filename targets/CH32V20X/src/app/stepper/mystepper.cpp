#include <atomic>
#include <array>


#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/data_iter.hpp"
#include "core/utils/release_info.hpp"
#include "core/utils/bitflag.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/build_date.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"
#include "hal/gpio/gpio.hpp"

#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"

#include "src/testbench/tb.h"

#include "types/regions/range2.hpp"

#include "meta_utils.hpp"
#include "tasks.hpp"
#include "ctrl.hpp"

#include "calibrate_utils.hpp"

#include "robots/repl/repl_service.hpp"
#include "digipw/prelude/abdq.hpp"
#include "digipw/pwmgen/stepper_pwmgen.hpp"
#include "core/utils/progress.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"
#include "dsp/motor_ctrl/position_corrector.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"



using namespace ymd;

#ifdef ENABLE_UART1

#define UART hal::uart1

using digipw::AlphaBetaCoord;



struct PreoperateTasks{

};




class MotorFibre{
public:
    using TaskError = BeepTasks::TaskError;
    MotorFibre(
        drivers::EncoderIntf & encoder,
        digipw::StepperPwmGen & pwm_gen_
    ):
        encoder_(encoder),
        svpwm_(pwm_gen_)
        {;}

    struct Error{
        Error(CoilMotionCheckTasks::TaskError err){
            PANIC{err};
        }

        Error(drivers::EncoderError err){
            PANIC{err};
        }

        friend OutputStream & operator<<(OutputStream & os, const Error & err){ 
            return os;
        }
    };

    Result<void, Error> resume(){
        const auto begin_u = clock::micros();

        const auto meas_lap_position = ({
            if(const auto res = retry(2, [&]{return encoder_.update();});
                res.is_err()) return Err(Error(res.unwrap_err()));
            // execution_time_ = clock::micros() - begin_u;
            const auto either_lap_position = encoder_.read_lap_angle();
            if(either_lap_position.is_err())
                return Err(Error(either_lap_position.unwrap_err()));
            Angle<q31>::from_turns(1 - either_lap_position.unwrap().to_turns());
        });

        auto & subprogress = calibrate_tasks_;
        // if(const auto may_err = subprogress.err(); may_err.is_some()){
        #if 0
            constexpr const auto TASK_COUNT = 
                std::decay_t<decltype(subprogress)>::TASK_COUNT;
            const auto idx = subprogress.task_index();
            [&]<auto... Is>(std::index_sequence<Is...>) {
                DEBUG_PRINTLN((std::move(subprogress.get_task<Is>().dignosis()).err)...);
            }(std::make_index_sequence<TASK_COUNT>{});

            const auto diagnosis = [&]<auto ...Is>(std::index_sequence<Is...>) {
                return (( (Is == idx) ? 
                (subprogress.get_task<Is>().dignosis(), 0u) : 
                0u), ...);
            }(std::make_index_sequence<TASK_COUNT>{});

            static_assert(std::is_integral_v<
                std::decay_t<decltype(diagnosis)>>, "can't find task");

            DEBUG_PRINTLN("Error occuared when executing\r\n", 
                "detailed infomation:", Reflecter::display(diagnosis));

            const auto res = TaskExecuter::execute(TaskSpawner::spawn(
                CalibrateTasks{pos_filter_.forward_cali_vec, 
                    pos_filter_.backward_cali_vec}
                >>= PreoperateTasks{}
            )).inspect_err([](const TaskError err){
                MATCH{err}(
                    TaskError::CoilCantMove, 
                        []{PANIC("check motor wire connection before restart");},
                    TaskError::RotorIsMovingBeforeChecking, 
                        []{PANIC("please keep rotor of motor still before calibrate");},
                    None, 
                        []{
                            hal::PROGRAM_FAULT_LED = HIGH;
                            TODO("helper is not done yet");
                            sys::abort();
                        }
                );
            });
            return Err(res.unwrap_err());
        #endif
        if(subprogress.is_finished()){
            is_subprogress_finished_ = true;
            execution_time_ = measure_total_elapsed_us([&]{                
                ctrl(meas_lap_position);
            });

            return Ok();
        }

        is_subprogress_finished_ = false;


        const auto [a,b] = subprogress.resume(
            Angle<q16>::from_turns(meas_lap_position.to_turns()));

        svpwm_.set_dutycycle({a,b});
        return Ok();
    }

    bool is_subprogress_finished() const{
        return is_subprogress_finished_;
    }


    void ctrl(Angle<q31> meas_lap_position){

        pos_filter_.update(meas_lap_position);
        // const auto [a,b] = sincospu(frac(meas_lap_position - 0.009_r) * 50);
        // const auto [s,c] = sincospu(frac(-(meas_lap_position - 0.019_r + 0.01_r)) * 50);
        
        // const auto input_targ_position = 16 * sin(ctime);
        // const auto targ_speed = 6 * cos(6 * ctime);
        
        // const auto input_targ_position = 10 * real_t(int(ctime));
        const auto ctime = clock::time();
        // const auto input_targ_position = 5 * sin(ctime/2);

        // command_shaper_.update(input_targ_position);
        // auto [targ_position, targ_speed] = std::make_tuple(
        //     command_shaper_.update(input_targ_position),
        //     command_shaper_.speed()
        // );

        const auto omega = 1.0_r * real_t(TAU);
        // const auto omega = 1.0_r;
        // const auto amp = 0.5_r;
        const auto amp = 0.05_r;
        const auto [targ_position, targ_speed] = std::make_tuple<q16, q16>(
            amp * sin(ctime * omega) + 9, omega * amp * cos(ctime * omega)
            // int(ctime * omega), 0
        );
        const auto meas_position = pos_filter_.position();
        const auto meas_speed = pos_filter_.speed();
        
        static constexpr dsp::PdCtrlLaw pd_ctrl_law{
            8.8_r, 0.239_r
        };
        
        const auto curr = CLAMP2(pd_ctrl_law(
            targ_position - meas_position,
            targ_speed - meas_speed
        ), 0.5_r);
        
        // const auto mag = 0.5_r * sinpu(ctime);
        const auto mag = ABS(curr);
        const auto tangles = (1 + MIN(ABS(meas_speed) * real_t(1.0 / 40), 0.15_r));

        const auto [s,c] = sincospu(
        (pos_filter_.lap_position() + SIGN_AS(0.005_r * tangles, curr)) * 50);
        
        svpwm_.set_dutycycle({c * mag,s * mag});


    }

    Result<void, void> print_vec() const {
        [[maybe_unused]] auto print_view = [](auto view){
            for (const auto & item : view) {
                const auto expected = item.expected();
                const auto measured = item.measured();
                const auto inacc = item.to_inaccuracy();
                DEBUG_PRINTLN(expected, measured, inacc * 1000);
                clock::delay(1ms);
            }
        };

        // print_view(forward_cali_table_.iter());
        // print_view(backward_cali_table_.iter());



        for(size_t i = 0; i < 50; i++){
            const auto raw = real_t(i) / 50;
            const auto corrected = corrector_.correct_raw_position(raw);
            DEBUG_PRINTLN(raw, corrected, (corrected - raw) * 1000);
            clock::delay(1ms);
        }

        return Ok();
    }


    Microseconds execution_time_ = 0us;
// private:
public:
    using CommandShaper = dsp::CommandShaper1;
    using CommandShaperConfig = CommandShaper::Config;

    drivers::EncoderIntf & encoder_;
    digipw::StepperPwmGen & svpwm_;

    CoilMotionCheckTasks coil_motion_check_comp_{};

    dsp::CalibrateTable forward_cali_table_{50};
    dsp::CalibrateTable backward_cali_table_{50};

    dsp::PositionCorrector corrector_{{
        .forward_cali_table = forward_cali_table_, 
        .backward_cali_table = backward_cali_table_
    }};


    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = ISR_FREQ,
            .r = 120
        }
    };

    bool is_subprogress_finished_ = false;

    static constexpr real_t MC_TAU = 80;


    static constexpr CommandShaperConfig CS_CONFIG{
            .kp = MC_TAU * MC_TAU,
            .kd = 2 * MC_TAU,
            .max_spd = 30.0_r,
            .max_acc = 140.0_r,
            .fs = ISR_FREQ
    };

    CommandShaper command_shaper_{CS_CONFIG};


    CalibrateTasks calibrate_tasks_{
        forward_cali_table_,
        backward_cali_table_
    };
};


[[maybe_unused]] 
static void motorcheck_tb(drivers::EncoderIntf & encoder,digipw::StepperPwmGen & pwm_gen_){
    DEBUGGER.no_brackets(EN);

    auto motor_system_ = MotorFibre{encoder, pwm_gen_};

    hal::timer1.attach(
        hal::TimerIT::Update, 
        {0,0}, 
        [&](){
            motor_system_.resume().examine();
        }, 
        EN
    );

    robots::ReplServer repl_server = {
        &UART, &UART
    };

    auto list = rpc::make_list(
        "list",
        rpc::make_function("rst", [](){sys::reset();}),
        rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
        rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
        rpc::make_function("now", [&](){return clock::time();})
    );

    while(true){
        // encoder.update();

        // clock::delay(1ms);
        // DEBUG_PRINTLN(
        //     clock::millis().count(), 
        //     subprogress.task_index(), 
        //     subprogress.is_finished(),
        //     subprogress.err().is_some()
        // );
        // DEBUG_PRINTLN(motor_system_.is_subprogress_finished());
        if(0){
            static bool printed = false;
            if(printed == false and motor_system_.is_subprogress_finished()){
                printed = true;
                motor_system_.print_vec().examine();
            }
        }
        // DEBUG_PRINTLN_IDLE(motor_system_.execution_time_.count());

        DEBUG_PRINTLN_IDLE(
            
            motor_system_.pos_filter_.position() * 10,
            motor_system_.pos_filter_.speed()
            // motor_system_.execution_time_.count(),
            // motor_system_.command_shaper_.get()
        );
        repl_server.invoke(list);
        clock::delay(1ms);
    }

    // DEBUG_PRINTLN("finished");

    while(true);
}

[[maybe_unused]] static void eeprom_tb(){
    auto scl_gpio = hal::PD<1>();
    auto sda_gpio = hal::PD<0>();
    hal::I2cSw i2c_sw{&scl_gpio, &sda_gpio};
    i2c_sw.init({800_KHz});
    drivers::AT24CXX at24{drivers::AT24CXX::Config::AT24C02{}, i2c_sw};

    const auto begin_u = clock::micros();
    const auto elapsed = measure_total_elapsed_us(
        [&]{
            uint8_t rdata[3] = {0};
            at24.load_bytes(0_addr, std::span(rdata)).examine();
            while(not at24.is_idle()){
                at24.poll().examine();
            }

            DEBUG_PRINTLN(rdata);
            const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
            at24.store_bytes(0_addr, std::span(data)).examine();

            while(not at24.is_idle()){
                at24.poll().examine();
            }
        }
    );
    DEBUG_PRINTLN("done", elapsed);
    while(true);
}


[[maybe_unused]] static void currentloop_tb(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    // DEBUG_PRINTLN(hash(.unwrap()));
    clock::delay(400ms);

    {
        hal::Gpio ena_gpio = hal::PB<0>();
        hal::Gpio enb_gpio = hal::PA<7>();
        ena_gpio.outpp(HIGH);
        enb_gpio.outpp(HIGH);
    }

    auto & timer = hal::timer1;

    timer.init({
        .freq = CHOP_FREQ,
        // .mode = hal::TimerCountMode::CenterAlignedDualTrig
        .mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    timer.enable_arr_sync(EN);
    timer.set_trgo_source(hal::TimerTrgoSource::Update);


    auto & pwm_ap = timer.oc<1>();
    auto & pwm_an = timer.oc<2>();
    auto & pwm_bp = timer.oc<3>();
    auto & pwm_bn = timer.oc<4>();

    pwm_ap.init({});
    pwm_an.init({});
    pwm_bp.init({});
    pwm_bn.init({});

    auto convert_pair_duty = [](const real_t duty) -> std::tuple<real_t, real_t>{
        if(duty > 0){
            return {1, 1-duty};
        }else{
            return {1 + duty, 1};
        }
    };

    auto set_alpha_beta_duty = [&](const real_t alpha, const real_t beta){
        {
            const auto [ap,an] = convert_pair_duty(alpha);
            pwm_ap.set_dutycycle(ap);
            pwm_an.set_dutycycle(an);
        }

        {
            const auto [bp,bn] = convert_pair_duty(beta);
            pwm_bp.set_dutycycle(bp);
            pwm_bn.set_dutycycle(bn);
        }
    };

    auto & adc = hal::adc1;
    adc.init(
        {
            {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelNth::CH3, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelNth::CH4, hal::AdcSampleCycles::T7_5},
        }, {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    adc.enable_auto_inject(DISEN);

    auto & inj_a = adc.inj<1>();
    auto & inj_b = adc.inj<2>();

    auto isr_trig_gpio = hal::PC<13>();
    isr_trig_gpio.outpp();

    real_t a_curr;
    real_t b_curr;
    adc.attach(
        hal::AdcIT::JEOC, 
        {0,0}, [&]{
            // isr_trig_gpio.toggle();
            // DEBUG_PRINTLN_IDLE(millis());
            // isr_trig_gpio.toggle();
            // static bool is_a = false;
            // b_curr = inj_b.get_voltage();
            // a_curr = inj_a.get_voltage();
        }, EN
    );


    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        b_curr = inj_b.get_voltage();
        a_curr = inj_a.get_voltage();
        const auto t = clock::time();
        const auto [s,c] = sincospu(t);
        constexpr auto mag = 0.4_r;
        set_alpha_beta_duty(
            c * mag,
            s * mag
        );
    }, EN);

    while(true){
        DEBUG_PRINTLN_IDLE(
            a_curr,
            b_curr,
            30 * (a_curr * a_curr + b_curr * b_curr)
        );
    }
}



void mystepper_main(){

    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    // DEBUG_PRINTLN(hash(.unwrap()));
    clock::delay(400ms);

    hal::Gpio ena_gpio = hal::PB<0>();
    hal::Gpio enb_gpio = hal::PA<7>();
    ena_gpio.outpp(HIGH);
    enb_gpio.outpp(HIGH);

    auto & timer = hal::timer1;

    timer.init({
        .freq = CHOP_FREQ,
        // .mode = hal::TimerCountMode::CenterAlignedDownTrig
        .mode = hal::TimerCountMode::CenterAlignedDualTrig
    }, EN);

    timer.enable_arr_sync(EN);
    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    digipw::StepperPwmGen pwm_gen_{
        timer.oc<1>(),
        timer.oc<2>(),
        timer.oc<3>(),
        timer.oc<4>(),
    };

    pwm_gen_.init_channels();

    auto & adc = hal::adc1;
    adc.init(
        {
            {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelNth::CH3, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelNth::CH4, hal::AdcSampleCycles::T7_5},
        },
        {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    adc.enable_auto_inject(DISEN);
    auto inj_a = hal::ScaledAnalogInput{adc.inj<1>(), Rescaler<q16>::from_scale(1)};
    auto inj_b = hal::ScaledAnalogInput{adc.inj<2>(), Rescaler<q16>::from_scale(1)};
    auto ma730_cs_gpio_ = hal::PA<15>();



    digipw::AlphaBetaCoord<q16> alpha_beta_curr = {0, 0};
    adc.attach(
        hal::AdcIT::JEOC, 
        {0,0}, 
        [&]{
            // static bool is_a = false;
            static bool is_a = true;
            is_a = !is_a;
            
            if(is_a){
                alpha_beta_curr.alpha = inj_a.get_value();

                // adc.init(
                //     {
                //         {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
                //     },{
                //         {hal::AdcChannelNth::CH3, hal::AdcSampleCycles::T7_5},
                //         {hal::AdcChannelNth::CH4, hal::AdcSampleCycles::T7_5},
                //     },
                //     {}
                // );
            }else{
                alpha_beta_curr.beta = inj_b.get_value();
                // adc.init(
                //     {
                //         {hal::AdcChannelNth::VREF, hal::AdcSampleCycles::T28_5}
                //     },{
                //         {hal::AdcChannelNth::CH4, hal::AdcSampleCycles::T7_5},
                //         {hal::AdcChannelNth::CH3, hal::AdcSampleCycles::T7_5},
                //     },
                //     {}
                // );
            }
        },
        EN
    );

    [[maybe_unused]] auto isr_trig_gpio = hal::PC<13>();
    [[maybe_unused]] auto PROGRAM_FAULT_LED = hal::PC<14>();

    isr_trig_gpio.outpp();

    auto & spi = hal::spi1;
    spi.init({18_MHz});

    drivers::MT6816 encoder{
        &spi, 
        spi.allocate_cs_gpio(&ma730_cs_gpio_).unwrap()
    };


    encoder.init({
        .fast_mode_en = DISEN
    }).examine();

    // motorcheck_tb(encoder, pwm_gen_);
    // eeprom_tb();

    hal::timer1.attach(
        hal::TimerIT::Update, 
        {0,0}, 
        [&](){
            [[maybe_unused]] const auto ctime = clock::time();
            const auto [s,c] = sincospu(10 * ctime);
            constexpr auto mag = 0.3_r;
            pwm_gen_.set_dutycycle({
                c * mag,
                s * mag
            });
            // pwm_gen_.set_alpha_beta_duty(
            //     mag,
            //     mag
            // );
        },
        EN
    );

    while(true){
        DEBUG_PRINTLN_IDLE(
            alpha_beta_curr
            // 30 * alpha_beta_curr.length_squared(),
            // ADC1->IDATAR2
        );
    }
}

#endif