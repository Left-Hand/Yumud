#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
// #include "core/fp/matchit.hpp"

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

#include "src/testbench/tb.h"

#include "types/regions/range2/range2.hpp"
#include "bitflag.hpp"


#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"
#include "core/string/StringView.hpp"
#include "build_date.hpp"
#include "meta_utils.hpp"
#include "tasks.hpp"
#include "ctrl.hpp"

#include "calibrate_utils.hpp"
#include <atomic>


using namespace ymd;


#define let const auto

#define UART hal::uart1


template<typename ... Args>
struct PhantomMarker {
    // Default constructor
    constexpr PhantomMarker() noexcept = default;

    // Copy/move constructors and assignments (defaulted)
    constexpr PhantomMarker(const PhantomMarker&) noexcept = default;
    constexpr PhantomMarker(PhantomMarker&&) noexcept = default;
    PhantomMarker& operator=(const PhantomMarker&) noexcept = default;
    PhantomMarker& operator=(PhantomMarker&&) noexcept = default;

    // No members - empty class
};

struct AlphaBetaDuty{
    q16 alpha;
    q16 beta;

    q16 & operator [](size_t idx){
        switch(idx){
            case 0: return alpha;
            case 1: return beta;
            default: __builtin_unreachable();
        }
    }


    const q16 & operator [](size_t idx) const{
        switch(idx){
            case 0: return alpha;
            case 1: return beta;
            default: __builtin_unreachable();
        }
    }
};


//AT8222
class StepperSVPWM{
public:
    StepperSVPWM(
        hal::TimerOC & pwm_ap,
        hal::TimerOC & pwm_an,
        hal::TimerOC & pwm_bp,
        hal::TimerOC & pwm_bn
    ):
        channel_a_(pwm_ap, pwm_an),
        channel_b_(pwm_bp, pwm_bn)
    {;}

    void init_channels(){
        // oc.init({.valid_level = LOW});

        channel_a_.inverse(EN);
        channel_b_.inverse(DISEN);

        static constexpr hal::TimerOcPwmConfig pwm_noinv_cfg = {
            .cvr_sync_en = EN,
            .valid_level = HIGH
        };

        static constexpr hal::TimerOcPwmConfig pwm_inv_cfg = {
            .cvr_sync_en = EN,
            .valid_level = LOW,
        };
        
        channel_a_.pos_channel().init(pwm_noinv_cfg);
        channel_a_.neg_channel().init(pwm_noinv_cfg);
        channel_b_.pos_channel().init(pwm_inv_cfg);
        channel_b_.neg_channel().init(pwm_inv_cfg);

    }

    void set_alpha_beta_duty(const real_t duty_a, const real_t duty_b){
        channel_a_.set_duty(duty_a);
        channel_b_.set_duty(duty_b);
    }

    // static constexpr std::tuple<q16, q16> map_duty_to_pair(const q16 duty){

    // }
private:

    hal::TimerOcPair channel_a_;
    hal::TimerOcPair channel_b_;
};


class EncoderCalibrateComponent{
    static constexpr size_t ACCEL_SECTORS = 15;
    static constexpr size_t DEACCEL_SECTORS = 15;
    static constexpr size_t CONSTANT_MOVE_POLES = 5;
    static constexpr real_t MAX_DUTY = 0.2_r;

    EncoderCalibrateComponent(drivers::EncoderIntf & encoder):
        encoder_(encoder)

        {;}
    struct TaskConfig{
    };
    
    struct TaskBase{
    };

    struct SpeedUpTaskContext{
        const size_t current_step;
        const bool is_forward;
    };


    void tick(){

    }

    struct CalibrateDataBlocks{
        // using T = PackedCalibratePoint;
        static constexpr auto MIN_MOVE_THRESHOLD = q16((1.0 / MOTOR_POLE_PAIRS / 4) * 0.3);

        using Block = CalibrateDataBlock;


        Block forward_block;
        Block backward_block;
    };

private:
    drivers::EncoderIntf & encoder_;

    // static void tick(EncoderCalibrateComponent & self, )
};




struct Reflecter{
    template<typename T>
    static constexpr int display(T && obj){
        return 0;
    }
};

struct TaskSpawner{
    template<typename T>
    static constexpr auto spawn(T && obj){
        return 0;
    }
};

struct PreoperateTasks{

};

namespace ymd::hal{
    auto & PROGRAM_FAULT_LED = PC<14>();
}

struct Archive{

};

class MotorSystem{
public:
    using TaskError = BeepTasks::TaskError;
    MotorSystem(
        drivers::EncoderIntf & encoder,
        StepperSVPWM & svpwm
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
    };

    Result<void, Error> resume(){
        let begin_u = clock::micros();

        let meas_lap_position = ({
            if(let res = retry(2, [&]{return encoder_.update();});
                res.is_err()) return Err(Error(res.unwrap_err()));
            // execution_time_ = clock::micros() - begin_u;
            let either_lap_position = encoder_.get_lap_position();
            if(either_lap_position.is_err())
                return Err(Error(either_lap_position.unwrap_err()));
            1 - either_lap_position.unwrap();
        });

        auto & comp = calibrate_comp_;
        // if(let may_err = comp.err(); may_err.is_some()){
        if constexpr(false){
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
                CalibrateTasks{pos_sensor_.forward_cali_vec, 
                    pos_sensor_.backward_cali_vec}
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
        }
        else if(comp.is_finished()){

            is_comp_finished_ = true;
            pos_sensor_.update(meas_lap_position);
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
            let meas_position = pos_sensor_.position();
            let meas_speed = pos_sensor_.speed();
            let pos_contribute = 0.8_r * (targ_position - meas_position);
            let speed_contribute = 0.039_r*(targ_speed - meas_speed);
            let curr = CLAMP2(pos_contribute + speed_contribute, 0.4_r);
            // let curr = 0.2_r;
            let [s,c] = sincospu(frac(
                // (correct_raw_position(meas_lap_position) - 0.007_r)) * 50);
                (pos_sensor_.lap_position() + SIGN_AS(0.007_r, curr))) * 50);
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
                let targ = item.get_targ();
                let meas = item.get_meas();
                // DEBUG_PRINTLN(targ, meas, fposmodp(q20(targ - meas), 0.02_q20) * 100);
                let position_err = q20(targ - meas);
                let mod_err = fposmodp(position_err, 0.02_q20);
                DEBUG_PRINTLN(targ, meas, mod_err * 100);
                clock::delay(1ms);
            }
        };

        print_view(pos_sensor_.forward_cali_vec.as_view());
        print_view(pos_sensor_.backward_cali_vec.as_view());

        for(int i = 0; i < 50; i++){
            let raw = real_t(i) / 50;
            let corrected = pos_sensor_.correct_raw_position(raw);
            DEBUG_PRINTLN(raw, corrected, (corrected - raw) * 100);
            clock::delay(1ms);
        }

        return Ok();
    }


    Microseconds execution_time_ = 0us;
// private:
public:
    drivers::EncoderIntf & encoder_;
    StepperSVPWM & svpwm_;

    CoilMotionCheckTasks coil_motion_check_comp_ = {};
    CalibrateTasks calibrate_comp_ = {
        pos_sensor_.forward_cali_vec,
        pos_sensor_.backward_cali_vec
    };


    bool is_comp_finished_ = false;

    static constexpr size_t MC_W = 1000u;
    static constexpr real_t MC_TAU = 80;



    using CommandShaper = dsp::CommandShaper1;
    using CommandShaperConfig = CommandShaper::Config;

    static constexpr CommandShaperConfig CS_CONFIG{
            .kp = MC_TAU * MC_TAU,
            .kd = 2 * MC_TAU,
            .max_spd = 30.0_r,
            .max_acc = 140.0_r,
            .fs = ISR_FREQ
    };


    CommandShaper cs_{CS_CONFIG};

    PositionSensor pos_sensor_;
};


class ArchiveSystem{

    void save(){
        // at24.load_bytes(0_addr, std::span(rdata)).examine();
        // while(not at24.is_available()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN(rdata);
        // const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
        // at24.store_bytes(0_addr, std::span(data)).examine();

        // while(not at24.is_available()){
        //     at24.poll().examine();
        // }

        // DEBUG_PRINTLN("done", clock::micros() - begin_u);
        // while(true);
    }

};


[[maybe_unused]] 
static void test_check(drivers::EncoderIntf & encoder,StepperSVPWM & svpwm){
    auto motor_system_ = MotorSystem{encoder, svpwm};

    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        let res = motor_system_.resume();
        if(res.is_err()){
            PANIC();
        }
    });

    DEBUGGER.no_brackets();
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
            
        //     motor_system_.pos_sensor_.position(),
        //     motor_system_.pos_sensor_.speed(),
        //     motor_system_.execution_time_.count(),
        //     motor_system_.cs_.get()
        // );
    }

    // DEBUG_PRINTLN("finished");

    while(true);
}




[[maybe_unused]] static void test_eeprom(){
    hal::I2cSw i2c_sw{hal::portD[1], hal::portD[0]};
    i2c_sw.init(800_KHz);
    drivers::AT24CXX at24{drivers::AT24CXX::Config::AT24C02{}, i2c_sw};

    const auto begin_u = clock::micros();
    uint8_t rdata[3] = {0};
    at24.load_bytes(0_addr, std::span(rdata)).examine();
    while(not at24.is_available()){
        at24.poll().examine();
    }

    DEBUG_PRINTLN(rdata);
    const uint8_t data[] = {uint8_t(rdata[0]+1),2,3};
    at24.store_bytes(0_addr, std::span(data)).examine();

    while(not at24.is_available()){
        at24.poll().examine();
    }

    DEBUG_PRINTLN("done", clock::micros() - begin_u);
    while(true);
}



template<typename T, typename R>
__fast_inline constexpr T map_nearest(const T value, R && range){
    auto it = std::begin(range);
    auto end = std::end(range);
    
    T nearest = *it;
    auto min_diff = ABS(value - nearest);
    
    while(++it != end) {
        let current = *it;
        let diff = ABS(value - current);
        if(diff < min_diff) {
            min_diff = diff;
            nearest = current;
        }
    }
    return nearest;
}

static constexpr void static_test(){
    static_assert(map_nearest(0, std::initializer_list<int>{1,-3,-5}) == 1);
}

void mystepper_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUG_PRINTLN(ReleaseInfo::from("Rs1aN", Version{1,0}));
    clock::delay(4ms);
    DEBUG_PRINTLN(ReleaseInfo::from("Rs1aN", Version{1,0}));
    clock::delay(4ms);
    DEBUG_PRINTLN(ReleaseInfo::from("Rs1aN", Version{1,0}));
    // PANIC(ReleaseInfo::from("Rstr", Version{1,0}).unwrap());
    // PANIC(Author::from("Rstr").unwrap());

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
        .mode = hal::TimerCountMode::CenterAlignedDualTrig
    });

    timer.enable_arr_sync();
    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    StepperSVPWM svpwm{
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
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelIndex::CH2, hal::AdcSampleCycles::T28_5},
            {hal::AdcChannelIndex::CH3, hal::AdcSampleCycles::T28_5},
        }
    );

    adc.set_injected_trigger(hal::AdcOnChip::InjectedTrigger::T1TRGO);
    adc.enable_auto_inject(DISEN);

    auto & inj_a = adc.inj<1>();
    auto & inj_b = adc.inj<3>();

    auto & trig_gpio = hal::PC<13>();
    trig_gpio.outpp();

    real_t a_curr;
    real_t b_curr;
    adc.attach(hal::AdcIT::JEOC, {0,0}, [&]{
        // trig_gpio.toggle();
        // DEBUG_PRINTLN_IDLE(millis());
        // trig_gpio.toggle();
        static bool is_a = false;
        is_a = !is_a;
        if(is_a){
            // DEBUG_PRINTLN_IDLE(a_curr);
            b_curr = inj_b.get_voltage();
            a_curr = inj_a.get_voltage();
        }else{
            b_curr = inj_b.get_voltage();
            a_curr = inj_a.get_voltage();
            // DEBUG_PRINTLN_IDLE(b_curr);
        }
    });


    auto & spi = hal::spi1;
    spi.init(18_MHz);

    drivers::MT6816 encoder{{
        spi, 
        spi.attach_next_cs(hal::portA[15]).value()
    }};


    encoder.init({
        .fast_mode_en = DISEN
    }).examine();

    test_check(encoder, svpwm);
    // test_eeprom();
}

struct LapCalibrateTable{
    using T = real_t;
    using Data = std::array<T, MOTOR_POLE_PAIRS>;


    Data data; 

    constexpr real_t error_at(const real_t raw) const {
        return forward_uni(raw);
    }

    constexpr real_t correct_position(const real_t raw_position) const{
        return raw_position + error_at(raw_position);
    }

    constexpr real_t position_to_elecrad(const real_t lap_pos) const{
        return real_t(MOTOR_POLE_PAIRS * TAU) * lap_pos;
    }

private:

    constexpr T forward(const T x) const {
        const T x_wrapped = fposmodp(x,real_t(MOTOR_POLE_PAIRS));
        const uint x_int = int(x_wrapped);
        const T x_frac = x_wrapped - x_int;

        let [ya, yb] = [&] -> std::tuple<real_t, real_t>{
            if(x_int == MOTOR_POLE_PAIRS - 1){
                return {data[MOTOR_POLE_PAIRS - 1], data[0]};
            }else{
                return {data[x_int], data[x_int + 1]};
            }
        }();

        return LERP(ya, yb, x_frac);
    }

    constexpr T forward_uni(const T x) const {
        return x * MOTOR_POLE_PAIRS;
    }
};