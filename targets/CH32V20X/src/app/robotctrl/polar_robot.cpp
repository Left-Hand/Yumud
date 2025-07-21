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
#include "types/vectors/vector2/Vector2.hpp"
#include "details/polar_robot_curvedata.hpp"

#include "types/colors/color/color.hpp"
#include "types/vectors/polar/polar.hpp"

#include "common_service.hpp"
#include "joints.hpp"

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
        Some<JointMotorActuatorIntf *> radius_joint;
        Some<JointMotorActuatorIntf *> theta_joint;
    };


    PolarRobotActuator(
        const Config & cfg, 
        const Params & params
    ):
        cfg_(cfg),
        joint_rho_(params.radius_joint.deref()),
        joint_theta_(params.theta_joint.deref())
    {;}


    // void set_coord(const real_t p.radius, const real_t p.theta){
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

struct PolarRobotKinePlanner{

    explicit PolarRobotKinePlanner(PolarRobotActuator & act):
        act_(act){;}

    [[nodiscard]] constexpr auto set_position(const Vector2<q16> pos){
        // if(x_meters == 0 && y_meters == 0) return;
        // auto apply = [&](const Solver::Solution & sol, const Solver::Gesture & gest){

        //     set_coord(
        //         sol.p.radius, 
        //         sol.p.theta
        //     );


        //     may_last_gest_ = Some(gest);
        //     may_last_theta_ = Some(sol.p.theta);
        // };

        // const auto gest = Solver::Gesture{
        //     .x_meters = pos.x,
        //     .y_meters = pos.y
        // };

        // const auto gest_vec2 = gest.to_vec2();
        // if(may_last_gest_.is_none()){
        //     const auto sol = Solver::Solution{
        //         .p.radius = gest_vec2.length(),
        //         .p.theta = gest_vec2.angle()
        //     };
        //     apply(sol, gest);
        //     return;
        // }

        // auto last_theta = ({
        //     if(may_last_theta_.is_none()) PANIC();
        //     may_last_theta_.unwrap();
        // });

        // if(last_theta > 25){
        //     last_theta = 0;
        // }

        // const auto last_gest_vec2 = may_last_gest_.unwrap().to_vec2();
        // const auto delta_theta = vec_angle_diff(gest_vec2, last_gest_vec2);
        // const auto radius = gest_vec2.length();
        // const auto sol = Solver::Solution{
        //     .p.radius = radius,
        //     .p.theta = last_theta + delta_theta
        // };

        
        // DEBUG_PRINTLN(
        //     sol.p.radius,
        //     sol.p.theta,

        //     x_meters,
        //     y_meters,
        //     last_theta,
        //     delta_theta
        // );

        // apply(sol, gest);
        // may_last_gest_ = Some(gest);
        // may_last_theta_ = Some(sol.p.theta);

        return [=, this]{
            const auto p = regu_(pos);
            act_.set_coord(p);
        };
    }
private:
    PolarRobotActuator & act_;
    Cartesian2ContinuousPolarRegulator regu_;
};


struct QueuePointIterator{

    explicit constexpr QueuePointIterator(
        const std::span<const Vector2<bf16>> data
    ):
        data_(data){;}

    [[nodiscard]] Vector2<q24> next(const q24 step){

        const auto curr_i = i_;

        
        const auto p1 = Vector2{
            q24::from(float(data_[curr_i].x)), 
            q24::from(float(data_[curr_i].y))
        };

        p = p.move_toward(p1, step);
        // p.x = STEP_TO(p.x, p1.x, 0.0002_r);
        // p.y = STEP_TO(p.y, p1.y, 0.0002_r);

        if(p.is_equal_approx(p1)){
            i_ = (i_ + 1) % data_.size();
        }
        // DEBUG_PRINTLN(p);
        return p;
        // return Vector2<q24>(data_[0].x, data_.size());
        // return Vector2<q24>(i_, data_.size());
    }

    [[nodiscard]] constexpr size_t index() const {
        return i_;
    }
private:
    std::span<const Vector2<bf16>> data_;
    Vector2<q24> p = {};
    size_t i_ = 0;
};


void polar_robot_main(){

    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.force_sync(EN);
    DEBUGGER.no_brackets();
    
    // while(true){
    //     DEBUG_PRINTLN(clock::millis());
    //     clock::delay(5ms);
    // }

    
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
    ZdtStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.nodeid = {2}}, &COMM_CAN};

    #endif

    ZdtJointMotorActuator radius_joint = {{
        .homming_mode = ZdtStepper::HommingMode::LapsCollision
    }, motor2};

    ZdtJointMotorActuator theta_joint = {{
        .homming_mode = ZdtStepper::HommingMode::LapsEndstop
    }, motor1};
    #else
    MockJointMotorActuator radius_joint = {};
    MockJointMotorActuator theta_joint = {};
    #endif

    PolarRobotActuator robot_actuator = {
        {
            .rho_transform_scale = 25_r,
            .theta_transform_scale = real_t(9.53 / TAU),
            .center_bias = 0.0_r,

            .rho_range = {0.0_r, 0.4_r},
            .theta_range = {-10_r, 10_r}
        },
        {
            .radius_joint = &radius_joint, 
            .theta_joint = &theta_joint
        }
    };

    PolarRobotKinePlanner robot_planner = PolarRobotKinePlanner{
        robot_actuator
    };

    auto list = rpc::make_list(
        "polar_robot",
        robot_actuator.make_rpc_list("actuator"),
        radius_joint.make_rpc_list("radius_joint"),
        theta_joint.make_rpc_list("theta_joint"),

        rpc::make_function("pxy", [&](const real_t x, const real_t y){
            robot_planner.set_position({x,y})();

        }),
        rpc::make_function("prt", [&](const real_t radius, const real_t theta){
            robot_actuator.set_coord({radius,theta});
        }),
        rpc::make_function("next", [&](){
            static Vector2<q16> position = {0.1_r, 0};
            position = position.cw();
            robot_planner.set_position(position)();
        })
    );

    [[maybe_unused]] auto draw_curve_service = [&]{
        static constexpr uint32_t CALL_FREQ = 500;
        static constexpr auto CALL_DURATION_MS = 1000ms / CALL_FREQ;
        static constexpr auto MAX_MOVE_SPEED = 0.05_q24; // 5cm / s
        static constexpr auto DELTA_PER_CALL = MAX_MOVE_SPEED / CALL_FREQ;

        static auto timer = async::RepeatTimer::from_duration(CALL_DURATION_MS);
        static auto it = QueuePointIterator{std::span(CURVE_DATA)};

        timer.invoke_if([&]{

            const auto p = it.next(DELTA_PER_CALL);

            robot_planner.set_position(p)();

            DEBUG_PRINTLN(p, radius_joint.get_last_position(), theta_joint.get_last_position());

        });
    };

    robot_actuator.trig_homing();
    [[maybe_unused]] auto repl_service = [&](){ 
        
        static robots::ReplServer repl_server = {
            &DBG_UART, &DBG_UART
        };
        repl_server.invoke(list);
    };

    while(true){
        draw_curve_service();
        repl_service();
        // const auto clock_time = clock::time();
        // const auto [s0,c0] = sincos(clock_time);
        // const auto [s,c] = std::make_tuple(s0, s0);
        // const auto vec = Vector2{s,c};
        // DEBUG_PRINTLN(s,c, atan2(s,c), vec_angle_diff<real_t>(vec, vec.rotated(1.6_r*s0)));
        // clock::delay(1ms);
    }
}
#endif