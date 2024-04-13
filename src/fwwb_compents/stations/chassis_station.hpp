#ifndef __FWWB_CHASSIS_STATION_HPP__

#define __FWWB_CHASSIS_STATION_HPP__


#include "target_station.hpp"
#include "src/device/Proximeter/VL53L0X/vl53l0x.hpp"
#include "src/device/VirtualIO/AW9523/aw9523.hpp"
#include "src/device/IMU/Gyroscope/QMC5883L/qmc5883l.hpp"

namespace FWWB{

class GM25{
protected:
    PwmChannel & pwm_channel;
    Gpio & dir_pin;
    CaptureChannelConcept & cap_channel;
    bool rsv;
    static constexpr float speed_scaler = 1 / (288 / 60 * TAU);
public:
    GM25(PwmChannel & _pwm_channel,Gpio & _dir_pin,CaptureChannelConcept & _cap_channel,const bool & _rsv = false):
            pwm_channel(_pwm_channel), dir_pin(_dir_pin), cap_channel(_cap_channel), rsv(_rsv){;}

    void setOmega(const real_t & speed){
        pwm_channel.setDuty(abs(speed) * speed_scaler);
        dir_pin = (speed > 0) ^ rsv;
    }

    void init(){
        pwm_channel.init();
        dir_pin.OutPP();
        cap_channel.init();
    }
};


class ChassisStation : public TargetStation{
protected:
    virtual void updateMotorTarget() = 0;
    VL53L0X & vl;
    GpioConcept & ir_left;
    GpioConcept & ir_right;

    void moveNotify(const CanMsg & msg){
        Vector2 vel;

        memcpy((void *)&vel, msg.getData(), sizeof(Vector2));
        setMove(vel);
        updateMotorTarget();
    }

    void omegaNotify(const CanMsg & msg){
        real_t omega;
        memcpy(&omega, msg.getData(), sizeof(real_t));
        setOmega(omega);
        updateMotorTarget();
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        if(!msg.isRemote()) return;
        logger.println("cmd:", (uint8_t)cmd);
        switch(cmd){
        case Command::CHASSIS_SET_MOVE:
            moveNotify(msg);
            break;
        case Command::CHASSIS_SET_OMEGA:
            omegaNotify(msg);
            break;
        default:
            TargetStation::parseCommand(cmd, msg);
            break;
        }
    }

    void runMachine() override{
        TargetStation::runMachine();
    }
public:

    ChassisStation(TargetStation & _instance, VL53L0X & _vl, GpioConcept & _ir_left, GpioConcept & _ir_right):
        TargetStation(_instance), vl(_vl), ir_left(_ir_left), ir_right(_ir_right){;}

    void init() override{
        TargetStation::init();
    }

    virtual void setMove(const Vector2 & move) = 0;
    virtual void setOmega(const real_t & omega) = 0;
};

class DiffChassisStation : public ChassisStation{
protected:
    GM25 & motor_left;
    GM25 & motor_right;

    real_t omega_comm;
    real_t omega_diff;

    static constexpr float wheel_radius = 0.047 / 2;
    static constexpr float wheel_base = 0.1;
    PID_t<real_t> pos_pid = PID_t<real_t>(real_t(0.01), real_t(0), real_t(0), real_t(0.4));

    void updateMotorTarget() override{
        motor_left.setOmega(omega_comm - omega_diff);
        motor_right.setOmega(omega_comm + omega_diff);
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        ChassisStation::parseCommand(cmd, msg);
    }

    void runMachine() override{
        ChassisStation::runMachine();

        if(mode){
            vl.update();
            real_t force_x = pos_pid.update(real_t(vl.getDistance()), real_t(100));
            real_t force_y = real_t(bool(ir_right) - bool(ir_left)) * PI;
            setMove(Vector2(force_x, force_y));
        }
    }

public:
    DiffChassisStation(TargetStation & _instance, VL53L0X & _vl, GpioConcept & _ir_left, GpioConcept & _ir_right, GM25 & _motor_left, GM25 & _motor_right):
            ChassisStation(_instance, _vl, _ir_left, _ir_right), motor_left(_motor_left), motor_right(_motor_right){;}

    void init() override{
        ChassisStation::init();
        motor_left.init();
        motor_right.init();
    }

    void setMove(const Vector2 & move) override{
        omega_comm = move.x * real_t(1 / wheel_radius);
        setOmega(move.y);
    }

    void setOmega(const real_t & omega) override{
        omega_diff = omega * real_t(wheel_base / 2 / wheel_radius);
        updateMotorTarget();
    }


};

};
#endif