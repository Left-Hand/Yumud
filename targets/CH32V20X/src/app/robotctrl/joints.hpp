#pragma once

#include <atomic>

#include "core/utils/serde/serde.hpp"

#include "robots/vendor/zdt/zdt_frame_factory.hpp"

#include "middlewares/repl/repl.hpp"


namespace ymd::robots{

class JointMotorActuatorIntf{
public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void set_angle(Angular<real_t> angle) = 0;
    virtual Angular<real_t> last_angle() = 0;
    virtual void trig_homing() = 0;
    virtual void trig_cali() = 0;
    virtual bool is_homing_done() = 0;

    auto make_repl_list(const StringView name){
        return script::make_list(
            name,
            DEF_CALLABLE_MEMFUNC(trig_homing),
            DEF_CALLABLE_MEMFUNC(is_homing_done),
            DEF_CALLABLE_MEMFUNC(deactivate),
            DEF_CALLABLE_MEMFUNC(activate),
            // DEF_CALLABLE_MEMFUNC(set_angle),
            // repl::make_memfunc("trig_homing", this, &std::decay_t<decltype(*this)>::trig_homing)
            DEF_CALLABLE_MEMFUNC(trig_cali)
        );
    }
};


class MockJointMotorActuator final:
    public JointMotorActuatorIntf{
public:
    void activate() {}
    void deactivate() {}
    void set_angle(Angular<real_t> angle) {
        angle = position_;
    }
    void trig_homing() {}
    void trig_cali() {}
    bool is_homing_done() {return true;}
    Angular<real_t> last_angle(){return 0_deg;}
private:
    Angular<real_t> position_ = 0_deg;
};

class ZdtJointMotorActuator final
    :public JointMotorActuatorIntf{
public:

    struct Config{
        zdtmotor::HommingMode homming_mode;
    };

    ZdtJointMotorActuator(
        const Config & cfg, 
        zdtmotor::ZdtFrameFactory & factory
    ):
        cfg_(cfg),
        factory_(factory){;}

    void activate(){
        const auto flat_packet = factory_.activate(EN);
        write_packet(flat_packet);
    }

    void deactivate(){
        const auto flat_packet = factory_.activate(DISEN);;
        write_packet(flat_packet);
    }

    void set_angle(Angular<real_t> angle){
        last_angle_ = angle;
        const auto flat_packet = factory_.set_angle(
            angle,
            0.47_r
        );
        write_packet(flat_packet);
    }

    Angular<real_t> last_angle(){
        return last_angle_ ;
    }

    void trig_homing(){
        homing_begin_ = Some(clock::millis());
        const auto flat_packet = factory_.trig_homming(cfg_.homming_mode);;
        write_packet(flat_packet);
    }

    void trig_cali(){ 
        const auto flat_packet = factory_.trig_cali();;
        write_packet(flat_packet);
    }

    bool is_homing_done(){
        if(homing_begin_.is_none()){
            trip_and_panic("Homing not started");
            return false;
        }

        return (clock::millis() - homing_begin_.unwrap()) > HOMING_TIMEOUT_;
    }

    void write_packet(const zdtmotor::FlatPacket & packet){
        auto && iter = packet.to_canframe_iter();
        while(iter.has_next()){
            hal::can1.try_write(iter.next()).examine();
        }
    };
private:
    static constexpr Milliseconds HOMING_TIMEOUT_ = 5000ms;

    Config cfg_;
    zdtmotor::ZdtFrameFactory & factory_;

    Angular<real_t> last_angle_;
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