#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/async/timer.hpp"
#include "core/utils/Unit.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/utils/split_iter.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"

#include "algebra/vectors/polar.hpp"
#include "algebra/vectors/vec2.hpp"

#include "common_service.hpp"
#include "joints.hpp"
#include "robots/gcode/gcode.hpp"
#include "details/gcode_file.hpp"

#ifdef UART1_PRESENT
using namespace ymd;

using namespace ymd::robots;
using robots::zdtmotor::ZdtStepper;

#define DBG_UART hal::usart2
#define COMM_UART hal::usart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

//CAN1 TX/PB9 RX/PB8

#define DEF_COMMAND_BIND(K, T) \
template<> \
struct command_to_kind<CommandKind, T>{ \
    static constexpr CommandKind KIND = K; \
};\
template<> \
struct kind_to_command<CommandKind, K>{ \
    using type = T; \
};



class PolarRobotActuator{
public:
    struct Config{
        iq16 rho_transform_scale;
        iq16 theta_transform_scale;
        Angular<iq16> center_bias;

        Range2<iq16> rho_range;
        Range2<iq16> theta_range;
    };

    struct Params{
        Some<JointMotorActuatorIntf *> radius_joint_;
        Some<JointMotorActuatorIntf *> theta_joint_;
    };


    PolarRobotActuator(
        const Config & cfg, 
        const Params & params
    ):
        cfg_(cfg),
        joint_rho_(params.radius_joint_.deref()),
        joint_theta_(params.theta_joint_.deref())
    {;}


    void set_coord(const Polar<iq16> p){

        const auto rho_angle = Angular<iq16>::from_turns(
            p.amplitude * cfg_.rho_transform_scale);

        const auto theta_angle = 
            p.phase * cfg_.theta_transform_scale;

        joint_rho_.set_angle(rho_angle - cfg_.center_bias);
        joint_theta_.set_angle(-theta_angle);
    }

    void activate(){
        joint_rho_.activate();
        joint_theta_.activate();
    }

    void deactivate(){
        joint_rho_.deactivate();
        joint_theta_.deactivate();
    }

    void trig_homing(){
        joint_rho_.trig_homing();   
        joint_theta_.trig_homing();
    }

    bool is_homing_done(){
        return joint_rho_.is_homing_done() 
            && joint_theta_.is_homing_done();
    }

    auto make_repl_list(const StringView name){
        return repl::make_list(
            name,
            DEF_CALLABLE_MEMFUNC(trig_homing),
            DEF_CALLABLE_MEMFUNC(is_homing_done),
            DEF_CALLABLE_MEMFUNC(deactivate),
            DEF_CALLABLE_MEMFUNC(activate)
            // DEF_CALLABLE_MEMFUNC(set_coord)
        );
    }

private:
    const Config cfg_;
    JointMotorActuatorIntf & joint_rho_;
    JointMotorActuatorIntf & joint_theta_;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        DEBUG_PRINTLN(std::forward<Args>(args)...);
    }

};

struct Cartesian2ContinuousPolarRegulator final {
    struct State {
        Vec2<iq16> coord;
        Angular<iq16> angle;  // 累积角度
    };

    Polar<iq16> operator()(const Vec2<iq16> coord) {
        if (may_last_state_.is_none()) {
            // 第一次调用，初始化状态
            may_last_state_ = Some(State{
                .coord = coord,
                .angle = coord.angle()  // 初始角度
            });
            return Polar<iq16>{coord.length(), coord.angle()};
        }

        // 获取上次状态
        const auto last_state = may_last_state_.unwrap();
        
        // 计算角度变化
        const auto delta_theta = last_state.coord.angle_between(coord);
        const auto new_theta = last_state.angle + delta_theta;

        // DEBUG_PRINTLN(last_state.coord, coord, delta_theta);

        // 更新状态
        may_last_state_ = Some(State{
            .coord = coord,
            .angle = new_theta  // 存储累积角度
        });

        return Polar<iq16>{coord.length(), new_theta};
    }

private:
    Option<State> may_last_state_ = None;
};


template<typename T>
constexpr Vec2<T> vec_step_to(const Vec2<T> from, const Vec2<T> to, T step){
    Vec2<T> delta = to - from;
    T distance = delta.length();
    if (distance <= step) return to;  // 如果步长足够大，直接到达目标
    return from + delta * (step / distance);  // 按比例移动
}



struct StepPointIterator{
    struct Config{
        Vec2<iq24> initial_coord;
    };

    explicit constexpr StepPointIterator(
        const Config & cfg
    ):
        now_coord_(cfg.initial_coord){;}

    constexpr void set_target_coord(
        const Vec2<iq24> target_coord
    ) {
        may_end_coord_ = Some(target_coord);
    }

    [[nodiscard]] constexpr Vec2<iq24> next(const iq24 step){
        if(may_end_coord_.is_none()) return now_coord_;
        now_coord_ = vec_step_to(now_coord_, may_end_coord_.unwrap(), step);
        return now_coord_;
    }

    [[nodiscard]] constexpr bool has_next() const {
        return may_end_coord_.is_some() and 
            (not now_coord_.is_equal_approx(may_end_coord_.unwrap(), 0.001_iq16));
    }
private:
    Vec2<iq24> now_coord_ = Vec2<iq24>::ZERO;
    Option<Vec2<iq24>> may_end_coord_ = None;
};


struct GcodeStateHolder{
    static constexpr auto X_LIMIT = 0.2_r;
    static constexpr auto Y_LIMIT = 0.2_r;

    iq16 max_speed = 0.02_r;
    iq16 speed;
    unit::Meter<iq16> x = unit::Meter<iq16>(0);
    unit::Meter<iq16> y = unit::Meter<iq16>(0);
    bool is_rapid;

    constexpr void set_speed(const iq16 _speed){
        speed = MIN(_speed / 1000, max_speed);
    }

    constexpr void set_x_by_mm(const iq16 _x){
        x = CLAMP2(_x / 1000, X_LIMIT);
    }

    constexpr void set_y_by_mm(const iq16 _y){
        y = CLAMP2(_y / 1000, Y_LIMIT);
    }
};

struct PolarRobotCurveGenerator{
    struct Config{
        uint32_t fs;
        iq16 speed;
        Vec2<iq24> initial_coord = {0,0};
    };

    explicit constexpr PolarRobotCurveGenerator(const Config & cfg):
        fs_(cfg.fs),
        delta_dist_(cfg.speed / cfg.fs),
        step_iter_(StepPointIterator{StepPointIterator::Config{
            .initial_coord = cfg.initial_coord
        }}){;}

    constexpr void add_end_coord(const Vec2<iq24> coord){
        step_iter_.set_target_coord(coord.flip_y());
    }

    constexpr void set_move_speed(const iq24 speed){
        delta_dist_ = (speed / fs_);
    }

    constexpr bool has_next(){
        return step_iter_.has_next();
    }

    constexpr Vec2<iq24> next(){
        coord_ = step_iter_.next(delta_dist_);
        return coord_;
    }

    constexpr Vec2<iq24> last_coord() const {
        return coord_;
    }
private:    
    uint32_t fs_;
    iq24 delta_dist_;
    Vec2<iq24> coord_ = Vec2<iq24>::ZERO;
    StepPointIterator step_iter_;
};


void polar_robot_main(){

    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.force_sync(EN);
    DEBUGGER.no_brackets(EN);

    auto & can = COMM_CAN;
    
    #ifndef MOCK_TEST
    #if PHY_SEL == PHY_SEL_UART

    auto & MOTOR1_UART = hal::usart1;
    auto & MOTOR2_UART = hal::usart1;

    MOTOR1_UART.init({921600});

    ZdtStepper motor1{{.nodeid = {1}}, &MOTOR1_UART};

    if(&MOTOR1_UART != &MOTOR2_UART){
        MOTOR2_UART.init({921600});
    }

    ZdtStepper motor2{{.nodeid = {2}}, &MOTOR2_UART};


    #else

    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    can.enable_hw_retransmit(DISEN);

    can.filters<0>().apply(
        hal::CanFilterConfig::accept_all()
    );

    ZdtStepper motor1{{.node_id = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.node_id = {2}}, &COMM_CAN};

    #endif

    ZdtJointMotorActuator radius_joint_ = {{
        .homming_mode = ZdtStepper::HommingMode::LapsCollision
    }, motor2};

    ZdtJointMotorActuator theta_joint_ = {{
        .homming_mode = ZdtStepper::HommingMode::LapsEndstop
    }, motor1};
    #else
    MockJointMotorActuator radius_joint_ = {};
    MockJointMotorActuator theta_joint_ = {};
    #endif

    PolarRobotActuator actuator_ = {
        {
            .rho_transform_scale = 25_r,
            .theta_transform_scale = iq16(9.53 / TAU),
            .center_bias = 0.0_deg,

            .rho_range = {0.0_r, 0.4_r},
            .theta_range = {-10_r, 10_r}
        },
        {
            .radius_joint_ = &radius_joint_, 
            .theta_joint_ = &theta_joint_
        }
    };
    Cartesian2ContinuousPolarRegulator regu_;

    static constexpr uint32_t POINT_GEN_FREQ = 500;
    static constexpr auto POINT_GEN_DURATION_MS = 1000ms / POINT_GEN_FREQ;
    static constexpr auto MAX_MOVE_SPEED = 0.002_iq24; // 2cm / s

    static constexpr auto GEN_CONFIG = PolarRobotCurveGenerator::Config{
        .fs = POINT_GEN_FREQ,
        .speed = MAX_MOVE_SPEED
    };

    PolarRobotCurveGenerator curve_gen_{GEN_CONFIG};

    [[maybe_unused]] auto fetch_next_gcode_line = [] -> Option<StringView>{
        static StringSplitIter line_iter{GCODE_LINES_NANJING, '\n'};
        while(line_iter.has_next()){
            const auto next_line = line_iter.next().unwrap();
            if(next_line.trim().length() == 0)
                continue;
            return Some(next_line); 
        }
        return None;
    };

    [[maybe_unused]] auto dispatch_gcode_line = [&](const StringView str){
        auto line = gcode::GcodeLine(str);

        ASSERT(line.query_mnemonic().examine().to_letter() == 'G', 
            "only G gcode is supported");

        static GcodeStateHolder state_;

        auto parse_g_command = [&](const gcode::GcodeArg & arg){
            const uint16_t major = static_cast<uint16_t>(arg.value);
            switch(major){
            case 0://rapid move
                curve_gen_.set_move_speed(state_.max_speed);
                curve_gen_.add_end_coord({state_.x.count(), state_.y.count()});
                break;
            case 1://linear move
                curve_gen_.set_move_speed(state_.speed);
                curve_gen_.add_end_coord({state_.x.count(), state_.y.count()});
                break;
            case 4:
                break;
            case 21://UseMillimetersUnits
                break;
            case 90://Use abs coord
                break;
            default:
                PANIC("not impleted gcode", major);
                break;
            }
        };

        auto parse_arg = [&](const gcode::GcodeArg & arg){
            switch(arg.letter){
            case 'X': 
                state_.x = unit::MilliMeter<iq16>(arg.value);
                break;
            case 'Y':
                state_.y = unit::MilliMeter<iq16>(arg.value);
                break;
            case 'F':
                state_.set_speed(arg.value);
                break;
            case 'G':
                parse_g_command(arg);
                break;
            }
        };


        {
            auto iter = gcode::GcodeArgsIter(str);
            while(iter.has_next()){
                const auto arg = iter.next().examine();
                // DEBUG_PRINTLN(arg);
                parse_arg(arg);
            }
        }
    };

    [[maybe_unused]] auto poll_gcode = [&]{
        const auto may_line_str = fetch_next_gcode_line();
        if(may_line_str.is_none()) return;
        const auto line = may_line_str.unwrap().trim();

        dispatch_gcode_line(line);
    };

    auto list = repl::make_list(
        "polar_robot",
        repl::make_function("reset", [&]{
            sys::reset();
        }),

        actuator_.make_repl_list("actuator"),
        radius_joint_.make_repl_list("radius_joint"),
        theta_joint_.make_repl_list("theta_joint"),

        repl::make_function("pxy", [&](const iq16 x, const iq16 y){
            curve_gen_.add_end_coord({
                CLAMP2(x, 0.14_r),
                CLAMP2(y, 0.14_r)
            });
        }),

        repl::make_function("next", [&](){
            static Vec2<iq16> coord = {0.1_r, 0};
            coord = coord.forward_90deg();
            actuator_.set_coord(regu_(coord));
        })
    );

    [[maybe_unused]] auto poll_curve_service = [&]{
        if(not curve_gen_.has_next()){
            poll_gcode();
        }
    };


    [[maybe_unused]] auto repl_service = [&](){ 
        
        static robots::ReplServer repl_server = {
            &DBG_UART, &DBG_UART
        };
        repl_server.invoke(list);
    };

    [[maybe_unused]] auto can_watch_service = [&]{
        while(COMM_CAN.available()){
            DEBUG_PRINTLN(COMM_CAN.read());
        }

        // static size_t cnt = 0;
        // cnt++;
        // if(cnt > 10) PANIC();
    };

    [[maybe_unused]] auto report_service = [&]{

    };

    actuator_.trig_homing();

    while(true){
        poll_curve_service();

        static auto timer = async::RepeatTimer
            ::from_duration(POINT_GEN_DURATION_MS);

        timer.invoke_if([&]{
            const auto coord = curve_gen_.next();

            actuator_.set_coord(regu_(coord));
        });

        repl_service();
        // report_service();

        DEBUG_PRINTLN_IDLE(
            curve_gen_.last_coord(), 
            radius_joint_.last_angle().to_turns(), 
            theta_joint_.last_angle().to_turns()
            // COMM_CAN.available(),
            // COMM_CAN.get_last_fault(),
            // COMM_CAN.get_rx_errcnt(),
            // COMM_CAN.get_rx_errcnt()
        );

        // can_watch_service();

        // const auto clock_time = clock::seconds();
        // const auto [s0,c0] = sincos(clock_time);
        // const auto [s,c] = std::make_tuple(s0, s0);
        // const auto vec = Vec2{s,c};
        // DEBUG_PRINTLN(s,c, atan2(s,c), vec_angle_diff<iq16>(vec, vec.rotated(1.6_r*s0)));
        // clock::delay(1ms);
    }
}
#endif