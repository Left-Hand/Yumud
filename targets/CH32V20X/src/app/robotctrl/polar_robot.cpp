#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/sync/timer.hpp"
#include "core/string/string_view.hpp"


#include "robots/vendor/zdt/zdt_stepper.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/cannet/slcan/slcan.hpp"
#include "robots/cannet/can_chain.hpp"


#include "atomic"
#include "types/vectors/vector2.hpp"
#include "details/polar_robot_curvedata.hpp"

#include "types/colors/color/color.hpp"
#include "types/vectors/polar.hpp"
#include "core/string/utils/multiline_split.hpp"

#include "common_service.hpp"
#include "joints.hpp"
#include "gcode.hpp"
#include "gcode_data.hpp"

#ifdef ENABLE_UART1
using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;
using robots::zdtmotor::ZdtStepper;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
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


#define DEF_QUICK_COMMAND_BIND(NAME) DEF_COMMAND_BIND(CommandKind::NAME, commands::NAME)


class PolarRobotActuator{
public:
    struct Config{
        real_t rho_transform_scale;
        real_t theta_transform_scale;
        real_t center_bias;

        Range2<real_t> rho_range;
        Range2<real_t> theta_range;
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


    void set_coord(const Polar<q16> p){

        const auto rho_position = p.radius * cfg_.rho_transform_scale;
        const auto theta_position = p.theta * cfg_.theta_transform_scale;

        joint_rho_.set_position(rho_position - cfg_.center_bias);
        joint_theta_.set_position(-theta_position);
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

    auto make_rpc_list(const StringView name){
        return rpc::make_list(
            name,
            DEF_RPC_MEMFUNC(trig_homing),
            DEF_RPC_MEMFUNC(is_homing_done),
            DEF_RPC_MEMFUNC(deactivate),
            DEF_RPC_MEMFUNC(activate)
            // DEF_RPC_MEMFUNC(set_coord)
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
        Vector2<q16> position;
        q16 theta;  // 累积角度
    };

    Polar<q16> operator()(const Vector2<q16> position) {
        if (may_last_state_.is_none()) {
            // 第一次调用，初始化状态
            may_last_state_ = Some(State{
                .position = position,
                .theta = position.angle()  // 初始角度
            });
            return Polar<q16>{position.length(), position.angle()};
        }

        // 获取上次状态
        const auto last_state = may_last_state_.unwrap();
        
        // 计算角度变化
        const q16 delta_theta = last_state.position.angle_between(position);
        const q16 new_theta = last_state.theta + delta_theta;

        // DEBUG_PRINTLN(last_state.position, position, delta_theta);

        // 更新状态
        may_last_state_ = Some(State{
            .position = position,
            .theta = new_theta  // 存储累积角度
        });

        return Polar<q16>{position.length(), new_theta};
    }

private:
    Option<State> may_last_state_ = None;
};


template<typename T>
constexpr Vector2<T> vec_step_to(const Vector2<T> from, const Vector2<T> to, T step){
    Vector2<T> delta = to - from;
    T distance = delta.length();
    if (distance <= step) return to;  // 如果步长足够大，直接到达目标
    return from + delta * (step / distance);  // 按比例移动
}

// template<typename T>
// constexpr Vector2<T> vec_step_to(const Vector2<T> from, const Vector2<T> to, T step) {
//     const Vector2<T> delta = to - from;
//     const T distance_sq = delta.length_squared();  // 避免开平方
//     const T step_sq = step * step;
    
//     if (distance_sq <= step_sq || distance_sq == T(0)) {
//         return to;
//     }
    
//     // 计算 sqrt(distance_sq) 并执行比例步长
//     const T distance = sqrt(distance_sq);  // 或使用定点数优化的快速开方
//     return from + delta * (step / distance);
// }



struct StepPointIterator{
    struct Config{
        Vector2<q24> initial_position;
    };

    explicit constexpr StepPointIterator(
        const Config & cfg
    ):
        current_position_(cfg.initial_position){;}

    constexpr void set_target_position(
        const Vector2<q24> target_position
    ) {
        may_end_position_ = Some(target_position);
    }

    [[nodiscard]] constexpr Vector2<q24> next(const q24 step){
        if(may_end_position_.is_none()) return current_position_;
        current_position_ = vec_step_to(current_position_, may_end_position_.unwrap(), step);
        return current_position_;
    }

    [[nodiscard]] constexpr bool has_next() const {
        return may_end_position_.is_some() and 
            (not current_position_.is_equal_approx(may_end_position_.unwrap()));
    }
private:
    Vector2<q24> current_position_ = {};
    Option<Vector2<q24>> may_end_position_ = None;
};


struct History{
    static constexpr auto X_LIMIT = 0.2_r;
    static constexpr auto Y_LIMIT = 0.2_r;
    real_t max_speed = 0.02_r;
    real_t speed;
    real_t x;
    real_t y;
    bool is_rapid;

    constexpr void set_speed(const real_t _speed){
        speed = MIN(_speed / 1000, max_speed);
    }

    constexpr void set_x_by_mm(const real_t _x){
        x = CLAMP2(_x / 1000, X_LIMIT);
    }

    constexpr void set_y_by_mm(const real_t _y){
        y = CLAMP2(_y / 1000, Y_LIMIT);
    }
};

struct PolarRobotCurveGenerator{
    struct Config{
        uint32_t fs;
        real_t speed;
        Vector2<q24> initial_position = {0,0};
    };

    explicit constexpr PolarRobotCurveGenerator(const Config & cfg):
        fs_(cfg.fs),
        delta_dist_(cfg.speed / cfg.fs),
        step_iter_(StepPointIterator{StepPointIterator::Config{
            .initial_position = cfg.initial_position
        }}){;}

    constexpr void add_end_position(const Vector2<q24> position){
        step_iter_.set_target_position(position.flip_y());
    }

    constexpr void set_move_speed(const q24 speed){
        delta_dist_ = (speed / fs_);
    }

    constexpr bool has_next(){
        return step_iter_.has_next();
    }

    constexpr Vector2<q24> next(){
        position_ = step_iter_.next(delta_dist_);
        return position_;
    }

    constexpr Vector2<q24> last_position() const {
        return position_;
    }
private:    
    uint32_t fs_;
    q24 delta_dist_;
    Vector2<q24> position_;
    StepPointIterator step_iter_;
};


void polar_robot_main(){

    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.force_sync(EN);
    DEBUGGER.no_brackets();
    
    #ifndef MOCK_TEST
    #if PHY_SEL == PHY_SEL_UART

    auto & MOTOR1_UART = hal::uart1;
    auto & MOTOR2_UART = hal::uart1;

    MOTOR1_UART.init({921600});

    ZdtStepper motor1{{.nodeid = {1}}, &MOTOR1_UART};

    if(&MOTOR1_UART != &MOTOR2_UART){
        MOTOR2_UART.init({921600});
    }

    ZdtStepper motor2{{.nodeid = {2}}, &MOTOR2_UART};


    #else

    COMM_CAN.init({
        .baudrate = CanBaudrate::_1M, 
        .mode = CanMode::Normal
    });

    COMM_CAN.enable_hw_retransmit(DISEN);

    COMM_CAN[0].mask({
            .id = CanStdIdMask{0x200, CanRemoteSpec::Any}, 
            .mask = CanStdIdMask::from_ignore_low(7, CanRemoteSpec::Any)
        },{
            .id = CanStdIdMask{0x000, CanRemoteSpec::Any}, 
            .mask = CanStdIdMask::from_accept_all()
        }
    );

    ZdtStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.nodeid = {2}}, &COMM_CAN};

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
            .theta_transform_scale = real_t(9.53 / TAU),
            .center_bias = 0.0_r,

            .rho_range = {0.0_r, 0.4_r},
            .theta_range = {-10_r, 10_r}
        },
        {
            .radius_joint_ = &radius_joint_, 
            .theta_joint_ = &theta_joint_
        }
    };
    Cartesian2ContinuousPolarRegulator regu_;

    static constexpr uint32_t POINT_GEN_FREQ = 400;
    static constexpr auto POINT_GEN_DURATION_MS = 1000ms / POINT_GEN_FREQ;
    static constexpr auto MAX_MOVE_SPEED = 0.02_q24; // 5cm / s

    static constexpr auto GEN_CONFIG = PolarRobotCurveGenerator::Config{
        .fs = POINT_GEN_FREQ,
        .speed = MAX_MOVE_SPEED
    };

    PolarRobotCurveGenerator curve_gen_{GEN_CONFIG};

    [[maybe_unused]] auto get_next_gcode_line = [] -> Option<StringView>{
        static strconv2::StringSplitIter line_iter{GCODE_LINES_NANJING, '\n'};
        while(line_iter.has_next()){
            const auto next_line = line_iter.next();
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

        static History history_;

        auto parse_g_command = [&](const gcode::GcodeArg & arg){
            const uint16_t major = int(arg.value);
            switch(major){
            case 0://rapid move
                curve_gen_.set_move_speed(history_.max_speed);
                curve_gen_.add_end_position({history_.x, history_.y});
                break;
            case 1://linear move
                curve_gen_.set_move_speed(history_.speed);
                curve_gen_.add_end_position({history_.x, history_.y});
                break;
            case 4:
                break;
            case 21://UseMillimetersUnits
                break;
            case 90://Use abs position
                break;
            default:
                PANIC("not impleted gcode", major);
                break;
            }
        };

        auto parse_arg = [&](const gcode::GcodeArg & arg){
            switch(arg.letter){
            case 'X': 
                history_.set_x_by_mm(arg.value);
                break;
            case 'Y':
                history_.set_y_by_mm(arg.value);
                break;
            case 'F':
                history_.set_speed(arg.value);
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
        const auto may_line_str = get_next_gcode_line();
        if(may_line_str.is_none()) return;
        const auto line = may_line_str.unwrap().trim();

        dispatch_gcode_line(line);
    };

    auto list = rpc::make_list(
        "polar_robot",
        rpc::make_function("reset", [&]{
            sys::reset();
        }),

        actuator_.make_rpc_list("actuator"),
        radius_joint_.make_rpc_list("radius_joint_"),
        theta_joint_.make_rpc_list("theta_joint_"),

        rpc::make_function("pxy", [&](const real_t x, const real_t y){
            curve_gen_.add_end_position({
                CLAMP2(x, 0.14_r),
                CLAMP2(y, 0.14_r)
            });
        }),

        // rpc::make_function("prt", [&](const real_t radius, const real_t theta){
        //     actuator_.set_coord({radius,theta});
        // }),

        rpc::make_function("next", [&](){
            static Vector2<q16> position = {0.1_r, 0};
            position = position.cw();
            actuator_.set_coord(regu_(position));
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


            const auto position = curve_gen_.next();

            actuator_.set_coord(regu_(position));
        });

        repl_service();
        // report_service();

        DEBUG_PRINTLN_IDLE(
            curve_gen_.last_position(), 
            radius_joint_.get_last_position(), 
            theta_joint_.get_last_position()
            // COMM_CAN.available(),
            // COMM_CAN.get_last_fault(),
            // COMM_CAN.get_rx_errcnt(),
            // COMM_CAN.get_rx_errcnt()
        );

        // can_watch_service();

        // const auto clock_time = clock::time();
        // const auto [s0,c0] = sincos(clock_time);
        // const auto [s,c] = std::make_tuple(s0, s0);
        // const auto vec = Vector2{s,c};
        // DEBUG_PRINTLN(s,c, atan2(s,c), vec_angle_diff<real_t>(vec, vec.rotated(1.6_r*s0)));
        // clock::delay(1ms);
    }
}
#endif