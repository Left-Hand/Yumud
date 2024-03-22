#ifndef __FWWB_CAN_STATION_HPP__
#define __FWWB_CAN_STATION_HPP__

#include "src/bus/can/can1.hpp"
#include "enums.hpp"

namespace FWWB{
class CanStation{
protected:

    Can & can;

    uint8_t node_id = 0;
    using Command = CanCommand;

    volatile void sendCommand(const Command & command, const uint8_t *buf, const uint8_t len){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | node_id), buf, len));
    }

    volatile void sendCommand(const Command & command){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | node_id), true));
    }

    void PowerOnNotify(){
        sendCommand(Command::POWER_ON);
    }


    enum class StateMachine:uint8_t{
        POWER_ON,
        INACTIVE,
        ACTIVE
    };

    volatile StateMachine sm = StateMachine::POWER_ON;

    virtual void parseCommand(const CanCommand & cmd, const CanMsg & msg){
        switch(cmd){
        case CanCommand::POWER_ON:
            sm = StateMachine::POWER_ON;
            break;
        case CanCommand::INACTIVE:
            sm = StateMachine::INACTIVE;
            break;
        case CanCommand::ACTIVE:
            sm = StateMachine::ACTIVE;
            break;
        case CanCommand::RST:
            Sys::Reset();
        default:
            break;
        }
    }

    virtual void runMachine(){
        switch(sm){
        case StateMachine::POWER_ON:
            PowerOnNotify();
            sm = StateMachine::ACTIVE;
            break;

        case StateMachine::INACTIVE:
            break;

        default:
            break;
        }
    }
public:
    CanStation(Can & _can) : can(_can){;}

    virtual void init() = 0;

    void run(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.getId() & 0b1111;
            if(id == node_id){
                CanCommand cmd = (CanCommand)(msg.getId() >> 4);
                parseCommand(cmd, msg);
            }
        }

        runMachine();
    }
};
};
#endif