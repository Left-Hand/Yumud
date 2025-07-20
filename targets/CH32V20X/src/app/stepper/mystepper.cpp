#include <atomic>
#include <array>


#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
// #include "core/fp/matchit.hpp"
#include "core/utils/bitflag.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/build_date.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"
#include "hal/gpio/gpio.hpp"

#include "drivers/Encoder/odometer.hpp"
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"

#include "src/testbench/tb.h"

#include "types/regions/range2/range2.hpp"

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
#include "core/utils/data_iter.hpp"
#include "core/utils/release_info.hpp"

using namespace ymd;

#ifdef ENABLE_UART1
#define let const auto

#define UART hal::uart1

using digipw::AlphaBetaDuty;



struct PreoperateTasks{

};




class MotorFibre{
public:
    using TaskError = BeepTasks::TaskError;
    MotorFibre(
        drivers::EncoderIntf & encoder,
        digipw::StepperSVPWM & svpwm
    ):
        encoder_(encoder),
        svpwm_(svpwm)
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
        let begin_u = clock::micros();

        let meas_lap_position = ({
            if(let res = retry(2, [&]{return encoder_.update();});
                res.is_err()) return Err(Error(res.unwrap_err()));
            // execution_time_ = clock::micros() - begin_u;
            let either_lap_position = encoder_.read_lap_position();
            if(either_lap_position.is_err())
                return Err(Error(either_lap_position.unwrap_err()));
            1 - either_lap_position.unwrap();
        });

        auto & comp = calibrate_comp_;
        // if(let may_err = comp.err(); may_err.is_some()){
        #if 0
            constexpr let TASK_COUNT = 
                std::decay_t<decltype(comp)>::TASK_COUNT;
            let idx = comp.task_index();
            [&]<auto... Is>(std::index_sequence<Is...>) {
                DEBUG_PRINTLN((std::move(comp.get_task<Is>().dignosis()).err)...);
            }(std::make_index_sequence<TASK_COUNT>{});

            let diagnosis = [&]<auto ...Is>(std::index_sequence<Is...>) {
                return (( (Is == idx) ? 
                (comp.get_task<Is>().dignosis(), 0u) : 
                0u), ...);
            }(std::make_index_sequence<TASK_COUNT>{});

            static_assert(std::is_integral_v<
                std::decay_t<decltype(diagnosis)>>, "can't find task");

            DEBUG_PRINTLN("Error occuared when executing\r\n", 
                "detailed infomation:", Reflecter::display(diagnosis));

            let res = TaskExecuter::execute(TaskSpawner::spawn(
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
        if(comp.is_finished()){

            is_comp_finished_ = true;
            pos_filter_.update(meas_lap_position);
            // let [a,b] = sincospu(frac(meas_lap_position - 0.009_r) * 50);
            // let [s,c] = sincospu(frac(-(meas_lap_position - 0.019_r + 0.01_r)) * 50);
            let t = clock::time();

            let input_targ_position = 16 * sin(t);
            // let targ_speed = 6 * cos(6 * t);
            
            // let input_targ_position = 10 * real_t(int(t));
            // let input_targ_position = 5 * frac(t/2);

            cs_.update(input_targ_position);
            auto [targ_position, targ_speed] = cs_.get();
            // static constexpr let SCALE = (6.0_r/9.8_r);
            // targ_speed*= SCALE;
            let meas_position = pos_filter_.position();
            let meas_speed = pos_filter_.speed();
            let pos_contribute = 0.8_r * (targ_position - meas_position);
            let speed_contribute = 0.039_r*(targ_speed - meas_speed);
            let curr = CLAMP2(pos_contribute + speed_contribute, 0.4_r);
            // let curr = 0.2_r;
            let [s,c] = sincospu(frac(
                // (correct_raw_position(meas_lap_position) - 0.007_r)) * 50);
                (pos_filter_.lap_position() + SIGN_AS(0.007_r, curr))) * 50);
            // let [a,b] = sincospu( - 0.004_r);
            // let mag = 0.5_r;
            let mag = ABS(curr);

            svpwm_.set_alpha_beta_duty(c * mag,s * mag);
            execution_time_ = clock::micros() - begin_u;

            return Ok();
        }

        is_comp_finished_ = false;


        let [a,b] = comp.resume(meas_lap_position);
        svpwm_.set_alpha_beta_duty(a,b);
        return Ok();
    }

    bool is_comp_finished() const{
        return is_comp_finished_;
    }


    Result<void, void> print_vec() const {
        auto print_view = [](auto view){
            for (let & item : view) {
                let expected = item.expected();
                let measured = item.measured();
                // DEBUG_PRINTLN(expected, measured, fposmod(q20(expected - measured), 0.02_q20) * 100);
                let position_err = q20(expected - measured);
                let mod_err = fposmod(position_err, 0.02_q20);
                DEBUG_PRINTLN(expected, measured, mod_err * 100);
                clock::delay(1ms);
            }
        };

        print_view(forward_cali_table_.iter());
        print_view(backward_cali_table_.iter());

        auto corrector = dsp::PositionCorrector{{
            .forward_cali_table = forward_cali_table_, 
            .backward_cali_table = backward_cali_table_
        }};

        for(size_t i = 0; i < 50; i++){
            let raw = real_t(i) / 50;
            let corrected = corrector.correct_raw_position(raw);
            DEBUG_PRINTLN(raw, corrected, (corrected - raw) * 100);
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
    digipw::StepperSVPWM & svpwm_;

    CoilMotionCheckTasks coil_motion_check_comp_{};

    dsp::CalibrateTable forward_cali_table_{50};
    dsp::CalibrateTable backward_cali_table_{50};

    CalibrateTasks calibrate_comp_ = {
        forward_cali_table_,
        backward_cali_table_
    };


    bool is_comp_finished_ = false;

    static constexpr real_t MC_TAU = 80;


    static constexpr CommandShaperConfig CS_CONFIG{
            .kp = MC_TAU * MC_TAU,
            .kd = 2 * MC_TAU,
            .max_spd = 30.0_r,
            .max_acc = 140.0_r,
            .fs = ISR_FREQ
    };

    CommandShaper cs_{CS_CONFIG};

    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .r = 50,
            .fs = ISR_FREQ
        }
    };
};

#if 0
template<typename T>
struct Context{
    static_assert(std::is_copy_assignable_v<T>);

    ReleaseInfo release_info;
    T obj;

    template<HashAlgo S>
    friend Hasher<S> & operator << (Hasher<S> & hs, const Context & self){
        return hs << self.release_info;
    }
};

template<typename T>
struct Bin{
    HashCode hashcode;
    Context<T> context;

    constexpr HashCode calc_hash_of_context() const{
        return hash(context);
    } 

    constexpr bool is_verify_passed(){
        return calc_hash_of_context() == hashcode;
    } 
    constexpr const Context<T> * operator ->() const {
        return context;
    }

    constexpr size_t size() const {
        return sizeof(*this);
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>{
            reinterpret_cast<const uint8_t *>(this), size()};
    }
};

#endif


namespace archive{
static constexpr size_t STORAGE_MAX_SIZE = 256;
static constexpr size_t HEADER_MAX_SIZE = 32;
static constexpr size_t CONTEXT_PLACE_OFFSET = HEADER_MAX_SIZE;
static constexpr size_t CONTEXT_MAX_SIZE = STORAGE_MAX_SIZE - CONTEXT_PLACE_OFFSET;

using Bin = std::array<uint8_t, STORAGE_MAX_SIZE>;

struct Header{
    HashCode hashcode;
    ReleaseInfo release_info;

    constexpr size_t size() const {
        return sizeof(*this);
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>{
            reinterpret_cast<const uint8_t *>(this), size()};
    }
};

static_assert(sizeof(Header) <= HEADER_MAX_SIZE);

template<typename Iter>
static constexpr auto generate_header(Iter && iter) -> Header{ 
    return Header{
        .hashcode = hash(iter),
        .release_info = ReleaseInfo::from("Rstr1aN", ReleaseVersion{0,1})
    };
}

struct Context{ 
};

static constexpr Bin generate_bin(const Header & header, const Context & context){
    Bin bin;

    return bin;
};
}

#if 0
class MotorArchiveSystem{
    MotorArchiveSystem(drivers::AT24CXX at24):
        at24_(at24){;}



    enum class State:uint8_t{
        Idle,
        Saving,
        Loading,
        Verifying
    };


    static_assert(sizeof(archive::Header) <= HEADER_MAX_SIZE);

    template<typename T>
    auto save(T & obj){
        const auto body_bytes = obj.as_bytes();
        const auto header = archive::generate_header(body_bytes);
        auto assign_header_and_obj_to_buf = [&]{
            std::copy(buf_.begin(), buf_.end(), header.as_bytes());
            std::copy(buf_.begin() + BODY_OFFSET, buf_.end(), obj.as_bytes());
        };

        assign_header_and_obj_to_buf();

        return at24_.store_bytes(Address(BODY_OFFSET), buf_);
    }

    template<typename T>
    auto load(T & obj){
        // const auto body_bytes = obj.as_bytes();
        // const Header header = {
        //     .hashcode = hash(body_bytes),
        //     .release_info = ReleaseInfo::from("Rstr1aN", {0,1})
        // };

        // auto assign_header_and_obj_to_buf = [&]{
        //     std::copy(buf_.begin(), buf_.end(), header.as_bytes());
        //     std::copy(buf_.begin() + BODY_OFFSET, buf_.end(), obj.as_bytes());
        // };

        // assign_header_and_obj_to_buf();

        // return at24_.store_bytes(Address(BODY_OFFSET), buf_);
    }

    auto poll(){
        if(not at24_.is_idle()){
            at24_.poll().examine();
        }
    }

    bool is_idle(){
        return at24_.is_idle();
    }

    Progress progress(){
        return {0,0};
    }
private:
    std::atomic<State> state_ = State::Idle;

    drivers::AT24CXX & at24_;
    std::array<uint8_t, STORAGE_MAX_SIZE> buf_;
};
#endif


    // void save(std::span<const uint8_t>){
        // at24.load_bytes(0_addr, std::span(rdata)).examine();
        // while(not at24.is_idle()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN(rdata);
        // const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
        // at24.store_bytes(0_addr, std::span(data)).examine();

        // while(not at24.is_idle()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN("done", clock::micros() - begin_u);
        // while(true);
    // }

[[maybe_unused]] 
static void motorcheck_tb(drivers::EncoderIntf & encoder,digipw::StepperSVPWM & svpwm){
    DEBUGGER.no_brackets();

    auto motor_system_ = MotorFibre{encoder, svpwm};

    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        motor_system_.resume().examine();
    });

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
        //     comp.task_index(), 
        //     comp.is_finished(),
        //     comp.err().is_some()
        // );
        // DEBUG_PRINTLN(motor_system_.is_comp_finished());
        // if(motor_system_.is_comp_finished()){
        //     motor_system_.print_vec().examine();
        //     break;
        // }
        // DEBUG_PRINTLN_IDLE(motor_system_.execution_time_.count());

        // DEBUG_PRINTLN_IDLE(
            
        //     motor_system_.pos_filter_.position(),
        //     motor_system_.pos_filter_.speed(),
        //     motor_system_.execution_time_.count(),
        //     motor_system_.cs_.get()
        // );
        repl_server.invoke(list);
        clock::delay(1ms);
    }

    // DEBUG_PRINTLN("finished");

    while(true);
}

template<typename Fn>
static Milliseconds measure_elapsed_ms(Fn && fn){
    const Milliseconds start = clock::millis();
    std::forward<Fn>(fn)();
    return clock::millis() - start;
}


template<typename Fn>
static Microseconds measure_elapsed_us(Fn && fn){
    const Microseconds start = clock::micros();
    std::forward<Fn>(fn)();
    return clock::micros() - start;
}


[[maybe_unused]] static void eeprom_tb(){
    hal::I2cSw i2c_sw{&hal::portD[1], &hal::portD[0]};
    i2c_sw.init(800_KHz);
    drivers::AT24CXX at24{drivers::AT24CXX::Config::AT24C02{}, i2c_sw};

    const auto begin_u = clock::micros();
    const auto elapsed = measure_elapsed_us(
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
        hal::Gpio & ena_gpio = hal::portB[0];
        hal::Gpio & enb_gpio = hal::portA[7];
        ena_gpio.outpp(HIGH);
        enb_gpio.outpp(HIGH);
    }

    auto & timer = hal::timer1;

    timer.init({
        .freq = CHOP_FREQ,
        // .mode = hal::TimerCountMode::CenterAlignedDualTrig
        .mode = hal::TimerCountMode::CenterAlignedUpTrig
    });

    timer.enable_arr_sync();
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
            pwm_ap.set_duty(ap);
            pwm_an.set_duty(an);
        }

        {
            const auto [bp,bn] = convert_pair_duty(beta);
            pwm_bp.set_duty(bp);
            pwm_bn.set_duty(bn);
        }
    };

    auto & adc = hal::adc1;
    adc.init(
        {
            {hal::AdcChannelIndex::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelIndex::CH4, hal::AdcSampleCycles::T7_5},
        }, {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    adc.enable_auto_inject(DISEN);

    auto & inj_a = adc.inj<1>();
    auto & inj_b = adc.inj<2>();

    auto & isr_trig_gpio = hal::PC<13>();
    isr_trig_gpio.outpp();

    real_t a_curr;
    real_t b_curr;
    adc.attach(hal::AdcIT::JEOC, {0,0}, [&]{
        // isr_trig_gpio.toggle();
        // DEBUG_PRINTLN_IDLE(millis());
        // isr_trig_gpio.toggle();
        // static bool is_a = false;
        // b_curr = inj_b.get_voltage();
        // a_curr = inj_a.get_voltage();
    });


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
    });

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
    // DEBUG_PRINTLN(hash(.unwrap()));
    clock::delay(400ms);

    // currentloop_tb();

    {
        hal::Gpio & ena_gpio = hal::portB[0];
        hal::Gpio & enb_gpio = hal::portA[7];
        ena_gpio.outpp(HIGH);
        enb_gpio.outpp(HIGH);
    }

    auto & timer = hal::timer1;

    timer.init({
        .freq = CHOP_FREQ,
        .mode = hal::TimerCountMode::CenterAlignedDownTrig
    });

    timer.enable_arr_sync();
    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    digipw::StepperSVPWM svpwm{
        timer.oc<1>(),
        timer.oc<2>(),
        timer.oc<3>(),
        timer.oc<4>(),
    };

    svpwm.init_channels();

    auto & adc = hal::adc1;
    adc.init(
        {
            {hal::AdcChannelIndex::VREF, hal::AdcSampleCycles::T28_5}
        },{
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T7_5},
            {hal::AdcChannelIndex::CH4, hal::AdcSampleCycles::T7_5},
        },
        {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1TRGO);
    adc.enable_auto_inject(DISEN);

    auto & inj_a = adc.inj<1>();
    auto & inj_b = adc.inj<2>();

    [[maybe_unused]] auto & isr_trig_gpio = hal::PC<13>();
    [[maybe_unused]] auto & PROGRAM_FAULT_LED = hal::PC<14>();

    isr_trig_gpio.outpp();

    real_t a_curr;
    real_t b_curr;
    adc.attach(hal::AdcIT::JEOC, {0,0}, [&]{
        static bool is_a = false;
        is_a = !is_a;
        
        if(is_a){
            b_curr = inj_b.get_voltage();
        }else{
            a_curr = inj_a.get_voltage();
        }
    });


    auto & spi = hal::spi1;
    spi.init({18_MHz});

    drivers::MT6816 encoder{
        &spi, 
        spi.allocate_cs_gpio(&hal::portA[15]).unwrap()
    };


    encoder.init({
        .fast_mode_en = DISEN
    }).examine();

    motorcheck_tb(encoder, svpwm);
    // eeprom_tb();

    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        const auto t = clock::time();
        const auto [s,c] = sincospu(t);
        constexpr auto mag = 0.4_r;
        svpwm.set_alpha_beta_duty(
            c * mag,
            s * mag
        );
        // svpwm.set_alpha_beta_duty(
        //     mag,
        //     mag
        // );
    });

    while(true){
        DEBUG_PRINTLN_IDLE(
            a_curr,
            b_curr,
            30 * (a_curr * a_curr + b_curr * b_curr)
        );
    }
}

#endif