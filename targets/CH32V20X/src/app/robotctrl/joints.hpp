#pragma once

#include "robots/vendor/zdt/zdt_stepper.hpp"
#include "core/utils/serde.hpp"
#include "robots/rpc/rpc.hpp"
#include <atomic>

namespace ymd::robots{

class JointMotorActuatorIntf{
public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void set_angle(Angle<real_t> angle) = 0;
    virtual Angle<real_t> last_angle() = 0;
    virtual void trig_homing() = 0;
    virtual void trig_cali() = 0;
    virtual bool is_homing_done() = 0;

    auto make_rpc_list(const StringView name){
        return rpc::make_list(
            name,
            DEF_RPC_MEMFUNC(trig_homing),
            DEF_RPC_MEMFUNC(is_homing_done),
            DEF_RPC_MEMFUNC(deactivate),
            DEF_RPC_MEMFUNC(activate),
            // DEF_RPC_MEMFUNC(set_angle),
            // rpc::make_memfunc("trig_homing", this, &std::decay_t<decltype(*this)>::trig_homing)
            DEF_RPC_MEMFUNC(trig_cali)
        );
    }
};


class MockJointMotorActuator final:
    public JointMotorActuatorIntf{
public:
    void activate() {}
    void deactivate() {}
    void set_angle(Angle<real_t> angle) {
        angle = position_;
    }
    void trig_homing() {}
    void trig_cali() {}
    bool is_homing_done() {return true;}
    Angle<real_t> last_angle(){return 0_deg;}
private:
    Angle<real_t> position_ = 0_deg;
};

class ZdtJointMotorActuator final
    :public JointMotorActuatorIntf{
public:
    using ZdtStepper = zdtmotor::ZdtStepper;

    struct Config{
        ZdtStepper::HommingMode homming_mode;
    };

    ZdtJointMotorActuator(
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

    void set_angle(Angle<real_t> angle){
        last_angle_ = angle;
        stepper_.set_angle({
            .angle = angle,
            .speed = 0.47_r
        }).unwrap();
    }

    Angle<real_t> last_angle(){
        return last_angle_ ;
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

        return (clock::millis() - homing_begin_.unwrap()) > HOMING_TIMEOUT_;
    }


private:
    static constexpr Milliseconds HOMING_TIMEOUT_ = 5000ms;

    Config cfg_;
    ZdtStepper & stepper_;

    Angle<real_t> last_angle_;
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
}