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
#include "hal/gpio/gpio.hpp"

#include "drivers/Encoder/odometer.hpp"
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"

#include "src/testbench/tb.h"

#include "types/regions/range2/range2.hpp"


#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"


using namespace ymd;

#define UART hal::uart1


template<typename Fn, typename Fn_Dur>
__inline auto retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    const auto res = std::forward<Fn>(fn)();
    using Ret = std::decay_t<decltype(res)>;
    if(res.is_ok()) return Ret(Ok());
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}


template<typename Fn>
__inline auto retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}

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
private:

    hal::TimerOcPair channel_a_;
    hal::TimerOcPair channel_b_;
};

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};


static constexpr size_t MOTOR_POLES = 50;


//储存了压缩保存的期望数据和实际数据 用于校准点分析
struct PackedCalibratePoint{
public:
    static constexpr size_t IGNORE_BITS = 5;
    // static constexpr auto MAX_ERR = q31(q30(1.0 / (1 << IGNORE_BITS)));
    static constexpr auto MAX_ERR = q31(1.0 / 5);
    // static constexpr q24 MAX_ERR = q24(1.0 / (1 << IGNORE_BITS));

    constexpr PackedCalibratePoint():
        targ_packed_data_(0),
        meas_packed_data_(0){;}

    static constexpr Option<PackedCalibratePoint> 
    from_targ_and_meas(const q31 targ, const q31 meas){
        const auto targ_packed_data = ({
            const auto opt = real_to_packed(targ);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        const auto meas_packed_data = ({
            const auto opt = real_to_packed(meas);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        return Some(PackedCalibratePoint(
            targ_packed_data,
            meas_packed_data
        ));
    }

    constexpr q31 get_targ() const {
        return packed_to_real(targ_packed_data_);
    }

    constexpr q31 get_meas() const {
        return packed_to_real(meas_packed_data_);
    }

    template <size_t N>
    constexpr auto get() const {
        if constexpr (N == 0) return get_targ();
        else if constexpr (N == 1) return get_meas();
    }
private:
    constexpr PackedCalibratePoint(
        const uint16_t targ_packed, 
        const uint16_t meas_packed
    ):
        targ_packed_data_(targ_packed),
        meas_packed_data_(meas_packed){;}

    uint16_t targ_packed_data_;
    uint16_t meas_packed_data_;

    static constexpr Option<uint16_t> real_to_packed(const q31 unpacked){
        constexpr auto SHIFT_BITS = (24-(16 + IGNORE_BITS));
        if(unpacked > IGNORE_BITS) return None;
        return Some(q24(unpacked).to_i32() >> SHIFT_BITS);
    }

    static constexpr q31 packed_to_real(const uint16_t packed){
        return q24::from_i32(packed << (8 - IGNORE_BITS));
    }
};

// Specialize tuple traits
namespace std {
    template <>
    struct tuple_size<PackedCalibratePoint> 
        : integral_constant<size_t, 2> {};

    template <size_t N>
    struct tuple_element<N, PackedCalibratePoint> {
        using type = ymd::q31;
    };
}

//储存了完整的校准数据
using CalibrateDataBlock = std::array<PackedCalibratePoint, MOTOR_POLES>;


//校准数据的视图
struct CalibrateDataBlockView{
    using Block = CalibrateDataBlock;

    struct Iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = PackedCalibratePoint;
        using difference_type = std::ptrdiff_t;
        using pointer = const PackedCalibratePoint*;
        using reference = const PackedCalibratePoint&;

        constexpr explicit Iterator(
            const PackedCalibratePoint* data,
            size_t index
        ) : 
            data_(data), 
            index_(index){}

        constexpr reference operator*() const { 
            return data_[index_];
        }

        constexpr Iterator& operator++() {
            ++index_;
            return *this;
        }

        constexpr bool operator!=(const Iterator& other) const {
            return index_ != other.index_;
        }

    private:
        const PackedCalibratePoint* data_;
        size_t index_;
    };

    constexpr explicit CalibrateDataBlockView(const Block & block):
        block_(block){;}
    
    constexpr Iterator begin() const noexcept {
        return Iterator(block_.data(), 0);
    }

    constexpr Iterator end() const noexcept {
        return Iterator(block_.data(), MOTOR_POLES);
    }

private:


    const Block & block_;
};

//储存了校准过程中不断提交的新数据
struct CalibrateDataVector{
public:
    constexpr CalibrateDataVector() = default;

    constexpr Result<void, void> push_back(const q31 targ, const q31 meas){
        if(len_ == MOTOR_POLES) return Err();
        block_[len_] = ({
            const auto opt = PackedCalibratePoint::from_targ_and_meas(
                targ, meas
            );

            if(opt.is_none()) return Err();
            opt.unwrap();
        });

        len_ ++;

        return Ok();
    }

    constexpr void clear(){
        len_ = 0;
    }

    constexpr bool is_full() const {
        return len_ == MOTOR_POLES;
    }

    constexpr size_t size() const {
        return len_;
    }

    constexpr Option<CalibrateDataBlockView>
    as_view() const {
        if(len_ != MOTOR_POLES) return None;
        return Some(CalibrateDataBlockView(block_));
    }


private:
    using Block = CalibrateDataBlock;

    Block block_;
    size_t len_ = 0;
};




//提供了对校准数据分析的纯函数
struct CalibrateDataAnalyzer{
    // static constexpr 

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



    //保存了
    struct CalibrateDataBlocks{
        // using T = PackedCalibratePoint;
        static constexpr auto MIN_MOVE_THRESHOLD = q16((1.0 / MOTOR_POLES / 4) * 0.3);

        using Block = CalibrateDataBlock;


        Block forward_block;
        Block backward_block;
    };

private:
    drivers::EncoderIntf & encoder_;

    // static void tick(EncoderCalibrateComponent & self, )
};



class CoilCheckComponent{
public:
    enum class Error:uint8_t{
        TaskNotDone,
        RotorIsMovingBeforeChecking,
    };
    static constexpr auto DRIVE_DUTY = 0.1_r;
    static constexpr auto MINIMAL_MOVING_THRESHOLD = 1.0_r / MOTOR_POLES;
    static constexpr auto MINIMAL_STILL_THRESHOLD = 0.0003_r;
    static constexpr auto STILL_CHECK_TICKS = 80u;
    static constexpr auto MOVE_CHECK_TICKS = 1600u;
    static constexpr auto BREAK_TICKS = 1000u;
    static constexpr auto TICKS_PER_SECTOR = 64u;
    
    // CoilCheckComponent(
    //     drivers::EncoderIntf & encoder,
    //     StepperSVPWM & svpwm
    // ):
    //     encoder_(encoder){;}

    // constexpr CoilCheckComponent():
    // {;}


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
    constexpr AlphaBetaDuty resume(const real_t lappos){
        return {1,0};
    }

private:
    //1. 检测转子已经停下
    //2. 检测A侧能够移动
    //3. 再次静止以保证下一项检测前停下
    //4. 检测转子已经停下
    //3. 检测B侧能够移动


    struct CheckStillTask final{

        struct Config{

        };

        
        struct Dignosis {
            Option<Error> err;
            Range2<q16> move_range;
        };
        
        constexpr CheckStillTask(const Config & cfg){;} 

        constexpr AlphaBetaDuty resume(const real_t lappos){
            if(may_move_range_.is_none())
                may_move_range_ = Some(Range2<q16>::from_center(lappos));
            else 
                may_move_range_ = Some(may_move_range_.unwrap().merge(lappos));

            return AlphaBetaDuty{
                .alpha = DRIVE_DUTY,
                .beta = 0
            };
        }

        constexpr bool is_done(){
            return tick_cnt_ > STILL_CHECK_TICKS;
        }

        constexpr Dignosis dignosis() const {
            ASSERT(may_move_range_.is_some());

            const auto move_range = may_move_range_.unwrap();

            auto make_err = [&]() -> Option<Error>{
                if(move_range.length() > MINIMAL_STILL_THRESHOLD)
                    Some(Error::RotorIsMovingBeforeChecking);
                return None;
            };

            return Dignosis{
                .err = make_err(),
                .move_range = move_range
            };
        }
    private:
        size_t tick_cnt_ = 0;

        Option<Range2<q16>> may_move_range_ = None;
    };

    struct CheckMovingTask final{
        struct Config{
            const bool is_beta;
        };

        struct [[nodiscard]] Dignosis {
            Option<Error> err;
            Range2<q16> move_range;
        };

        constexpr CheckMovingTask(const Config & cfg){
            is_beta_ = cfg.is_beta;
        };

        constexpr AlphaBetaDuty resume(const real_t lappos){
            if(may_move_range_.is_none())
                may_move_range_ = Some(Range2<q16>::from_center(lappos));
            else 
                may_move_range_ = Some(may_move_range_.unwrap().merge(lappos));

            const auto duty = sinpu(LERP(
                q16(tick_cnt_) / MOVE_CHECK_TICKS,
                -0.5_r, 0.5_r
            ));

            auto make_duty = [&]() -> AlphaBetaDuty{
                if(is_beta_){
                    return {duty, 0};
                }else{
                    return {0, duty};
                }
            };

            return make_duty();
        }

        constexpr bool is_done(){
            return tick_cnt_ > STILL_CHECK_TICKS;
        }

        constexpr Dignosis dignosis() const {
            ASSERT(may_move_range_.is_some());

            const auto move_range = may_move_range_.unwrap();

            auto make_err = [&]() -> Option<Error>{
                if(move_range.length() > MINIMAL_STILL_THRESHOLD)
                    Some(Error::RotorIsMovingBeforeChecking);
                return None;
            };

            return Dignosis{
                .err = make_err(),
                .move_range = move_range
            };
        }
    private:
        size_t tick_cnt_ = 0;
        Option<Range2<q16>> may_move_range_ = None;
        bool is_beta_;
    };

};


void test_calibrate(){
    const CalibrateDataVector vec;
    // ... fill data ...

    const auto view = vec.as_view().unwrap();
    for (const auto [targ, meas] : view) {
        DEBUG_PRINTLN(targ, meas);
        // Use targ/meas directly
        // targ will be get_targ() value
        // meas will be get_meas() value
    }
}

#define let const auto
// static constexpr size_t CHOP_FREQ = 30_KHz;
static constexpr size_t CHOP_FREQ = 20_KHz;
// static constexpr size_t CHOP_FREQ = 100;




template<typename T, typename R>
__fast_inline constexpr T map_nearest(const T value, R && range){
    auto it = std::begin(range);
    auto end = std::end(range);
    
    T nearest = *it;
    auto min_diff = ABS(value - nearest);
    
    while(++it != end) {
        const auto current = *it;
        const auto diff = ABS(value - current);
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


    real_t targ_lappos = 0;
    real_t meas_lappos = 0;
    timer.attach(hal::TimerIT::Update, {0,0}, [&](){
        // retry(1, [&]{return encoder.update();}).examine();
        retry(2, [&]{return encoder.update();}).examine();
        meas_lappos = encoder.get_lap_position().examine();
        // DEBUG_PRINTLN(drivers::EncoderError::Kind::CantSetup);
        const auto t = clock::time();
        // const auto [st, ct] = sincospu(t * 93);
        // const auto [st, ct] = sincospu(t * 3);
        // const auto [st, ct] = sincospu(10 * sinpu(t));
        targ_lappos = frac(2.4_r * t);
        let ref_epos = MOTOR_POLES * targ_lappos;
        const auto [st, ct] = sincospu(ref_epos);
        // const auto [st, ct] = sincospu(sinpu(t));
        const auto amp = 0.3_r;

        svpwm.set_alpha_beta_duty(ct * amp, st * amp);
    });



    while(true){

        DEBUG_PRINTLN_IDLE(
            // timer.oc<1>().cvr(), 
            // timer.oc<2>().cvr(), 
            // timer.oc<3>().cvr(), 
            // timer.oc<4>().cvr(), 

            // hal::portA[8].read().to_bool(),
            // hal::portA[9].read().to_bool(),
            // hal::portA[10].read().to_bool(),
            // hal::portA[11].read().to_bool(),

            // ena_gpio.read().to_bool(),
            // enb_gpio.read().to_bool(),
            // inj_a.get_voltage(),
            // inj_b.get_voltage(),
            100 * (targ_lappos - frac(2.4_r * clock::time())),
            meas_lappos,
            // 100 * (position - targ_lappos),
            // 100 * (position2 - targ_lappos),
            // 100 * (position3 - targ_lappos),
            // // 100 * (position4 - targ_lappos),
            // 100 * best_err,
            
            trig_gpio.read().to_bool()
        );

        // clock::delay(10us);
        clock::delay(1ms);
    }
}

struct LapCalibrateTable{
    using T = real_t;
    using Data = std::array<T, MOTOR_POLES>;


    Data data; 

    constexpr real_t error_at(const real_t raw) const {
        return forward_uni(raw);
    }

    constexpr real_t correct_position(const real_t raw_position) const{
        return raw_position + error_at(raw_position);
    }

    constexpr real_t position_to_elecrad(const real_t lap_pos) const{
        return real_t(MOTOR_POLES * TAU) * lap_pos;
    }

private:

    constexpr T forward(const T x) const {
        const T x_wrapped = fposmodp(x,real_t(50));
        const uint x_int = int(x_wrapped);
        const T x_frac = x_wrapped - x_int;

        const auto [ya, yb] = [&] -> std::tuple<real_t, real_t>{
            if(x_int == MOTOR_POLES - 1){
                return {data[MOTOR_POLES - 1], data[0]};
            }else{
                return {data[x_int], data[x_int + 1]};
            }
        }();

        return LERP(ya, yb, x_frac);
    }

    constexpr T forward_uni(const T x) const {
        return x * MOTOR_POLES;
    }
};