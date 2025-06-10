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


// template<typename Tuple, typename F, size_t... Is>
// constexpr auto tuple_transform_impl(Tuple&& t, F&& f, std::index_sequence<Is...>) {
//     return std::make_tuple(f(std::get<Is>(std::forward<Tuple>(t)))...);
// }

// template<typename Tuple, typename F>
// constexpr auto tuple_transform(Tuple&& t, F&& f) {
//     return tuple_transform_impl(std::forward<Tuple>(t), 
//         std::forward<F>(f),
//         std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
// }


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


struct CoilMotionCheckComponentUtils{
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

    static constexpr auto DRIVE_DUTY = 0.1_r;
    static constexpr auto MINIMAL_MOVING_THRESHOLD = 0.003_r;
    static constexpr auto MINIMAL_STILL_THRESHOLD = 0.0003_r;
    static constexpr auto STILL_CHECK_TICKS = 80u;
    static constexpr auto MOVE_CHECK_TICKS = 1600u;
    static constexpr auto STILL_TICKS = 1000u;



    struct StallTask final{

        struct Config{
            using Task = StallTask;

            bool is_beta;
        };

        
        struct Dignosis {
            Option<TaskError> err;
        };
        
        constexpr StallTask(const Config & cfg){
            is_beta_ = cfg.is_beta;
        } 

        constexpr AlphaBetaDuty resume(const real_t lappos){
            tick_cnt_++;

            if(not is_beta_)
                return AlphaBetaDuty{
                    .alpha = DRIVE_DUTY,
                    .beta = 0
                };
            else
                return AlphaBetaDuty{
                    .alpha = 0,
                    .beta = DRIVE_DUTY
                };
        }

        constexpr bool is_finished(){
            return tick_cnt_ > STILL_TICKS;
        }

        constexpr Dignosis dignosis() const {
            return Dignosis{
                .err = None,
            };
        }
    private:
        size_t tick_cnt_ = 0;

        Option<Range2<q16>> may_move_range_ = None;

        bool is_beta_;
    };

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

            return tick_cnt_ > STILL_CHECK_TICKS;
        }

        constexpr Dignosis dignosis() const {
            ASSERT(may_move_range_.is_some());
            const auto move_range = may_move_range_.unwrap();
            
            auto make_err = [&]() -> Option<TaskError>{
                if(move_range.length() > MINIMAL_STILL_THRESHOLD)
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

            const auto duty = sinpu(LERP(
                q16(tick_cnt_) / MOVE_CHECK_TICKS,
                -0.5_r, 0.5_r
            )) * DRIVE_DUTY;

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

            const auto move_range = may_move_range_.unwrap();

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

            const auto dignosis = task.dignosis();
            if(dignosis.err.is_some()){
                save_dignosis(dignosis);
                is_all_tasks_finished_ = true;
                return;
            }
            
            const auto res = switch_to_next_task();
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
private:
    template<size_t I>
    constexpr void switch_to_task_impl(){
        static constexpr size_t N = std::tuple_size_v<Configs>;
        static_assert(I < N, "Invalid task index");
        using Task = idx_to_task_t<I, Configs>;
        const auto & config = std::get<I>(CONFIGS);
        tasks_variant_ = Task(config);
    }

    constexpr Result<void, void> switch_to_task(const size_t i){
        constexpr size_t N = std::tuple_size_v<Configs>;

        if (i >= N)//last task
            return Err();

        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (( (Is == i) ? 
                (switch_to_task_impl<Is>(), 0) : 
                0 ), ...);
        }(std::make_index_sequence<N>());

        return Ok();
    }

    template<typename Dignosis>
    constexpr void save_dignosis(const Dignosis & dignosis){
        may_dignosis_variant_ = Some(dignosis);
    }

    constexpr Result<void, void> switch_to_next_task(){
        const auto next_task_index = task_index_ + 1;
        const auto res =  switch_to_task(next_task_index);
        if(res.is_ok()) task_index_ = next_task_index;
        return res;
    }

    const Configs CONFIGS;
    TasksVariant tasks_variant_ = {std::get<0>(CONFIGS)};
    Option<DignosisVariant> may_dignosis_variant_ = None;
    size_t task_index_ = 0;
    bool is_all_tasks_finished_ = false;
};


class CoilMotionCheckTasks:public CoilMotionCheckComponentUtils{
public:
    static constexpr auto CONFIGS = std::make_tuple(
        //令转子停下
        StallTask::Config{
            .is_beta = false
        },
    
        // 检测转子已经停下
        CheckStallTask::Config{},

        // 检测转子是否能够在A相的驱使下运动
        CheckMovingTask::Config{
            .is_beta = false
        },

        //令转子停下
        StallTask::Config{
            .is_beta = true
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

class MotorSystem{
public:
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

        const auto lap_position = ({
            if(const auto res = retry(2, [&]{return encoder_.update();});
                res.is_err()) return Err(Error(res.unwrap_err()));

            const auto either_lap_position = encoder_.get_lap_position();
            if(either_lap_position.is_err())
                return Err(Error(either_lap_position.unwrap_err()));
            either_lap_position.unwrap();
        });

        auto & comp = comp_;
        if(const auto may_err = comp.err(); may_err.is_some())
            return Err(Error(may_err.unwrap()));

        if(comp.is_finished())
            return Ok();

        const auto [a,b] = comp.resume(lap_position);
        svpwm_.set_alpha_beta_duty(a,b);
        return Ok();

    }

private:
    drivers::EncoderIntf & encoder_;
    StepperSVPWM & svpwm_;

    CoilMotionCheckTasks comp_ = {};
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


void test_check(drivers::EncoderIntf & encoder,StepperSVPWM & svpwm){
    auto motor_system_ = MotorSystem{encoder, svpwm};

    while(true){
        // encoder.update();


        const auto res = motor_system_.resume();
        if(res.is_err()){
            PANIC();
        }
        clock::delay(1ms);
        // DEBUG_PRINTLN(
        //     clock::millis().count(), 
        //     comp.task_index(), 
        //     comp.is_finished(),
        //     comp.err().is_some()
        // );
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

    test_check(encoder, svpwm);


    q20 targ_lappos = 0;
    real_t meas_lappos = 0;
    timer.attach(hal::TimerIT::Update, {0,0}, [&](){
        // retry(1, [&]{return encoder.update();}).examine();
        retry(2, [&]{return encoder.update();}).examine();
        meas_lappos = encoder.get_lap_position().examine();
        // DEBUG_PRINTLN(drivers::EncoderError::Kind::CantSetup);
        // const auto t = clock::time();
        // const auto [st, ct] = sincospu(t * 93);
        // const auto [st, ct] = sincospu(t * 3);
        // const auto [st, ct] = sincospu(10 * sinpu(t));

        targ_lappos += (4 * meas_lappos - 2) * 0.00005_q20;
        let ref_epos = MOTOR_POLES * q16(targ_lappos);
        const auto [st, ct] = sincospu(ref_epos);
        // const auto [st, ct] = sincospu(sinpu(t));
        const auto amp = 0.6_r;

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