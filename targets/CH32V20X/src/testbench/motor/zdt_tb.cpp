#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"

#include "core/string/StringView.hpp"
#include "atomic"
#include "types/vectors/vector2/Vector2.hpp"


#include "robots/cannet/slcan/slcan.hpp"
#include "data.hpp"

#include "core/sync/timer.hpp"


#ifdef ENABLE_UART1
using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN


struct PolarRobotSolver{
    struct Gesture{
        real_t x_meters;
        real_t y_meters;
    };

    struct Solution{
        real_t rho_meters;
        real_t phi_radians;
    };

    static constexpr Solution forward(const Gesture & g){
        return {
            .rho_meters = sqrt(square(g.x_meters) + square(g.y_meters)),
            .phi_radians = atan2(g.y_meters, g.x_meters)
        };
    }

    static constexpr Gesture inverse(const Solution & s){
        return {
            s.rho_meters * cos(s.phi_radians),
            s.rho_meters * sin(s.phi_radians)
        };
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
            DEF_MAKE_MEMFUNC(trig_homing),
            DEF_MAKE_MEMFUNC(is_homing_done),
            DEF_MAKE_MEMFUNC(deactivate),
            DEF_MAKE_MEMFUNC(activate),
            DEF_MAKE_MEMFUNC(set_position),
            DEF_MAKE_MEMFUNC(trig_cali)
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
        real_t phi_transform_scale;
        real_t center_bias;

        Range2<real_t> rho_range;
        Range2<real_t> phi_range;
    };

    using Solver = PolarRobotSolver;


    PolarRobotActuator(
        const Config & cfg, 
        JointMotorIntf & joint_rho,
        JointMotorIntf & joint_phi
    ):
        cfg_(cfg),
        joint_rho_(joint_rho),
        joint_phi_(joint_phi)
    {;}

    void set_polar(const real_t rho_meters, const real_t phi_radians){

        const auto rho_position = rho_meters * cfg_.rho_transform_scale;
        const auto phi_position = phi_radians * cfg_.phi_transform_scale;

        // DEBUG_PRINTLN(
        //     // x_meters, 
        //     // y_meters, 
        //     rho_position,
        //     phi_position
        // );


        joint_rho_.set_position(rho_position - cfg_.center_bias);
        joint_phi_.set_position(phi_position);
    }

    void set_position(const real_t x_meters, const real_t y_meters){
        const auto s = Solver::forward({
            .x_meters = x_meters,
            .y_meters = y_meters
        });

        set_polar(s.rho_meters, s.phi_radians);
    }

    void activate(){
        joint_rho_.activate();
        joint_phi_.activate();
    }

    void deactivate(){
        joint_rho_.deactivate();
        joint_phi_.deactivate();
    }

    void trig_homing(){
        joint_rho_.trig_homing();   
        joint_phi_.trig_homing();
    }

    bool is_homing_done(){
        return joint_rho_.is_homing_done() 
            && joint_phi_.is_homing_done();
    }

    auto make_rpc_list(const StringView name){
        return rpc::make_list(
            name,
            DEF_MAKE_MEMFUNC(trig_homing),
            DEF_MAKE_MEMFUNC(is_homing_done),
            DEF_MAKE_MEMFUNC(deactivate),
            DEF_MAKE_MEMFUNC(activate),
            DEF_MAKE_MEMFUNC(set_position),
            DEF_MAKE_MEMFUNC(set_polar)
        );
    }

private:
    const Config cfg_;
    JointMotorIntf & joint_rho_;
    JointMotorIntf & joint_phi_;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        DEBUG_PRINTLN(std::forward<Args>(args)...);
    }

};

class CanMsgHandlerIntf{ 
public:
    struct HandleStatus{
        static constexpr HandleStatus from_handled() { return HandleStatus{true}; }
        static constexpr HandleStatus from_unhandled() { return HandleStatus{false}; }

        bool is_handled() const { return is_handled_; }
    private:
        constexpr HandleStatus(bool is_handled) : is_handled_(is_handled) {}

        bool is_handled_;
    };

    virtual HandleStatus handle(const hal::CanMsg & msg) = 0;
};

class CanMsgHandlerChainlink final: public CanMsgHandlerIntf{ 
public:
    CanMsgHandlerChainlink(
        Some<CanMsgHandlerIntf *> curr, 
        Option<CanMsgHandlerIntf &> next
    ):
        curr_handler_(*curr.get()),
        next_handler_(next){;}

    HandleStatus handle(const hal::CanMsg & msg){ 
        HandleStatus res = curr_handler_.handle(msg);
        if(next_handler_.is_none()) return res;
        return next_handler_.unwrap().handle(msg);
    }
private:
    CanMsgHandlerIntf & curr_handler_;
    Option<CanMsgHandlerIntf &> next_handler_ = None;
};


template<typename T>
class RobotMsgCtrp{
    constexpr hal::CanMsg serialize_to_canmsg() const {
        const auto bytes_iter = static_cast<const T *>(this)->iter_bytes();
        return hal::CanMsg::from_range(bytes_iter);
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

class CanHandlerAdaptor_PolarRobotActuator final:
public CanMsgHandlerIntf{
    HandleStatus handle(const hal::CanMsg & msg){ 
        if(not msg.is_std()) 
            return HandleStatus::from_unhandled();
        switch(msg.id()){
            case 0:
                DEBUG_PRINTLN("o"); break;
            default:
                return HandleStatus::from_unhandled();
        }
        return HandleStatus::from_handled();
    }
};

class CanHandlerTerminator final: 
public CanMsgHandlerIntf{ 
    HandleStatus handle(const hal::CanMsg & msg){ 
        // PANIC("uncaught msg", msg);
        return HandleStatus::from_handled();
    }
};

// #define MOCK_TEST


[[maybe_unused]] static __attribute__((__noreturn__))
void polar_robot_main(){
    auto & OUT_UART = hal::uart2;
    OUT_UART.init({576000});

    DEBUGGER.retarget(&OUT_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);


    
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

    JointMotorAdapter_ZdtStepper joint_phi = {{
        .homming_mode = ZdtStepper::HommingMode::LapsEndstop
    }, motor1};
    #else
    JointMotorAdapter_Mock joint_rho = {};
    JointMotorAdapter_Mock joint_phi = {};
    #endif

    PolarRobotActuator robot_actuator = {
        {
            .rho_transform_scale = 25_r,
            .phi_transform_scale = real_t(9.53 / TAU),
            .center_bias = 0.0_r,

            .rho_range = {0.0_r, 0.4_r},
            .phi_range = {-10_r, 10_r}
        },
        joint_rho, joint_phi
    };

    auto list = rpc::make_list(
        "polar_robot",
        robot_actuator.make_rpc_list("actuator"),
        joint_rho.make_rpc_list("joint_rho"),
        joint_phi.make_rpc_list("joint_phi")
    );

    robots::ReplService repl_service = {
        &OUT_UART, &OUT_UART
    };

    constexpr auto POINT_DUR = 700ms; 
    // auto rpt = RepeatTimer{POINT_DUR};

    #if 0
    while(true){
        static size_t i = 0;

        rpt.invoke([&]{i++;});

        const auto curr_i = i % data.size();
        const auto next_i = (i + 1) % data.size();


        const auto p1 = data[curr_i];
        const auto p2 = data[next_i];

        const auto r = real_t(rpt.since_last_invoke().count()) / POINT_DUR.count();
        const auto p = p1.lerp(p2, r);
        // const auto p1 = Vector2<real_t>(-0.02_r, -0.17_r);
        // const auto p2 = Vector2<real_t>(0.17_r, 0.02_r);

        // const auto p = p1.lerp(p2, 0.5_r + 0.5_r * sinpu(clock::time() * 0.1_r));
        // const auto p = Vector2<real_t>(0.1_r, 0.0_r) + Vector2<real_t>(0, 0.07_r).rotated(clock::time() * 0.2_r);

        // const auto p1 = Vector2<real_t>(-0.02_r, -0.17_r);
        // const auto p2 = Vector2<real_t>(0.17_r, 0.02_r);

        // const auto p = Vector2<real_t>::RIGHT.rotated(clock::time() * 0.4_r) * 
        //     ((0.5_r + 0.5_r * sinpu(clock::time() * 0.5_r)) * 0.1_r + 0.1_r);
        robot_actuator.set_position(p.x, p.y);
        DEBUG_PRINTLN(p.x, p.y, i, r, rpt.since_last_invoke().count());

        clock::delay(5ms);
        // if(COMM_UART.available()){
        //     std::vector<uint8_t> recv;
        //     while(COMM_UART.available()){
        //         char chr;
        //         COMM_UART.read1(chr);
        //         recv.push_back(chr);
        //     }

        //     DEBUG_PRINTLN(
        //         "ret", 
        //         std::hex, 
        //         std::noshowbase, 
        //         recv
        //     );
        // }

        // constexpr auto k =  2_r / sqrt(2_r) * 0.1_r;
        // const auto [s,c] = sincospu(clock::time());
        // const auto p = atan2(s * k,s * k);
        // DEBUG_PRINTLN(s * k,c * k,p);

        repl_service.invoke(list);
    }
    #else 

    Vector2<q16> p = data[0];

    size_t i = 0;
    while(true){
        
        const auto curr_i = (i) % data.size();

        
        const auto p1 = data[curr_i];
        p = (p * 128).move_toward(p1 * 128, 0.02_r) / 128;
        // p.x = STEP_TO(p.x, p1.x, 0.0002_r);
        // p.y = STEP_TO(p.y, p1.y, 0.0002_r);

        // if(ABS(p.x - p1.x) < 0.00001_r) i++;
        if(p.is_equal_approx(p1)) i++;

        robot_actuator.set_position(p.x, p.y);
        DEBUG_PRINTLN(p.x, p.y, p1.x, p1.y, i);

        clock::delay(5ms);

        repl_service.invoke(list);
    }
    #endif
}

[[maybe_unused]] static __attribute__((__noreturn__))
void slcan_test(){
    auto & OUT_UART = hal::uart1;
    OUT_UART.init({576000});

    DEBUGGER.retarget(&OUT_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    can1.init({hal::Can::BaudRate::_1M});
    robots::asciican::AsciiCanPhy phy{can1};
    robots::asciican::Slcan slcan{phy};
    auto list = rpc::make_list(
        "slcan",
        rpc::make_function("tx", [&](const StringView str) {
            const auto u_begin = clock::micros();
            const auto res = slcan.on_recv_string(str);
            const auto u_end = clock::micros();
            DEBUG_PRINTLN(res, u_end - u_begin);
            // DEBUG_PRINTLN(res.is_err());
        })
    );

    robots::ReplService repl_service = {
        &OUT_UART, &OUT_UART
    };


    while(true){
        // DEBUG_PRINTLN(clock::millis());
        repl_service.invoke(list);
    }
}

void zdt_main(){
    polar_robot_main();
    // slcan_test();
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({
        .baudrate = CanBaudrate::_1M, 
        .mode = CanMode::Normal
    });

    COMM_CAN.enable_hw_retransmit(DISEN);
    ZdtStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.nodeid = {2}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor1.activate().unwrap();
    motor2.activate().unwrap();
    clock::delay(10ms);
    // motor.set_subdivides(256);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsCollision);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsEndstop);
    // motor.query_homming_paraments();

    robots::ReplService repl_service = {
        &DBG_UART, &DBG_UART
    };


    // auto list = rpc::make_list(
    //     "list",

    // );

    while(true){
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                char chr;
                COMM_UART.read1(chr);
                recv.push_back(chr);
            }

            DEBUG_PRINTLN(
                "ret", 
                std::hex, 
                std::noshowbase, 
                recv
            );
        }
        #else
        if(COMM_CAN.available()){
            DEBUG_PRINTLN("rx", COMM_CAN.read());
        }

        // DEBUG_PRINTLN(COMM_CAN.pending());

        // clock::delay(200ms);
        // motor.activate();
        const auto d1 = sin(clock::time()*0.7_r);
        const auto d2 = tpzpu(clock::time()*0.2_r);
        motor1.set_target_position(d1).unwrap();
        clock::delay(5ms);
        motor2.set_target_position(d2).unwrap();
        clock::delay(5ms);
        DEBUG_PRINTLN(d1, d2);
        // DEBUG_PRINTLN(clock::millis());
        #endif

    }

}
#endif