#ifndef __FWWB_CHASSIS_STATION_HPP__

#define __FWWB_CHASSIS_STATION_HPP__


#include "target_station.hpp"

namespace FWWB{

class GM25{
protected:
    PwmChannel & pwm_channel;
    Gpio & dir_pin;
    CaptureChannelConcept & cap_channel;
    bool rsv;
    static constexpr int speed_scaler = 400;
public:
    GM25(PwmChannel & _pwm_channel,Gpio & _dir_pin,CaptureChannelConcept & _cap_channel,const bool & _rsv = false):
            pwm_channel(_pwm_channel), dir_pin(_dir_pin), cap_channel(_cap_channel), rsv(_rsv){;}

    void setOmega(const real_t & speed){
        pwm_channel.setDuty(abs(speed / speed_scaler));
        dir_pin = (speed > 0) ^ rsv;
    }

    void init(){
        dir_pin.OutPP();
    }
};


class ChassisStation : public TargetStation{
protected:
    virtual void updateMotorTarget() = 0;
    virtual void setVelocity(const Vector2 & velocity) = 0;
    virtual void setOmega(const real_t & omega) = 0;

    void velocityNotify(const CanMsg & msg){
        Vector2 vel;

        memcpy((void *)&vel, msg.getData(), sizeof(Vector2));
        vel.x *= 60;
        vel.y *= 60;
        setVelocity(vel);
        setOmega(vel.y);
        updateMotorTarget();
        logger.println(vel.x, vel.y);
    }

    void omegaNotify(const CanMsg & msg){
        setOmega(*reinterpret_cast<const real_t *>(msg.getData()));
        updateMotorTarget();
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        TargetStation::parseCommand(cmd, msg);
        switch(cmd){
        case Command::CHASSIS_SET_VEL:
            velocityNotify(msg);
            break;
        case Command::CHASSIS_SET_OMEGA:
            omegaNotify(msg);
            break;
        default:
            break;
        }
    }

    void runMachine() override{
        TargetStation::runMachine();
    }
public:

    ChassisStation(TargetStation & _instance): TargetStation(_instance){;}

    void init() override{
        TargetStation::init();
    }
};

class DiffChassisStation : public ChassisStation{
protected:
    GM25 & motor_left;
    GM25 & motor_right;

    real_t omega_comm;
    real_t omega_diff;

    static constexpr float wheel_radius = 0.047 / 2;
    static constexpr float wheel_base = 0.1;

    void updateMotorTarget() override{
        motor_left.setOmega(omega_comm - omega_diff);
        motor_right.setOmega(omega_comm + omega_diff);
    }

    void setVelocity(const Vector2 & velocity) override{
        omega_comm = velocity.x / real_t(wheel_radius);
    }

    void setOmega(const real_t & omega) override{
        omega_diff = omega * real_t(wheel_base / 2 / wheel_radius);
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        ChassisStation::parseCommand(cmd, msg);
    }

    void runMachine() override{
        ChassisStation::runMachine();
    }

public:
    DiffChassisStation(TargetStation & _instance, GM25 & _motor_left, GM25 & _motor_right):
            ChassisStation(_instance), motor_left(_motor_left), motor_right(_motor_right){;}

    void init() override{
        ChassisStation::init();
        motor_left.init();
        motor_right.init();
    }
};

};
#endif