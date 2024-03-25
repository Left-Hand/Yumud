#ifndef __FWWB_CAN_ACCESS_POINT_HPP__

#define __FWWB_CAN_ACCESS_POINT_HPP__

#include "../can_station.hpp"

class CanAcessPoint:public CanFacility{
protected:
    enum class StateMachine:uint8_t{
        POWER_ON,
        ACTIVE
    };

    volatile StateMachine sm = StateMachine::POWER_ON;

    void sendCommand(const uint8_t & sta_id, const Command & command, const uint8_t *buf, const uint8_t len){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | sta_id), buf, len));
    }

    void sendCommand(const uint8_t & sta_id, const Command & command){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | sta_id), true));
    }

    bool sta_valid[14] = {false};

    void scanStations(){
        struct MsgFormat{
            union{
                uint8_t buf[4];
                uint32_t val;
            };
            uint8_t node_id;
        };

        for(uint8_t i = 0; i < 14; i++){
            MsgFormat content{.val = sta_crcs[i], .node_id = (uint8_t)(i + 1)};
            sendCommand(i + 1, Command::SCAN, PTR8_AND_SIZE(content));
        }
    }
public:
    CanAcessPoint(Can & can) : CanFacility(can, 0xf) {;}

    virtual void parseCommand(const Command & cmd, const CanMsg & msg){
        switch(cmd){
        case Command::SCAN:

        default:
            break;
        }
    }

    void run(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.getId() & 0b1111;
            if(id == 0 || id == node_id){
                Command cmd = (Command)(msg.getId() >> 4);
                parseCommand(cmd, msg);
            }
        }
    }
};

#endif