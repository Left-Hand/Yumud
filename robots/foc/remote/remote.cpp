#include "remote.hpp"
#include "sys/math/float/bf16.hpp"

using namespace CANProtocolUtils;

using Command = StepperEnums::Command;
using RunStatus = StepperEnums::RunStatus;

#define MSG(cmd, ...) CanMsg{(((uint32_t)(node_id) << 7) | (uint8_t)(cmd)), __VA_ARGS__}

#define DEBUG_MSG(msg)
// #define DEBUG_MSG(msg) DEBUG_PRINTLN(msg)

#define POST(cmd, ...)\
auto msg = MSG(cmd, __VA_ARGS__);\
DEBUG_MSG(msg);\
can.write(msg);\

// while(can.pending()){;};

bool RemoteFOCMotor::loadArchive(const bool outen){POST(Command::LOAD)return true;}
void RemoteFOCMotor::saveArchive(const bool outen){POST(Command::SAVE);}
void RemoteFOCMotor::removeArchive(const bool outen){POST(Command::CLEAR);}

void RemoteFOCMotor::setTargetVector(const real_t _pos){POST(Command::SET_TRG_VECT, E(meta.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::freeze(){POST(Command::FREEZE);}
void RemoteFOCMotor::setTargetCurrent(const real_t current){POST(Command::SET_TRG_CURR, E(current));}
void RemoteFOCMotor::setTargetSpeed(const real_t speed){POST(Command::SET_TRG_SPD, E(speed));}
void RemoteFOCMotor::setTargetPosition(const real_t _pos){POST(Command::SET_TRG_POS, E(meta.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::setTargetTrapezoid(const real_t _pos){POST(Command::SET_TRG_TPZ, E(meta.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::setTargetTeach(const real_t _max_curr){POST(Command::SET_TRG_TEACH, E(_max_curr));}
void RemoteFOCMotor::setOpenLoopCurrent(const real_t current){POST(Command::SET_OPEN_CURR, E(current));}
void RemoteFOCMotor::setCurrentLimit(const real_t max_current){POST(Command::SET_CURR_LMT, E(max_current));}
void RemoteFOCMotor::locateRelatively(const real_t _pos){POST(Command::LOCATE, E(_pos));}

bool RemoteFOCMotor::isActive() const{return true;}
volatile RunStatus & RemoteFOCMotor::status(){POST(Command::STAT); return run_status;}

real_t RemoteFOCMotor::getCurrent() const{POST(Command::GET_CURR); return readCurrent();}
real_t RemoteFOCMotor::getSpeed() const{POST(Command::GET_SPD); return readSpeed();}
real_t RemoteFOCMotor::getPosition() const{POST(Command::GET_POS); return readPosition();}
real_t RemoteFOCMotor::getAccel() const{POST(Command::GET_ACC); return readAccel();}
void RemoteFOCMotor::updateAll() const{POST(Command::GET_ALL);}

void RemoteFOCMotor::setPositionLimit(const Range & clamp){
    meta.pos_limit = clamp;
    POST(Command::SET_POS_LMT, std::pair<E,E>{clamp.from, clamp.to});
}


void RemoteFOCMotor::enable(const bool en){POST(en ? Command::ACTIVE: Command::INACTIVE);}
void RemoteFOCMotor::setNodeId(const NodeId _id){}
void RemoteFOCMotor::setSpeedLimit(const real_t max_spd){POST(Command::SET_SPD_LMT,E(max_spd));}
void RemoteFOCMotor::setAccelLimit(const real_t max_acc){POST(Command::SET_ACC_LMT, E(max_acc));}
void RemoteFOCMotor::triggerCali(){POST(Command::TRG_CALI);}
void RemoteFOCMotor::reset(){POST(Command::RST);}
void RemoteFOCMotor::setNozzle(const real_t duty){POST(duty ? Command::NOZZLE_ON : Command::NOZZLE_OFF);}

void RemoteFOCMotor::parseCan(const CanMsg & msg){
    NodeId id = msg.id() >> 7;
    if(id != node_id) return;
    Command cmd = Command(msg.id() & 0x7f);

    parseCommand(id, cmd, msg);
}

void RemoteFOCMotor::parseCommand(const NodeId id, const Command cmd, const CanMsg &msg){
    if(id != node_id) return;
    switch(cmd){
        case Command::GET_POS:
            meta.pos = E(msg);
            break;
        case Command::GET_SPD:
            meta.spd = E(msg);
            break;
        case Command::GET_ACC:
            meta.accel = E(msg);
            break;
        case Command::GET_CURR:
            meta.curr = E(msg);
            break;
        case Command::GET_ALL:{
            auto res = E_4(msg);
            meta.curr =     std::get<0>(res);
            meta.spd =      std::get<1>(res);
            meta.pos =      std::get<2>(res);
            meta.accel =    std::get<3>(res);
            break;
        }
        default:
            break;
    }
}