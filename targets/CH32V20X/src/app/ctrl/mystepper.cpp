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


#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"


using namespace ymd;

#define UART hal::uart1

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

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};


static constexpr size_t CALIBRATE_SECTORS = 50;


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
using CalibrateDataBlock = std::array<PackedCalibratePoint, CALIBRATE_SECTORS>;


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
        return Iterator(block_.data(), CALIBRATE_SECTORS);
    }

private:


    const Block & block_;
};

//储存了校准过程中不断提交的新数据
struct CalibrateDataVector{
public:
    constexpr CalibrateDataVector() = default;

    constexpr Result<void, void> push_back(const q31 targ, const q31 meas){
        if(len_ == CALIBRATE_SECTORS) return Err();
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
        return len_ == CALIBRATE_SECTORS;
    }

    constexpr size_t size() const {
        return len_;
    }

    constexpr Option<CalibrateDataBlockView>
    as_view() const {
        if(len_ != CALIBRATE_SECTORS) return None;
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
    static constexpr size_t CONSTANT_SECTORS = 5;
    static constexpr real_t MAX_DUTY = 0.2_r;
    // struct Config{
    //     size_t accel_sectors = 15;
    //     size_t deaccel_sectors = 15;
    //     size_t micr
    //     real_t max_duty = 0.2_r;
    // };

    struct TaskConfig{
    };
    
    struct TaskBase{
        


    };

    struct SpeedUpTask{

    };

//保存了
struct CalibrateDataBlocks{
    // using T = PackedCalibratePoint;
    static constexpr size_t CALIBRATE_SECTORS = 50;

    using Block = CalibrateDataBlock;


    Block forward_block;
    Block backward_block;
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

    hal::Gpio & ena_gpio = hal::portB[0];
    hal::Gpio & enb_gpio = hal::portA[7];
    
    auto & timer = hal::timer1;
    auto & pwm_ap = timer.oc<1>();
    auto & pwm_an = timer.oc<2>();
    auto & pwm_bp = timer.oc<3>();
    auto & pwm_bn = timer.oc<4>();

    ena_gpio.outpp(HIGH);
    enb_gpio.outpp(HIGH);


    timer.init({
        .freq = CHOP_FREQ,
        .mode = hal::TimerCountMode::CenterAlignedDualTrig
    });

    timer.enable_arr_sync();
    timer.set_trgo_source(hal::TimerTrgoSource::Update);

    const hal::TimerOcPwmConfig pwm_noinv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = HIGH
    };

    const hal::TimerOcPwmConfig pwm_inv_cfg = {
        .cvr_sync_en = EN,
        .valid_level = LOW,
    };
    
    pwm_ap.init(pwm_noinv_cfg);
    pwm_an.init(pwm_noinv_cfg);
    pwm_bp.init(pwm_inv_cfg);
    pwm_bn.init(pwm_inv_cfg);

    hal::TimerOcPair pwm_a = {pwm_ap, pwm_an};
    hal::TimerOcPair pwm_b = {pwm_bp, pwm_bn};

    pwm_a.inverse(EN);
    pwm_b.inverse(DISEN);

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


    real_t ref_lappos = 0;
    timer.attach(hal::TimerIT::Update, {0,0}, [&](){
        // retry(1, [&]{return encoder.update();}).examine();
        retry(1, [&]{return encoder.update();}).examine();
        // DEBUG_PRINTLN(drivers::EncoderError::Kind::CantSetup);
        const auto t = clock::time();
        // const auto [st, ct] = sincospu(t * 93);
        // const auto [st, ct] = sincospu(t * 3);
        // const auto [st, ct] = sincospu(10 * sinpu(t));
        ref_lappos = frac(2.4_r * t);
        let ref_epos = CALIBRATE_SECTORS * ref_lappos;
        const auto [st, ct] = sincospu(ref_epos);
        // const auto [st, ct] = sincospu(sinpu(t));
        const auto amp = 0.3_r;

        pwm_a.set_duty(st * amp);
        pwm_b.set_duty(ct * amp);
    });



    while(true){
        static q16 best_err = encoder.get_lap_position().examine() - ref_lappos;
        const auto position = encoder.get_lap_position().examine();
        // const auto position2 = q16::from_i32((position.to_i32() ^ 0x00000080) - 0x00000080);

        const auto position2 = q16::from_i32(position.to_i32() - 0x00000100);
        const auto position3 = q16::from_i32(position.to_i32() - 0x00000080);

        const auto err = position - ref_lappos;
        const auto err2 = position2 - ref_lappos;
        const auto err3 = position3 - ref_lappos;

        // const auto position4 = q16::from_i32(position.to_i32() & ~0x00000100);

        // auto map_bit_flip = [](const q16 last, const q16 raw) -> q16{

        //     // Helper function to calculate wrapped difference
        //     // auto wrapped_diff = [](q16 a, q16 b) {
        //     //     const auto direct = ABS(a - b);
        //     //     return std::min(direct, MAX_Q16 - direct);
        //     // };

        //     // Generate possible candidates
        //     const auto candidates = 

        //     // Find candidate with smallest wrapped difference
        //     q16 min_diff = 1;
        //     q16 best = raw;
            
        //     for (const auto & cand : candidates) {
        //         const auto diff = ABS(last - cand);
        //         if (diff < min_diff) {
        //             min_diff = diff;
        //             best = cand;
        //         }
        //     }

        //     return best;
        // };

        best_err = map_nearest(best_err, std::initializer_list<q16>{
            err,
            err2, err3
        });

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
            100 * (position - ref_lappos),
            100 * (position2 - ref_lappos),
            100 * (position3 - ref_lappos),
            // 100 * (position4 - ref_lappos),
            100 * best_err,

            position, 
            position2, 
            position3, 
            // ADC1->IDATAR1,
            // ADC1->IDATAR2,
            // ADC1->IDATAR3,
            // a_curr, b_curr,
            // q31(1.0 / 5),
            trig_gpio.read().to_bool()
        );

        // clock::delay(10us);
        clock::delay(1ms);
    }
}

struct LapCalibrateTable{
    using T = real_t;
    using Data = std::array<T, CALIBRATE_SECTORS>;


    Data data; 

    constexpr real_t error_at(const real_t raw) const {
        return forward_uni(raw);
    }

    constexpr real_t correct_position(const real_t raw_position) const{
        return raw_position + error_at(raw_position);
    }

    constexpr real_t position_to_elecrad(const real_t lap_pos) const{
        return real_t(CALIBRATE_SECTORS * TAU) * lap_pos;
    }

private:

    constexpr T forward(const T x) const {
        const T x_wrapped = fposmodp(x,real_t(50));
        const uint x_int = int(x_wrapped);
        const T x_frac = x_wrapped - x_int;

        const auto [ya, yb] = [&] -> std::tuple<real_t, real_t>{
            if(x_int == CALIBRATE_SECTORS - 1){
                return {data[CALIBRATE_SECTORS - 1], data[0]};
            }else{
                return {data[x_int], data[x_int + 1]};
            }
        }();

        return LERP(ya, yb, x_frac);
    }

    constexpr T forward_uni(const T x) const {
        return x * CALIBRATE_SECTORS;
    }
};