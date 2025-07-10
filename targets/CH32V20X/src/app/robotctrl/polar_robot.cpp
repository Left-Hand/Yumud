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


#ifdef ENABLE_UART1
using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;

#define DBG_UART hal::uart1
#define COMM_UART hal::uart2
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

//CAN1 TX/PB9 RX/PB8

class LedService{
public:
    enum class BreathMethod:uint8_t{
        Sine = 0,
        Saw,
        Square,
        Triangle
    };
protected:
    struct Blink{
        Milliseconds on;
        Milliseconds off;
        size_t times;
    };

    struct Pulse{
        Milliseconds on;
    };

public:

    void resume();

private:
    // void set_color(const Color<real_t> & color){

    // }
};

class BeepService{
public:
    struct Config{

    };

    explicit BeepService(
        Some<hal::Gpio *> gpio
    ): gpio_(gpio.deref()){;}

    void push_pulse(const Milliseconds period){

    };

    void resume(){

    }

private:
    hal::Gpio & gpio_;
};


struct PolarRobotSolver{
    struct Gesture{
        real_t x_meters;
        real_t y_meters;

        constexpr Vector2<real_t> to_vec2() const {
            return Vector2(x_meters, y_meters);
        }
    };

    struct Solution{
        real_t rho_meters;
        real_t theta_radians;
    };

    static constexpr Gesture inverse(const Solution & sol){
        const auto [s,c] = sincos(sol.theta_radians);
        return {
            sol.rho_meters * c,
            sol.rho_meters * s
        };
    }

    static constexpr Solution nearest_forward(
        const Gesture & last_g, 
        const Gesture & g
    ){

        const auto g_vec2 = g.to_vec2();
        const auto last_g_vec2 = last_g.to_vec2();

        const auto delta_theta = g_vec2.angle_to(last_g_vec2);

        return {
            .rho_meters = g_vec2.length(),
            .theta_radians = last_g_vec2.angle() + delta_theta
        };
    }

private:

    static constexpr real_t nearest_fmod(
        const real_t x,
        const real_t step
    ){
        const auto half_step = step / 2;
        return fposmodp(x + half_step, step) - half_step;
    }
    static constexpr real_t wrapped_diff(
        const real_t last_x,
        const real_t x,
        const real_t step
    ){
        const auto diff = x - last_x;
        return nearest_fmod(diff, step);
    }
};

class JointMotorIntf{
public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void set_position(real_t position) = 0;
    virtual void trig_homing() = 0;
    virtual void trig_cali() = 0;
    virtual bool is_homing_done() = 0;

    auto make_rpc_list(const StringView name){
        return rpc::make_list(
            name,
            DEF_RPC_MAKE_MEMFUNC(trig_homing),
            DEF_RPC_MAKE_MEMFUNC(is_homing_done),
            DEF_RPC_MAKE_MEMFUNC(deactivate),
            DEF_RPC_MAKE_MEMFUNC(activate),
            DEF_RPC_MAKE_MEMFUNC(set_position),
            DEF_RPC_MAKE_MEMFUNC(trig_cali)
        );
    }
};

class JointMotorAdapter_Mock final:
    public JointMotorIntf{
public:
    void activate() {}
    void deactivate() {}
    void set_position(real_t position) {}
    void trig_homing() {}
    void trig_cali() {}
    bool is_homing_done() {return true;}
};

class JointMotorAdapter_ZdtStepper final
    :public JointMotorIntf{
public:

    struct Config{
        ZdtStepper::HommingMode homming_mode;
    };

    JointMotorAdapter_ZdtStepper(
        const Config & cfg, 
        ZdtStepper & stepper
    ):
        cfg_(cfg),
        stepper_(stepper){;}

    void activate(){
        stepper_.activate(EN).unwrap();
    }

    void deactivate(){
        stepper_.activate(DISEN).unwrap();;
    }

    void set_position(real_t position){
        // if(is_homed_ == false) 
        //     trip_and_panic("motor not homed");
        // DEBUG_PRINTLN("set_pos");
        stepper_.set_target_position(position).unwrap();;
    }

    void trig_homing(){
        homing_begin_ = Some(clock::millis());
        stepper_.trig_homming(cfg_.homming_mode).unwrap();;
    }

    void trig_cali(){ 
        stepper_.trig_cali().unwrap();;
    }

    bool is_homing_done(){
        if(homing_begin_.is_none()){
            trip_and_panic("Homing not started");
            return false;
        }

        return (clock::millis() - homing_begin_.unwrap()) > homing_timeout_;
    }


private:
    Config cfg_;
    ZdtStepper & stepper_;

    static constexpr Milliseconds homing_timeout_ = 5000ms;
    Option<Milliseconds> homing_begin_ = None;
    std::atomic<bool> is_homed_ = false;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        DEBUG_PRINTLN(std::forward<Args>(args)...);
    }

    struct HomingStrategy_Timeout{

    };
};

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
        Some<JointMotorIntf *> joint_rho;
        Some<JointMotorIntf *> joint_theta;
    };

    using Solver = PolarRobotSolver;


    PolarRobotActuator(
        const Config & cfg, 
        const Params & params
    ):
        cfg_(cfg),
        joint_rho_(params.joint_rho.deref()),
        joint_theta_(params.joint_theta.deref())
    {;}

    void set_polar(const real_t rho_meters, const real_t theta_radians){

        const auto rho_position = rho_meters * cfg_.rho_transform_scale;
        const auto theta_position = theta_radians * cfg_.theta_transform_scale;

        // DEBUG_PRINTLN(
        //     // x_meters, 
        //     // y_meters, 
        //     rho_position,
        //     theta_position
        // );


        joint_rho_.set_position(rho_position - cfg_.center_bias);
        joint_theta_.set_position(theta_position);
    }

    void set_position(const real_t x_meters, const real_t y_meters){
        const auto gest = Solver::Gesture{
            .x_meters = x_meters,
            .y_meters = y_meters
        };

        const auto sol = Solver::nearest_forward(
            last_gest_, gest);

        last_gest_ = gest;

        set_polar(
            sol.rho_meters, 
            sol.theta_radians);
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
            DEF_RPC_MAKE_MEMFUNC(trig_homing),
            DEF_RPC_MAKE_MEMFUNC(is_homing_done),
            DEF_RPC_MAKE_MEMFUNC(deactivate),
            DEF_RPC_MAKE_MEMFUNC(activate),
            DEF_RPC_MAKE_MEMFUNC(set_position),
            DEF_RPC_MAKE_MEMFUNC(set_polar)
        );
    }

private:
    const Config cfg_;
    JointMotorIntf & joint_rho_;
    JointMotorIntf & joint_theta_;

    Solver::Gesture last_gest_;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        DEBUG_PRINTLN(std::forward<Args>(args)...);
    }

};

class CanHandlerAdaptor_PolarRobotActuator final:
public CanMsgHandlerIntf{
    HandleStatus handle(const hal::CanMsg & msg){ 
        if(not msg.is_standard()) 
            return HandleStatus::from_unhandled();
        switch(msg.id_as_u32()){
            case 0:
                DEBUG_PRINTLN("o"); break;
            default:
                return HandleStatus::from_unhandled();
        }
        return HandleStatus::from_handled();
    }
};

template<typename T>
class RobotMsgCtrp{
    constexpr hal::CanMsg serialize_to_canmsg() const {
        const auto bytes_iter = static_cast<const T *>(this)->iter_bytes();
        return hal::CanMsg::from_bytes(bytes_iter);
    }
};

class RobotMsgMoveXy final:public RobotMsgCtrp<RobotMsgMoveXy>{
    real_t x;
    real_t y;

    constexpr std::array<uint8_t, 8> iter_bytes() const {
        uint64_t raw;
        raw |= std::bit_cast<uint32_t>(y.to_i32());
        raw <<= 32;
        raw |= std::bit_cast<uint32_t>(x.to_i32());
        return std::bit_cast<std::array<uint8_t, 8>>(raw);
    } 
};



// #define MOCK_TEST
struct QueuePointIterator{

    explicit constexpr QueuePointIterator(
        const std::span<const Vector2<q16>> data
    ):
        data_(data){;}

    [[nodiscard]] constexpr Vector2<q16> next(){
        // const auto curr_i = i;
        // const auto p_dest = data_[curr_i];
        // p = (p * 128).move_toward(p_dest * 128, 0.02_r) / 128;

        // if(p.is_equal_approx(p_dest)){
        //     const auto next_i = (i + 1) % data_.size();
        //     i = next_i;
        // }

        // return p_dest;

        const auto curr_i = (i) % data_.size();

        
        const auto p1 = data_[curr_i];
        p = (p * 128).move_toward(p1 * 128, 0.02_r) / 128;
        // p.x = STEP_TO(p.x, p1.x, 0.0002_r);
        // p.y = STEP_TO(p.y, p1.y, 0.0002_r);

        // if(ABS(p.x - p1.x) < 0.00001_r) i++;
        if(p.is_equal_approx(p1)) i++;
        return p;
        // return Vector2<q16>(data_[0].x, data_.size());
        // return Vector2<q16>(i, data_.size());
    }

    [[nodiscard]] constexpr size_t index() const {
        return i;
    }
private:
    std::span<const Vector2<q16>> data_;
    Vector2<q16> p = {};
    size_t i = 0;
};

void polar_robot_main(){

    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
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
    ZdtStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.nodeid = {2}}, &COMM_CAN};

    #endif

    JointMotorAdapter_ZdtStepper joint_rho = {{
        .homming_mode = ZdtStepper::HommingMode::LapsCollision
    }, motor2};

    JointMotorAdapter_ZdtStepper joint_theta = {{
        .homming_mode = ZdtStepper::HommingMode::LapsEndstop
    }, motor1};
    #else
    JointMotorAdapter_Mock joint_rho = {};
    JointMotorAdapter_Mock joint_theta = {};
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
            .joint_rho = &joint_rho, 
            .joint_theta = &joint_theta
        }
    };

    auto list = rpc::make_list(
        "polar_robot",
        robot_actuator.make_rpc_list("actuator"),
        joint_rho.make_rpc_list("joint_rho"),
        joint_theta.make_rpc_list("joint_theta")
    );

    robots::ReplServer repl_server = {
        &DBG_UART, &DBG_UART
    };

    constexpr auto SAMPLE_DUR = 700ms; 

    auto it = QueuePointIterator{std::span(curve_data)};

    while(true){
        async::RepeatTimer timer{5ms};
        timer.invoke_if([&]{
            const auto p = it.next();

            robot_actuator.set_position(p.x, p.y);
            DEBUG_PRINTLN(p.x, p.y);

            repl_server.invoke(list);
        });
    }


    while(true){
        DEBUG_PRINTLN(clock::millis());
    }
}
#endif