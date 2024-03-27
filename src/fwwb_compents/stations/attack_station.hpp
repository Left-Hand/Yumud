#ifndef __FWWB_ATTACK_STATION_HPP__

#define __FWWB_ATTACK_STATION_HPP__

#include "target_station.hpp"
#include "../simple_modem.hpp"
#include "src/device/Actuator/Servo/FOC/tinyfoc.hpp"

namespace FWWB{

#define FATHER_STATION TargetStation
class AttackStation : public FATHER_STATION{
protected:
    MotorWithFoc & servo_yaw;
    PwmAngleServo & servo_pitch;
    SimpleModem & modem;

    real_t yaw;
    real_t pitch;
    uint8_t shot_remains = 0;
    uint8_t shot_code = 3;

    void setYaw(const real_t & _yaw){
        yaw = _yaw;
        servo_yaw.trackLapPos(yaw / TAU);
    }

    void setPitch(const real_t & _pitch){
        pitch = _pitch;
        servo_pitch.setAngle(real_t(RAD2ANGLE(pitch) + 90));
    }

    void setFace(const Vector2 & face){
        setYaw(face.x);
        setPitch(face.y);
    }

    void faceNotify(const CanMsg & msg){
        Vector2 vel;

        memcpy((void *)&vel, msg.getData(), sizeof(Vector2));

        setFace(vel);
    }

    void pitchNotify(const CanMsg & msg){
        setPitch(*reinterpret_cast<const real_t *>(msg.getData()));
    }

    void yawNotify(const CanMsg & msg){
        setYaw(*reinterpret_cast<const real_t *>(msg.getData()));
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        switch(cmd){
        case Command::ATTACK_SET_FACE:
            faceNotify(msg);
            break;
        case Command::ATTACK_GET_YAW:
            yawNotify(msg);
            break;
        case Command::ATTACK_SET_PITCH:
            pitchNotify(msg);
            break;
        case Command::ATTACK_SET_SHOT_SPEC:
            shot_code = msg[0];
            break;
        case Command::ATTACK_SHOT:
            shot_remains = msg[0];
            break;
        default:
            FATHER_STATION::parseCommand(cmd, msg);
            break;
        }
    }

    void runMachine() override{
        switch(sm){
        case StateMachine::ACTIVE:
            // if(modem.isIdle() && shot_remains > 0){
                // modem.sendCode(shot_code);
            //     shot_remains--;
            // }
            FATHER_STATION::runMachine();
            break;
        default:
            FATHER_STATION::runMachine();
            break;
        }
    }
public:
    AttackStation(FATHER_STATION & _instance, MotorWithFoc & _servo_yaw, PwmAngleServo & _servo_pitch, SimpleModem & _modem):
            FATHER_STATION(_instance), servo_yaw(_servo_yaw), servo_pitch(_servo_pitch), modem(_modem){;}

    void init() override{
        FATHER_STATION::init();
        servo_yaw.init();
        servo_yaw.enable();
        servo_pitch.init();
        servo_pitch.enable();
    }

};
#undef FATHER_STATION

};


#endif