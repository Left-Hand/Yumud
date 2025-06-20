#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"

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
    virtual void set_position(real_t position) = 0;
    virtual void trig_homing() = 0;
    virtual bool is_homing_done() = 0;
};

class JointMotorAdapterForZdtStepper final
    :public JointMotorIntf{
public:

    struct Config{
        ZdtStepper::HommingMode homming_mode;
    };

    JointMotorAdapterForZdtStepper(
        const Config & cfg, 
        ZdtStepper & stepper
    ):
        cfg_(cfg),
        stepper_(stepper){;}

    void activate(){
        stepper_.activate();
    }

    void set_position(real_t position){
        stepper_.set_target_position(position);

    }

    void trig_homing(){
        homing_begin_ = Some(clock::millis());
        stepper_.trig_homming(cfg_.homming_mode);
    }

    bool is_homing_done(){
        if(homing_begin_.is_none()){
            PANIC("Homing not started");
            return false;
        }

        return (clock::millis() - homing_begin_.unwrap()) > homing_timeout_;
    }
private:
    Config cfg_;
    ZdtStepper & stepper_;

    static constexpr Milliseconds homing_timeout_ = 5000ms;
    Option<Milliseconds> homing_begin_ = None;

    struct HomingStrategyTimeout{
        
    };
};

class PolarRobotActuator{
public:
    struct Config{
        real_t rho_transform_scale;
        real_t phi_transform_scale;
    };

    using Solver = PolarRobotSolver;
    using Gesture = typename Solver::Gesture;
    using Solution = typename Solver::Solution;

    PolarRobotActuator(
        const Config & cfg, 
        JointMotorIntf & joint_rho,
        JointMotorIntf & joint_phi
    ):
        cfg_(cfg),
        joint_rho_(joint_rho),
        joint_phi_(joint_phi)
    {;}


    void set_position(const Gesture & g){
        const auto s = Solver::forward(g);

        const auto rho_position = s.rho_meters * cfg_.rho_transform_scale;
        const auto phi_position = s.phi_radians * cfg_.phi_transform_scale;

        joint_rho_.set_position(rho_position);
        joint_phi_.set_position(phi_position);
    }

    void activate(){
        joint_rho_.activate();
        joint_phi_.activate();
    }

    void trig_homing(){
        joint_rho_.trig_homing();   
        joint_phi_.trig_homing();
    }

    bool is_homing_done(){
        return joint_rho_.is_homing_done() 
            && joint_phi_.is_homing_done();
    }

private:
    const Config cfg_;
    JointMotorIntf & joint_rho_;
    JointMotorIntf & joint_phi_;
};

void zdt_main(){
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