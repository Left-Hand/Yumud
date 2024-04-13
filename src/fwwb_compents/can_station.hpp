#ifndef __FWWB_CAN_STATION_HPP__
#define __FWWB_CAN_STATION_HPP__

#include "src/bus/can/can1.hpp"
#include "src/system.hpp"

#include "src/misc.h"
#include "enums.hpp"
#include <algorithm>

#ifndef FWWB_DEBUG
#define FWWB_DEBUG(...) logger.println(__VA_ARGS__)
#endif

namespace FWWB{
using Sys::t;
static constexpr const char supported_manu[] = "Rstr1aN";

static std::array<uint32_t, 14> sta_crcs = {
    0xC3C0612,
    1180707222,
    3458428716,
    205260306
};

__fast_inline
int8_t getDeaultID(const uint32_t crc_code){
    for(int i = 0; i < 14; i++){
        if(sta_crcs[i] == crc_code) return i;
    }
    return -1;
}

class CanFacility{
protected:
    Can & can;
    Printer & logger;
    uint8_t node_id = 0;
    uint8_t weight = 0;
    uint8_t mode = 0;

public:
    CanFacility(Can & _can, Printer & _logger, const uint8_t & _node_id) : can(_can), logger(_logger), node_id(_node_id) {;}
};

class CanStation:public CanFacility{
protected:
    enum class StateMachine:uint8_t{
        POWER_ON,
        INACTIVE,
        ACTIVE
    };

    volatile StateMachine sm = StateMachine::POWER_ON;

    void sendCommand(const Command & command, const uint8_t *buf, const uint8_t len){
        uint16_t std_id = (uint16_t)((uint8_t)command << 4 | node_id);
        can.write(CanMsg(std_id, buf, len));
    }

    void sendCommand(const Command & command){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | node_id), true));
    }

    void powerOnNotify(){
        union MsgFormat{
            uint8_t buf[4];
            uint32_t val;
        }msgFormat;
        msgFormat.val = Sys::Chip::getChipIdCrc();
        // FWWB_DEBUG("ChipCrc: ", msgFormat.val);
        sendCommand(Command::POWER_ON, msgFormat.buf, 4);
    }

    void manuIdNotify(){
        FWWB_DEBUG("Manu: ", supported_manu);
        sendCommand(Command::MANU_ID, (const uint8_t *)supported_manu, 8);
    }

    void resetNotified(){
        FWWB_DEBUG("rst");
        NVIC_SystemReset();
        // Sys::Reset();
    }

    void scanNotified(){

    }

    void SyncNotified(const CanMsg & msg){
        uint32_t new_tick;
        memcpy(&new_tick, msg.getData(), sizeof(uint32_t));
        SetTick(new_tick);
    }

    void registerNodeIDNotified(const CanMsg & msg){
        struct MsgFormat{
            union{
                uint8_t buf[4];
                uint32_t val;
            };
            uint8_t node_id;
        };

        const MsgFormat& format = *reinterpret_cast<const MsgFormat*>(msg.getData());

        uint32_t crc_code = Sys::Chip::getChipIdCrc();
        if(memcmp((void *)crc_code, (void *)format.buf, 4)){
            node_id = format.node_id;
        }
    }

    virtual void parseCommand(const Command & cmd, const CanMsg & msg){
        if(!msg.isRemote()) return;
        switch(cmd){
        case Command::INACTIVE:
            sm = StateMachine::INACTIVE;
            break;
        case Command::ACTIVE:
            sm = StateMachine::ACTIVE;
            break;
        case Command::SCAN:
            break;
        case Command::REGISTER_NODEID:
            registerNodeIDNotified(msg);
        case Command::MANU_ID:
            manuIdNotify();
        case Command::POWER_ON:
            sm = StateMachine::POWER_ON;
            break;
        case Command::RST:
            resetNotified();
            break;
        case Command::SYNC:
            SyncNotified(msg);
            break;
        default:
            break;
        }
    }

    virtual void runMachine(){
        switch(sm){
        case StateMachine::POWER_ON:
            powerOnNotify();
            manuIdNotify();
            sm = StateMachine::ACTIVE;
            break;

        case StateMachine::INACTIVE:
            break;

        default:
            break;
        }
    }
public:
    CanStation(Can & _can, Printer & _logger) : CanFacility(_can, _logger, 0) {
        auto default_id = getDeaultID(Sys::Chip::getChipIdCrc());
        if(default_id >= 0){
            node_id = default_id;
        }else{
            node_id = 0;
        }
    }
    CanStation(Can & _can, Printer & _logger, const uint8_t & _node_id) : CanFacility(_can, _logger, _node_id) {;}

    virtual void init(){
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

        runMachine();
    }

    void setMode(const bool & _mode){
        mode = _mode;
    }
};
};
#endif