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

    Vector2 face_target;
    Vector2 face_actual;

    Range_t<uint32_t> shot_time_window;
    uint8_t shot_code = 3;

    void towardNotified(const CanMsg & msg){
        Vector2 face;
        memcpy(&face, msg.getData(), sizeof(Vector2));
        setFace(face);
    }

    void towardNotify(){
        getFace();
        uint8_t buf[8] = {0};
        memcpy(buf, PTR8_AND_SIZE(face_actual));
        sendCommand(Command::ATTACK_GET_TOWARD, PTR8_AND_SIZE(buf));
    }

    void pitchNotified(const CanMsg & msg){
        real_t pitch;
        memcpy(&pitch, msg.getData(), sizeof(real_t));
        setPitch(pitch);
    }

    void yawNotified(const CanMsg & msg){
        real_t yaw;
        memcpy(&yaw, msg.getData(), sizeof(real_t));
        setYaw(yaw);
    }

    void shotNotified(const CanMsg & msg){
        shotMs(msg[0]);
        setShotCode(msg[1]);
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        // if(!msg.isRemote()) return;
        switch(cmd){
        case Command::ATTACK_SET_ID:
            node_id = msg[0];
            break;
        case Command::ATTACK_GET_ID:
            {
                uint8_t buf[1] = {node_id};
                sendCommand(Command::ATTACK_GET_ID, PTR8_AND_SIZE(buf));
            }
            break;
        case Command::ATTACK_SET_TOWARD:
            towardNotified(msg);
            break;
        case Command::ATTACK_GET_TOWARD:
            towardNotify();
            break;
        case Command::ATTACK_GET_YAW:
            yawNotified(msg);
            break;
        case Command::ATTACK_SET_PITCH:
            pitchNotified(msg);
            break;
        case Command::ATTACK_SHOT:
            shotNotified(msg);
            break;
        default:
            FATHER_STATION::parseCommand(cmd, msg);
            break;
        }
    }

    void runMachine() override{
        switch(sm){
        case StateMachine::ACTIVE:
            if(modem.isIdle() && shot_time_window.has_value(millis())){
                modem.sendCode(shot_code);
            }
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
        setFace(Vector2(0, 0));
    }


    void setShotCode(const uint8_t & _code){
        shot_code = _code;
    }

    void shotMs(const uint16_t & _remain_millis){
        shot_time_window.start = millis();
        shot_time_window.end = shot_time_window.start + _remain_millis;
    }

    void setYaw(const real_t & _yaw){
        servo_yaw.trackLapPos(_yaw / TAU);
    }

    void setPitch(const real_t & _pitch){
        servo_pitch.setAngle(real_t(RAD2ANGLE(_pitch)) + 90);
    }

    void setFace(const Vector2 & _face){
        face_target = _face;
        setYaw(_face.x);
        setPitch(_face.y);
    }

    Vector2 getFace(){
        face_actual.x = getYaw();
        face_actual.y = getPitch();
        return face_actual;
    }

    real_t getYaw(){
        return servo_yaw.getPosition() * TAU;
    }

    real_t getPitch(){
        return face_target.y;
    }
};
#undef FATHER_STATION

};


#endif