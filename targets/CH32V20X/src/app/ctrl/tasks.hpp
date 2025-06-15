#pragma once

#include "core/utils/Errno.hpp"
#include "core/math/realmath.hpp"

#include "types/regions/range2/range2.hpp"

#include "meta_utils.hpp"
#include "calibrate_utils.hpp"

namespace ymd{

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


struct MotorUtils{
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


    enum class ServiceError:uint8_t{
        HwErrorSavingArchive,
        HwErrorLoadingArchive,
        SavingArchiveHasBeenStarted,
        LoadingArchiveHasBeenStarted,
        ArchiveIsCorrupted
    };

    FRIEND_DERIVE_DEBUG(ServiceError)

    // static constexpr auto DRIVE_DUTY = 0.3_r;
    static constexpr auto CALIBRATE_DRIVE_DUTY = 0.3_r;
    static constexpr auto STALL_DRIVE_DUTY = 0.3_r;
    static constexpr auto BEEP_DRIVE_DUTY = 0.3_r;

    static constexpr size_t MICROSTEPS_PER_SECTOR = 256;

    static constexpr size_t seconds_to_ticks(const q16 seconds){
        return size_t(seconds * ISR_FREQ);
    }

    static constexpr q16 ticks_to_linear_position(const size_t ticks){
        return q16(ticks) / MICROSTEPS_PER_SECTOR / MOTOR_POLE_PAIRS / 4;
    }

    static constexpr size_t linear_position_to_ticks(const q16 rotations){
        return size_t(rotations * MICROSTEPS_PER_SECTOR) * 4 * MOTOR_POLE_PAIRS;
    }

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

            const auto [s,c] = sincospu(targ_elec_rotation_);

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
            const auto targ_lap_position = ticks_to_linear_position(tick_cnt_);
            const auto [s,c] = sincospu(0.25_r * sinpu(targ_lap_position * freq_));
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
            const auto targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_);
            const auto [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
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
            const auto targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_);
            const auto [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
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
        static constexpr Result<void, MotorUtils::TaskError>execute(T && obj){
            return Err(MotorUtils::TaskError::TaskNotDone);
        }
    };

struct CoilCheckTasksUtils:public MotorUtils{
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
            const auto move_range = may_move_range_.unwrap();
            
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

            const auto duty = sinpu(LERP(
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



struct CalibrateTasksUtils:public MotorUtils{

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
            const auto targ_lap_position = SIGN_AS(ticks_to_linear_position(tick_cnt_), delta_position_);
            const auto [s,c] = sincospu(targ_lap_position * MOTOR_POLE_PAIRS);
            
            tick_cnt_++;

            if(tick_cnt_ % (MICROSTEPS_PER_SECTOR * 4) == 0){
                const auto res = push_data(targ_lap_position, meas_lap_position);
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

    const TasksVariant & tasks_variant() const {
        return tasks_variant_;
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
                (switch_to_task_impl<Is>(), 0) : 0 ), ...);
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

class BeepTasks:public MotorUtils{
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


}