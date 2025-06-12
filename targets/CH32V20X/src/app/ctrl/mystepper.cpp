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

#include <atomic>


using namespace ymd;


#define let const auto 
namespace ymd::fp{
// https://www.bluepuni.com/archives/zip-for-cpp20/
template <std::ranges::input_range ...Views>
class Zip : public std::ranges::view_interface<Zip<Views...>> {
public:
    struct iterator;
    struct sentinel;

public:
    Zip() = default;
    // Views are cheap to copy, but owning views cannot be done. (= delete)
    constexpr Zip(Views ...vs) noexcept: _views(std::move(vs)...) {}
    constexpr auto begin() {
        return std::apply([&](Views &...views) { return iterator(views...); }, _views);
    }
    constexpr auto end() requires (std::ranges::random_access_range<Views> && ...) {
        return sentinel{this};
    }
    constexpr auto size() const requires (std::ranges::sized_range<Views> && ...) {
        return std::apply([&](auto &&...views)
            { return std::min({std::ranges::size(views)...}); }, _views);
    }

private:
    std::tuple<Views...> _views;
};

template <std::ranges::input_range ...Views>
struct Zip<Views...>::iterator {
    friend struct sentinel;
    // TODO: flexible iterator_concepts.
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = std::tuple<std::ranges::range_value_t<Views>...>;
    using difference_type = std::common_type_t<std::ranges::range_difference_t<Views>...>;

    iterator() = default;
    constexpr iterator(Views &...views): _currents{std::ranges::begin(views)...} {}

    constexpr auto operator*() const {
        return std::apply([&](auto &&...iters) {
            // No <auto> decay!
            // Example: zip(views::iota(1, 5), named_vector_of_int).
            // Return: std::tuple<int, int&>.
            return std::tuple<decltype(*iters)...>((*iters)...);
        }, _currents);
    }

    constexpr auto operator[](difference_type n) const {
        auto tmp = *this;
        tmp.operator+=(n);
        return tmp;
    }

    constexpr iterator& operator++() {
        return this->operator+=(1);
    }

    constexpr iterator operator++(int) {
        auto tmp = *this;
        this->operator+=(1);
        return tmp;
    }
    constexpr iterator& operator+=(difference_type n) {
        std::apply([&](auto &...iters) { ((iters += n),...); }, _currents);
        return *this;
    }

    friend constexpr auto operator<=>(const iterator &x, const iterator &y) = default;

private:
    std::tuple<std::ranges::iterator_t<Views>...> _currents;
};

template <std::ranges::input_range ...Views>
struct Zip<Views...>::sentinel {
    sentinel() = default;
    constexpr sentinel(Zip *this_zip) noexcept: _this_zip(this_zip) {}

    friend bool operator==(const iterator &x, const sentinel &y) {
        return [&]<auto ...Is>(std::index_sequence<Is...>) {
            return ((std::get<Is>(x._currents)
                        == std::ranges::end(std::get<Is>(y._this_zip->_views))) || ...);
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

private:
    Zip *_this_zip;
};

inline constexpr struct Zip_fn {
    // template <std::ranges::input_range ...Rs>
    template <typename ...Rs>
    [[nodiscard]]
    constexpr auto operator()(Rs &&...rs) const {
        if constexpr (sizeof...(rs) == 0) {
            return std::views::empty<std::tuple<>>;
        } else {
            return Zip<std::views::all_t<Rs>...>(std::forward<Rs>(rs)...);
        }
    }
} zip;
}

// static constexpr size_t CHOP_FREQ = 30_KHz;
static constexpr size_t CHOP_FREQ = 20_KHz;
static constexpr size_t ISR_FREQ = 20_KHz * 2;
// static constexpr size_t CHOP_FREQ = 100;

static constexpr size_t STEPPER_SECTORS_PER_ROTATION = 4;
static constexpr size_t BLDC_SECTORS_PER_ROTATION = 6;

enum class MotorType:uint8_t{
    Bldc,
    Stepper
};

struct Nema42MotorSettings{
    static constexpr MotorType MOTOR_TYPE = MotorType::Stepper;
    static constexpr size_t MOTOR_POLE_PAIRS = 50;
    static constexpr size_t SECTORS_PER_ROTATION = STEPPER_SECTORS_PER_ROTATION;
    static constexpr size_t SECTORS_PER_POSITION = MOTOR_POLE_PAIRS * SECTORS_PER_ROTATION;
};

using MyMotorSettings = Nema42MotorSettings;
static constexpr size_t SECTORS_PER_ROTATION = MyMotorSettings::SECTORS_PER_ROTATION;
static constexpr size_t SECTORS_PER_POSITION = MyMotorSettings::SECTORS_PER_POSITION;
static constexpr size_t MOTOR_POLE_PAIRS = MyMotorSettings::MOTOR_POLE_PAIRS;


#define UART hal::uart1

template<typename Fn, typename Fn_Dur>
__inline auto retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    let res = std::forward<Fn>(fn)();
    using Ret = std::decay_t<decltype(res)>;
    if(res.is_ok()) return Ret(Ok());
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}


template<typename Fn>
__inline auto retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}


namespace ymd::magic{
namespace details{
template<typename Tup>
struct tuple_to_variant{
};

template<typename... Ts>
struct tuple_to_variant<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

template<typename Var>
struct variant_to_tuple{
};

template<typename... Ts>
struct variant_to_tuple<std::variant<Ts...>> {
    using type = std::tuple<Ts...>;
};

template<size_t I, typename Variant>
struct variant_get_trait{
    static constexpr auto get(const Variant & var, size_t index){
        return std::get<index>(var);
    }
};



template <typename T, typename... Ts>
struct tuple_erase_duplicate : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct tuple_erase_duplicate<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
    , tuple_erase_duplicate<std::tuple<Ts...>, Us...>
    , tuple_erase_duplicate<std::tuple<Ts..., U>, Us...>> {};



}

template<typename Tup>
using tuple_to_variant_t = typename details::tuple_to_variant<Tup>::type;

template<typename Var>
using variant_to_tuple_t = typename details::variant_to_tuple<Var>::type;

template<size_t I, typename Variant>
using variant_element_t = std::tuple_element_t<I, variant_to_tuple_t<Variant>>;

static_assert(std::is_same_v<
    tuple_to_variant_t<std::tuple<int, float, bool>>,
    std::variant<int, float, bool>
>);


template <typename... Ts>
using tuple_erase_duplicate_t = typename details::
    tuple_erase_duplicate<std::tuple<>, Ts...>::type;

}

template<typename T>
using config_to_task_t = typename T::Task;

template<typename ConfigsTuple, typename IndexSeq>
struct configs_tuple_to_tasks_variant_impl;

template<typename... Configs, std::size_t... Is>
struct configs_tuple_to_tasks_variant_impl<
    std::tuple<Configs...>, std::index_sequence<Is...>>
{
    using type = magic::tuple_to_variant_t<
        magic::tuple_erase_duplicate_t<
            typename std::tuple_element_t<Is, std::tuple<Configs...>>::Task...>>;
};

template<typename ConfigsTuple>
using configs_tuple_to_tasks_variant_t = 
    typename configs_tuple_to_tasks_variant_impl<ConfigsTuple,
    std::make_index_sequence<std::tuple_size_v<ConfigsTuple>>>::type;

template<typename ConfigsTuple, typename IndexSeq>
struct configs_tuple_to_dignosis_variant_impl;

template<typename... Configs, std::size_t... Is>
struct configs_tuple_to_dignosis_variant_impl<
    std::tuple<Configs...>, std::index_sequence<Is...>> {
    using type = magic::tuple_to_variant_t<
        magic::tuple_erase_duplicate_t<
            typename std::tuple_element_t<Is, 
                std::tuple<Configs...>>::Task::Dignosis...>>;
};

template<size_t I, typename ConfigsTuple>
using idx_to_task_t = typename std::tuple_element_t<I, ConfigsTuple>::Task;

template<typename ConfigsTuple>
using configs_tuple_to_dignosis_variant_t = typename 
    configs_tuple_to_dignosis_variant_impl<ConfigsTuple,
    std::make_index_sequence<std::tuple_size_v<ConfigsTuple>>
>::type;


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

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};





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
        let targ_packed_data = ({
            let opt = real_to_packed(targ);
            if(opt.is_none()) return None;
            opt.unwrap();
        });

        let meas_packed_data = ({
            let opt = real_to_packed(meas);
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

    constexpr q31 to_inaccuracy() const {
        return fposmodp(q20(get_targ() - get_meas()), 0.02_q20);
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

    static constexpr Option<uint16_t> real_to_packed(const q16 unpacked){
        return Some(uint16_t(unpacked.to_i32()));
    }

    static constexpr q31 packed_to_real(const uint16_t packed){
        return q16::from_i32(packed);
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
using CalibrateDataBlock = std::array<PackedCalibratePoint, MOTOR_POLE_PAIRS>;


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
        return Iterator(block_.data(), MOTOR_POLE_PAIRS);
    }

private:
    const Block & block_;
};

//储存了校准过程中不断提交的新数据
struct CalibrateDataVector{
    constexpr CalibrateDataVector(const size_t capacity):
        capacity_(capacity){
            reset();
        }
    constexpr Result<void, void> push_back(const q31 targ, const q31 meas){
        //确定原始数据的扇区
        let index = position_to_index(meas);
        // PANIC(index);
        if(index >= capacity_) return Err();
        if(cnts_[index] != 0) return Err(); 

        block_[index] = ({
            let opt = PackedCalibratePoint::from_targ_and_meas(
                targ, meas
            );

            if(opt.is_none()) return Err();
            opt.unwrap();
        });

        return Ok();
    }

    constexpr size_t position_to_index(const q31 position) const{
        return size_t(q24(position) * capacity_);
    }

    constexpr PackedCalibratePoint operator[](const size_t idx) const {
        return block_[idx];
    }

    constexpr PackedCalibratePoint operator[](const q31 raw_position) const {
        return block_[position_to_index(raw_position)];
    }

    constexpr void reset(){
        block_.fill(PackedCalibratePoint{});
        cnts_.fill(0);
    }

    constexpr bool is_full() const {
        // return cnt_ == capacity_;
        return std::accumulate(cnts_.begin(), cnts_.end(), 0u) == capacity_;
    }

    constexpr CalibrateDataBlockView
    as_view() const {
        return CalibrateDataBlockView(block_);
    }


private:
    using Block = CalibrateDataBlock;
    using Cnts = std::array<uint8_t, MOTOR_POLE_PAIRS>;

    Block block_;
    Cnts cnts_;
    size_t capacity_ = 0;
};

struct EncoderCorrector{
private:
    // using Storage = std::array<>
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


struct MotorTasksUtils{
    enum class TaskError:uint8_t{
        TaskNotDone,
        RotorIsMovingBeforeChecking,
        RotorIsMovingBeforeCheckingCoilA,
        RotorIsMovingBeforeCheckingCoilB,
        CoilCantMove,
        CoilACantMove,
        CoilBCantMove,
    };




    FRIEND_DERIVE_DEBUG(TaskError)

    // static constexpr auto DRIVE_DUTY = 0.3_r;
    static constexpr auto CALIBRATE_DRIVE_DUTY = 0.3_r;
    static constexpr auto STALL_DRIVE_DUTY = 0.3_r;
    static constexpr auto BEEP_DRIVE_DUTY = 0.3_r;

    static constexpr size_t MICROSTEPS_PER_SECTOR = 256;

    // static constexpr q24 ticks_to_seconds(const size_t ticks){
    //     return 
    // }
    static constexpr size_t seconds_to_ticks(const q16 seconds){
        return size_t(seconds * ISR_FREQ);
    }

    static constexpr q16 ticks_to_linear_position(const size_t ticks){
        return q16(ticks) / MICROSTEPS_PER_SECTOR / MOTOR_POLE_PAIRS / 4;
    }

    static constexpr size_t linear_position_to_ticks(const q16 rotations){
        return size_t(rotations * MICROSTEPS_PER_SECTOR) * 4 * MOTOR_POLE_PAIRS;
    }

    // static constexpr q16 ticks_to_accdec_rotations(const size_t ticks){
    //     return 
    // }

    static constexpr size_t accdec_rotations_to_ticks(const q16 rotations){
        return linear_position_to_ticks(rotations) * 2;
    }

    struct StallTask final{

        struct Config{
            using Task = StallTask;

            q16 targ_elec_rotation;
            size_t timeout_ticks;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr StallTask(const Config & cfg){
            targ_elec_rotation_ = cfg.targ_elec_rotation;
            ticks_ = cfg.timeout_ticks;
        } 

        constexpr AlphaBetaDuty resume(const real_t lappos){
            tick_cnt_++;

            let [s,c] = sincospu(targ_elec_rotation_);

            return AlphaBetaDuty{
                .alpha = c * STALL_DRIVE_DUTY,
                .beta = s * STALL_DRIVE_DUTY
            };
        }

        constexpr bool is_finished(){
            return tick_cnt_ > ticks_;
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }
    private:
        size_t tick_cnt_ = 0;

        q16 targ_elec_rotation_;
        size_t ticks_;
    };

    struct BeepTask final{

        struct Config{
            using Task = BeepTask;

            // q16 delta_position;
            size_t freq;
            Milliseconds period;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr BeepTask(const Config & cfg):
            freq_(cfg.freq),
            period_(cfg.period){
            ;
        }

        constexpr AlphaBetaDuty resume(const real_t meas_lap_position){
            tick_cnt_++;
            let targ_lap_position = ticks_to_linear_position(tick_cnt_);
            let [s,c] = sincospu(0.25_r * sinpu(targ_lap_position * freq_));
            return AlphaBetaDuty{
                .alpha = BEEP_DRIVE_DUTY * 2,
                .beta = s * BEEP_DRIVE_DUTY * 2
            };
        }

        constexpr bool is_finished(){
            return tick_cnt_ > period_.count() * 100;
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }
    private:
        size_t tick_cnt_ = 0;

        size_t freq_;
        Milliseconds  period_;
    };


    struct LinearRotateTask final{

        struct Config{
            using Task = LinearRotateTask;

            q16 delta_position;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr LinearRotateTask(const Config & cfg):
            delta_(cfg.delta_position){
            ;
        } 

        constexpr AlphaBetaDuty resume(const real_t meas_lap_position){
            let targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_);
            let [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
            tick_cnt_++;
            return AlphaBetaDuty{
                .alpha = c * STALL_DRIVE_DUTY,
                .beta = s * STALL_DRIVE_DUTY
            };
        }

        constexpr bool is_finished(){
            return tick_cnt_ > linear_position_to_ticks(ABS(delta_));
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }
    private:
        size_t tick_cnt_ = 0;

        q16 delta_;
    };


    struct AccDecRotateTask final{

        struct Config{
            using Task = AccDecRotateTask;

            q16 delta_position;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr AccDecRotateTask(const Config & cfg):
            delta_(cfg.delta_position){
            ;
        } 

        constexpr AlphaBetaDuty resume(const real_t meas_lap_position){
            let targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_);
            let [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
            tick_cnt_++;
            return AlphaBetaDuty{
                .alpha = c * STALL_DRIVE_DUTY,
                .beta = s * STALL_DRIVE_DUTY
            };
        }

        constexpr bool is_finished(){
            return tick_cnt_ > linear_position_to_ticks(ABS(delta_));
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }
    private:
        size_t tick_cnt_ = 0;

        q16 delta_;
    };
};

    struct TaskExecuter{
    template<typename T>
        static constexpr Result<void, MotorTasksUtils::TaskError>execute(T && obj){
            return Err(MotorTasksUtils::TaskError::TaskNotDone);
        }
    };

struct CoilCheckTasksUtils:public MotorTasksUtils{
    static constexpr auto MINIMAL_MOVING_THRESHOLD = 0.003_r;
    static constexpr auto MINIMAL_STALL_THRESHOLD = 0.0003_r;
    static constexpr auto STALL_CHECK_TICKS = 80u;
    static constexpr auto MOVE_CHECK_TICKS = 1600u;
    static constexpr auto STALL_TICKS = 1000u;

    struct CheckStallTask final{

        struct Config{
            using Task = CheckStallTask;
        };

        struct Dignosis {
            Option<TaskError> err;
            Range2<q16> move_range;
        };
        
        constexpr CheckStallTask(const Config & cfg){;} 

        constexpr AlphaBetaDuty resume(const real_t cont_position){

            if(may_move_range_.is_none())
                may_move_range_ = Some(Range2<q16>::from_center(cont_position));
            else 
                may_move_range_ = Some(may_move_range_.unwrap().merge(cont_position));
            tick_cnt_++;
            return AlphaBetaDuty{
                .alpha = 0,
                .beta = 0
            };
        }

        constexpr bool is_finished(){

            return tick_cnt_ > STALL_CHECK_TICKS;
        }

        constexpr Dignosis dignosis() const {
            ASSERT(may_move_range_.is_some());
            let move_range = may_move_range_.unwrap();
            
            auto make_err = [&]() -> Option<TaskError>{
                if(move_range.length() > MINIMAL_STALL_THRESHOLD)
                    Some(TaskError::RotorIsMovingBeforeChecking);
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
            using Task = CheckMovingTask;
            const bool is_beta;
        };

        struct [[nodiscard]] Dignosis {
            Option<TaskError> err;
            Range2<q16> move_range;
        };

        constexpr CheckMovingTask(const Config & cfg){
            is_beta_ = cfg.is_beta;
        };

        constexpr AlphaBetaDuty resume(const real_t cont_position){

            if(may_move_range_.is_none())
                may_move_range_ = Some(Range2<q16>::from_center(cont_position));
            else 
                may_move_range_ = Some(may_move_range_.unwrap().merge(cont_position));

            let duty = sinpu(LERP(
                q16(tick_cnt_) / MOVE_CHECK_TICKS,
                -0.5_r, 0.5_r
            )) * STALL_DRIVE_DUTY;

            auto make_duty = [&]() -> AlphaBetaDuty{
                if(is_beta_){
                    return {duty, 0};
                }else{
                    return {0, duty};
                }
            };
            tick_cnt_++;
            return make_duty();
        }

        constexpr bool is_finished(){
            return tick_cnt_ > MOVE_CHECK_TICKS;
        }

        constexpr Dignosis dignosis() const {
            // ASSERT(may_move_range_.is_some());

            let move_range = may_move_range_.unwrap();

            [[maybe_unused]] auto make_err = [&]() -> Option<TaskError>{
                const bool is_ok = move_range.length() > MINIMAL_MOVING_THRESHOLD;
                if(is_ok)
                    return None;
                return Some(is_beta_ ? TaskError::CoilBCantMove : TaskError::CoilACantMove);
            };

            return Dignosis{
                .err = make_err(),
                // .err = Some(TaskError::CoilACantMove),
                .move_range = move_range
            };
        }
    private:
        size_t tick_cnt_ = 0;
        Option<Range2<q16>> may_move_range_ = None;
        bool is_beta_;
    };
};



struct CalibrateTasksUtils:public MotorTasksUtils{

    template<size_t N>
    struct AverageHelper{
        constexpr Result<void, void> push_back(const q24 val){
            if(cnt_ >= N)
                return Err();
            sum_ += val;
            cnt_++;
            return Ok();
        }

        constexpr Option<q24> get_avg(){
            if(cnt_ != N)
                return None;
            return Ok(sum_ / N);
        }

        constexpr void reset(){
            sum_ = 0;
            cnt_ = 0;
        }
    private:
        q24 sum_ = 0;
        size_t cnt_ = 0;
    };


    struct CalibrateRotateTask final{

        struct Config{
            using Task = CalibrateRotateTask;

            CalibrateDataVector & vector;
            q16 delta_position;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr CalibrateRotateTask(const Config & cfg):
            vector_(cfg.vector),
            delta_position_(cfg.delta_position){
            ;
        }

        constexpr CalibrateRotateTask(const CalibrateRotateTask &) = default;
        constexpr CalibrateRotateTask(CalibrateRotateTask &&) = default;

        constexpr CalibrateRotateTask & operator = (const CalibrateRotateTask & ) = default;
        constexpr CalibrateRotateTask & operator = (CalibrateRotateTask &&) = default;

        constexpr AlphaBetaDuty resume(const real_t meas_lap_position){
            let targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_position_);
            let [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
            
            tick_cnt_++;

            if(tick_cnt_ % (MICROSTEPS_PER_SECTOR * 4) == 0){
                let res = push_data(targ_lap_position, meas_lap_position);
                // if(res.is_err()) PANIC(vector_.get().as_view());
                if(res.is_err()) PANIC();
                    // .expect(vector_.get().size(), 80);
            }


            return AlphaBetaDuty{
                .alpha = c * CALIBRATE_DRIVE_DUTY,
                .beta = s * CALIBRATE_DRIVE_DUTY
            };
        }

        constexpr bool is_finished(){
            return tick_cnt_ >= linear_position_to_ticks(ABS(delta_position_));
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }

    private:
        constexpr Result<void, void> push_data(
            const real_t targ_lap_position,
            const real_t meas_lap_position
        ){
            return vector_.get().push_back(targ_lap_position, meas_lap_position);
        }

        size_t tick_cnt_ = 0;
        
        std::reference_wrapper<CalibrateDataVector> vector_;
        q16 delta_position_;
    };

};





template<typename TaskSettings>
struct TaskSequence final{
    using Configs = std::decay_t<typename TaskSettings::Configs>;
    using TaskError = typename TaskSettings::Error;
    using Args = typename TaskSettings::Args;
    using Ret = typename TaskSettings::Ret;
    using TasksVariant = configs_tuple_to_tasks_variant_t<Configs>;
    using DignosisVariant = configs_tuple_to_dignosis_variant_t<Configs>;

    constexpr TaskSequence(const Configs & configs):
        CONFIGS(configs){;}
    constexpr Ret resume(const Args cont_position){
        std::visit([&](auto && task) -> void{
            if(not task.is_finished()) return;

            let dignosis = task.dignosis();
            if(dignosis.err.is_some()){
                save_dignosis(dignosis);
                is_all_tasks_finished_ = true;
                return;
            }
            
            let res = switch_to_next_task();
            if(res.is_err()){
                is_all_tasks_finished_ = true;
            }
        }, tasks_variant_);

        return std::visit([&](auto && task) -> AlphaBetaDuty {
            return task.resume(cont_position);
        }, tasks_variant_);
    };

    constexpr Option<TaskError> err() const {
        if(may_dignosis_variant_.is_none())
            return None;
        
        return std::visit([&](auto && dignosis) -> Option<TaskError> {
            return dignosis.err;
        }, may_dignosis_variant_.unwrap());
    }

    constexpr bool is_finished() const {
        return is_all_tasks_finished_;
    }

    constexpr bool is_done() const {
        return is_all_tasks_finished_ and err().is_none();
    }

    consteval size_t task_count() const {
        return std::tuple_size_v<Configs>;
    }

    constexpr size_t task_index() const {
        return task_index_;
    }

    const TasksVariant & tasks_variant() const {
        return tasks_variant_;
    }
private:
    template<size_t I>
    constexpr void switch_to_task_impl(){
        static constexpr size_t N = std::tuple_size_v<Configs>;
        static_assert(I < N, "Invalid task index");
        using Task = idx_to_task_t<I, Configs>;
        let & config = std::get<I>(CONFIGS);
        tasks_variant_ = Task(config);
    }

    constexpr Result<void, void> switch_to_task(const size_t i){
        constexpr size_t N = std::tuple_size_v<Configs>;

        if (i >= N)//last task
            return Err();
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (( (Is == i) ? 
                (switch_to_task_impl<Is>(), 0) : 0 ), ...);
        }(std::make_index_sequence<N>());

        return Ok();
    }

    template<typename Dignosis>
    constexpr void save_dignosis(const Dignosis & dignosis){
        may_dignosis_variant_ = Some(dignosis);
    }

    constexpr Result<void, void> switch_to_next_task(){
        let next_task_index = task_index_ + 1;
        let res =  switch_to_task(next_task_index);
        if(res.is_ok()) task_index_ = next_task_index;
        return res;
    }


    const Configs CONFIGS;
    TasksVariant tasks_variant_ = {std::get<0>(CONFIGS)};
    Option<DignosisVariant> may_dignosis_variant_ = None;
    size_t task_index_ = 0;
    std::atomic<bool> is_all_tasks_finished_ = false;
};


class CoilMotionCheckTasks:public CoilCheckTasksUtils{
public:
    static constexpr auto CONFIGS = std::make_tuple(
        //令转子停下
        StallTask::Config{
            .targ_elec_rotation = 0,
            .timeout_ticks = STALL_TICKS
        },
    
        // 检测转子已经停下
        CheckStallTask::Config{},

        // 检测转子是否能够在A相的驱使下运动
        CheckMovingTask::Config{
            .is_beta = false
        },

        //令转子停下
        StallTask::Config{
            .targ_elec_rotation = 0,
            .timeout_ticks = STALL_TICKS
        },

        // 检测转子已经停下
        CheckStallTask::Config{},

        // 检测转子是否能够在B相的驱使下运动
        CheckMovingTask::Config{
            .is_beta = true
        }
    );

    constexpr AlphaBetaDuty resume(const real_t lap_position){
        return task_sequence_.resume(lap_position);
    }

    bool is_finished() const {
        return task_sequence_.is_finished();
    }

    size_t task_index() const {
        return task_sequence_.task_index();
    }

    auto err() const {
        return task_sequence_.err();
    }

private:
    struct Settings{
        using Configs = std::decay_t<decltype(CONFIGS)>;
        using Error = TaskError;
        using Args = real_t;
        using Ret = AlphaBetaDuty;
    };

    TaskSequence<Settings> task_sequence_ = {CONFIGS};
};

auto make_calibrate_configs = [](
        CalibrateDataVector & forward_calibrate_data_vector,
        CalibrateDataVector & backward_calibrate_data_vector
    ) -> auto{
    return std::make_tuple(
        // LinearRotateTask::Config{
        //     .delta_position = 0.4_r
        // },
    
        // LinearRotateTask::Config{
        //     .delta_position = -0.4_r
        // },

        // CalibrateTasksUtils::LinearRotateTask::Config{
        //     .delta_position = 0.4_r
        // },
    
        // CalibrateTasksUtils::LinearRotateTask::Config{
        //     .delta_position = -0.4_r
        // }
        // ,
        CalibrateTasksUtils::CalibrateRotateTask::Config{
            .vector = forward_calibrate_data_vector,
            .delta_position = 1_r
        },

        CalibrateTasksUtils::CalibrateRotateTask::Config{
            .vector = backward_calibrate_data_vector,
            .delta_position = -1_r
        }



        // CalibrateTasksUtils::CalibrateRotateTask::Config{
        //     .vector = calibrate_data_vector,
        //     .delta_position = -0.4_r
        // }
    );
};


class CalibrateTasks:public CalibrateTasksUtils{
public:
    template<typename ... Args>
    static constexpr std::tuple<Args...> make_config(Args && ... args){
        return std::make_tuple(
            (std::forward<Args>(args), ...)
        );
    }

    using IConfigs = magic::functor_ret_t<decltype(make_calibrate_configs)>;

    IConfigs configs_;


    struct Settings{
        using Configs = IConfigs;
        using Error = TaskError;
        using Args = real_t;
        using Ret = AlphaBetaDuty;
    };
    TaskSequence<Settings> task_sequence_ = {configs_};

    constexpr CalibrateTasks(
        CalibrateDataVector & forward_calibrate_data_vector,
        CalibrateDataVector & backward_calibrate_data_vector
    ):
        configs_(make_calibrate_configs(
            forward_calibrate_data_vector,
            backward_calibrate_data_vector
        )){;}

    constexpr AlphaBetaDuty resume(const real_t lap_position){
        return task_sequence_.resume(lap_position);
    }

    bool is_finished() const {
        return task_sequence_.is_finished();
    }

    size_t task_index() const {
        return task_sequence_.task_index();
    }

    TaskSequence<Settings> & task_sequence(){
        return task_sequence_;
    }

    using DignosisVariant = typename TaskSequence<Settings>::DignosisVariant;

    
    static constexpr size_t TASK_COUNT = std::tuple_size_v<IConfigs>;

    template<size_t I>
    auto get_task(){
        static_assert(I < TASK_COUNT);
        return std::get<
            typename std::tuple_element_t<I, IConfigs>::Task>(
            task_sequence_.tasks_variant());
    }
    // consteval size_t task_count() const {
    //     // return std::tuple_size_v<IConfigs>;
    //     return 2;
    // }
    auto err() const {
        return task_sequence_.err();
    }

    auto operator >>=(auto && obj){
        return 0;
    }
private:

};

class BeepTasks:public MotorTasksUtils{
public:
    static constexpr auto CONFIGS = std::make_tuple(
        BeepTask::Config{
            .freq = 700,
            .period = 100ms
        },
        BeepTask::Config{
            .freq = 0,
            .period = 100ms
        },
        BeepTask::Config{
            .freq = 800,
            .period = 100ms
        },
        BeepTask::Config{
            .freq = 0,
            .period = 100ms
        },
        BeepTask::Config{
            .freq = 500,
            .period = 100ms
        },
        BeepTask::Config{
            .freq = 0,
            .period = 100ms
        }
    );

    constexpr AlphaBetaDuty resume(const real_t lap_position){
        return task_sequence_.resume(lap_position);
    }

    bool is_finished() const {
        return task_sequence_.is_finished();
    }

    size_t task_index() const {
        return task_sequence_.task_index();
    }

    auto err() const {
        return task_sequence_.err();
    }

private:
    struct Settings{
        using Configs = std::decay_t<decltype(CONFIGS)>;
        using Error = TaskError;
        using Args = real_t;
        using Ret = AlphaBetaDuty;
    };

    TaskSequence<Settings> task_sequence_ = {CONFIGS};
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

            const auto res = TaskExecuter::execute(TaskSpawner::spawn(
                CalibrateTasks{forward_cali_vec_, backward_cali_vec_}
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

            // let [a,b] = sincospu(frac(meas_lap_position - 0.009_r) * 50);
            // let [s,c] = sincospu(frac(-(meas_lap_position - 0.019_r + 0.01_r)) * 50);
            let [s,c] = sincospu(frac(
                // (correct_raw_position(meas_lap_position) - 0.007_r)) * 50);
                (correct_raw_position(meas_lap_position) - 0.007_r)) * 50);
            // let [a,b] = sincospu( - 0.004_r);
            let mag = 0.5_r;

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
        // DEBUG_PRINTLN(calibrate_data_vector_);
        // let view = calibrate_data_vector_.as_view();

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

        print_view(forward_cali_vec_.as_view());
        print_view(backward_cali_vec_.as_view());

        for(int i = 0; i < 50; i++){
            let raw = real_t(i) / 50;
            let corrected = correct_raw_position(raw);
            DEBUG_PRINTLN(raw, corrected, (corrected - raw) * 100);
            clock::delay(1ms);
        }

        return Ok();
    }

    constexpr q16 correct_raw_position(const q16 raw_position) const {
        let corr1 = forward_cali_vec_[raw_position].to_inaccuracy();
        let corr2 = backward_cali_vec_[raw_position].to_inaccuracy();

        return raw_position + mean(corr1, corr2);
        // return raw_position + corr1;
    }
    Microseconds execution_time_ = 0us;
private:
    drivers::EncoderIntf & encoder_;
    StepperSVPWM & svpwm_;

    CoilMotionCheckTasks coil_motion_check_comp_ = {};
    CalibrateTasks calibrate_comp_ = {
        forward_cali_vec_,
        backward_cali_vec_
    };

    CalibrateDataVector forward_cali_vec_ = {MOTOR_POLE_PAIRS};
    CalibrateDataVector backward_cali_vec_ = {MOTOR_POLE_PAIRS};
    std::atomic<bool> is_comp_finished_ = false;


};


void test_calibrate(){
    const CalibrateDataVector vec{MOTOR_POLE_PAIRS};
    // ... fill data ...
    let view = vec.as_view();
    for (let [targ, meas] : view) {
        DEBUG_PRINTLN(targ, meas);
    }
    while(true);
}


void test_check(drivers::EncoderIntf & encoder,StepperSVPWM & svpwm){
    auto motor_system_ = MotorSystem{encoder, svpwm};

    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&](){
        let res = motor_system_.resume();
        if(res.is_err()){
            PANIC();
        }
    });


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
        DEBUG_PRINTLN_IDLE(motor_system_.execution_time_.count());
    }

    // DEBUG_PRINTLN("finished");

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


    q20 targ_lappos = 0;
    real_t meas_lappos = 0;
    timer.attach(hal::TimerIT::Update, {0,0}, [&](){
        // retry(1, [&]{return encoder.update();}).examine();
        retry(2, [&]{return encoder.update();}).examine();
        meas_lappos = encoder.get_lap_position().examine();
        // DEBUG_PRINTLN(drivers::EncoderError::Kind::CantSetup);
        // let t = clock::time();
        // let [st, ct] = sincospu(t * 93);
        // let [st, ct] = sincospu(t * 3);
        // let [st, ct] = sincospu(10 * sinpu(t));

        targ_lappos += (4 * meas_lappos - 2) * 0.00005_q20;
        let ref_epos = MOTOR_POLE_PAIRS * q16(targ_lappos);
        let [st, ct] = sincospu(ref_epos);
        // let [st, ct] = sincospu(sinpu(t));
        let amp = 0.6_r;

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
            100 * (targ_lappos - frac(4.4_r * clock::time())),
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