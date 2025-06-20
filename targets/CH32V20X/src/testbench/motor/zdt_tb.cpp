#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"

#include "core/string/StringView.hpp"
#include "atomic"

#ifdef ENABLE_UART1
using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

#define PHY_SEL PHY_SEL_UART


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
            sqrt(square(g.x_meters) + square(g.y_meters)),
            atan2(g.y_meters, g.x_meters)
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
    virtual bool is_homing_done() = 0;
};

class JointMotorAdapter_Mock final:
    public JointMotorIntf{
public:
    void activate() {}
    void deactivate() {}
    void set_position(real_t position) {}
    void trig_homing() {}
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
        stepper_.activate(EN);
    }

    void deactivate(){
        stepper_.activate(DISEN);
    }

    void set_position(real_t position){
        if(is_homed_ == false) 
            trip_and_panic("motor not homed");
        stepper_.set_target_position(position);
    }

    void trig_homing(){
        homing_begin_ = Some(clock::millis());
        stepper_.trig_homming(cfg_.homming_mode);
    }

    void trig_cali(){ 
        stepper_.trig_cali();
    }

    bool is_homing_done(){
        if(homing_begin_.is_none()){
            trip_and_panic("Homing not started");
            return false;
        }

        return (clock::millis() - homing_begin_.unwrap()) > homing_timeout_;
    }

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
private:
    Config cfg_;
    ZdtStepper & stepper_;

    static constexpr Milliseconds homing_timeout_ = 5000ms;
    Option<Milliseconds> homing_begin_ = None;
    std::atomic<bool> is_homed_ = false;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        PANIC(std::forward<Args>(args)...);
    }

    struct HomingStrategy_Timeout{

    };
};

class PolarRobotActuator{
public:
    struct Config{
        real_t rho_transform_scale;
        real_t phi_transform_scale;

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

    void set_position(const real_t x_meters, const real_t y_meters){
        const auto s = Solver::forward({
            .x_meters = x_meters,
            .y_meters = y_meters
        });

        const auto rho_position = s.rho_meters * cfg_.rho_transform_scale;
        const auto phi_position = s.phi_radians * cfg_.phi_transform_scale;

        if(not cfg_.rho_range.contains(rho_position))
            trip_and_panic("rho out of range");

        if(not cfg_.phi_range.contains(phi_position))
            trip_and_panic("phi out of range");

        joint_rho_.set_position(rho_position);
        joint_phi_.set_position(phi_position);
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
            DEF_MAKE_MEMFUNC(set_position)
        );
    }

private:
    const Config cfg_;
    JointMotorIntf & joint_rho_;
    JointMotorIntf & joint_phi_;

    template<typename ... Args>
    void trip_and_panic(Args && ... args){
        deactivate();
        PANIC(std::forward<Args>(args)...);
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
        PANIC("uncaught msg", msg);
        return HandleStatus::from_handled();
    }
};


static __attribute__((__noreturn__))
void polar_robot_main(){
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);


    auto & MOTOR1_UART = hal::uart1;
    auto & MOTOR2_UART = hal::uart1;

    MOTOR1_UART.init({921600});
    ZdtStepper motor1{{.nodeid = {1}}, &MOTOR1_UART};

    if(&MOTOR1_UART != &MOTOR2_UART){
        MOTOR2_UART.init({921600});
    }

    ZdtStepper motor2{{.nodeid = {2}}, &MOTOR2_UART};

    JointMotorAdapter_ZdtStepper joint1 = {{
        .homming_mode = ZdtStepper::HommingMode::LapsCollision
    }, motor1};

    JointMotorAdapter_ZdtStepper joint2 = {{
        .homming_mode = ZdtStepper::HommingMode::LapsEndstop
    }, motor2};

    PolarRobotActuator robot_actuator = {
        {
            .rho_transform_scale = 0.1_r,
            .phi_transform_scale = 0.1_r,

            .rho_range = {0.0_r, 0.2_r},
            .phi_range = {0.0_r, 1.0_r}
        },
        joint1, joint2
    };

    auto list = rpc::make_list(
        "polar_robot",
        robot_actuator.make_rpc_list("actuator"),
        joint1.make_rpc_list("joint1"),
        joint2.make_rpc_list("joint2")
    );

    robots::ReplService repl_service = {
        &DBG_UART, &DBG_UART
    };

    while(true){
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

        repl_service.invoke(list);
    }
}

void zdt_main(){
    polar_robot_main();
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({CanBaudrate::_1M});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor.activate();
    clock::delay(10ms);
    // motor.set_subdivides(256);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsCollision);
    motor.trig_homming(ZdtStepper::HommingMode::LapsEndstop);
    // motor.query_homming_paraments();

    robots::ReplService repl_service = {
        &DBG_UART, &DBG_UART
    };


    // auto list = rpc::make_list(
    //     "list",

    // );

    while(true){
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
    }

    clock::delay(10ms);

    // motor.trigger_cali();
    while(true){
        const auto t = clock::time();
        clock::delay(10ms);
        // motor.enable();
        const auto targ_pos = 0.1_r * sin(t);
        motor.set_target_position(targ_pos);

        // motor.set_target_position(sin(clock::time()));    
        clock::delay(10ms);
        // DEBUG_PRINTLN(clock::millis());

            
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                char chr;
                COMM_UART.read1(chr);
                recv.push_back(chr);
            }

            DEBUG_PRINTLN(std::hex, std::noshowbase, recv);
        }
        #else
        if(COMM_CAN.available()) DEBUG_PRINTLN(COMM_CAN.read());
        #endif
        

        // DEBUG_PRINTLN(COMM_CAN.pending(), COMM_CAN.getRxErrCnt(), COMM_CAN.getTxErrCnt());
    }
}
#endif