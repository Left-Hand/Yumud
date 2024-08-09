#include "remote.hpp"
#include "types/float/bf16.hpp"

using E = bf16;

using Command = StepperEnums::Command;
using RunStatus = StepperEnums::RunStatus;

#define MSG(cmd, ...) CanMsg{(((uint32_t)(node_id) << 7) | (uint8_t)(cmd)), __VA_ARGS__}

// #define DEBUG_MSG(msg)
#define DEBUG_MSG(msg) DEBUG_PRINTLN(msg)

#define POST(cmd, ...)\
auto msg = MSG(cmd, __VA_ARGS__);\
DEBUG_MSG(msg);\
can.write(msg);\

bool RemoteFOCMotor::loadArchive(const bool outen){POST(Command::LOAD)return true;}
void RemoteFOCMotor::saveArchive(const bool outen){POST(Command::SAVE);}
void RemoteFOCMotor::removeArchive(const bool outen){POST(Command::CLEAR);}

void RemoteFOCMotor::setTargetVector(const real_t _pos){POST(Command::SET_TRG_VECT, E(ctrl_limits.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::freeze(){POST(Command::FREEZE);}
void RemoteFOCMotor::setTargetCurrent(const real_t current){POST(Command::SET_TRG_CURR, E(current));}
void RemoteFOCMotor::setTargetSpeed(const real_t speed){POST(Command::SET_TRG_SPD, E(speed));}
void RemoteFOCMotor::setTargetPosition(const real_t _pos){POST(Command::SET_TRG_POS, E(ctrl_limits.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::setTargetTrapezoid(const real_t _pos){POST(Command::SET_TRG_TPZ, E(ctrl_limits.pos_limit.clamp(_pos)));}
void RemoteFOCMotor::setOpenLoopCurrent(const real_t current){POST(Command::SET_OPEN_CURR, E(current));}
void RemoteFOCMotor::setCurrentLimit(const real_t max_current){POST(Command::SET_CURR_LMT, E(max_current));}
void RemoteFOCMotor::locateRelatively(const real_t _pos){POST(Command::LOCATE, E(_pos));}

bool RemoteFOCMotor::isActive() const{return true;}
const volatile RunStatus & RemoteFOCMotor::status(){POST(Command::STAT); return run_status;}

real_t RemoteFOCMotor::getSpeed() const{POST(Command::GET_SPD); return spd;}
real_t RemoteFOCMotor::getPosition() const{POST(Command::GET_SPD); return pos;}
real_t RemoteFOCMotor::getCurrent() const{POST(Command::GET_CURR); return curr;}

void RemoteFOCMotor::setPositionLimit(const Range & clamp){
    ctrl_limits.pos_limit = clamp;
    POST(Command::SET_POS_LMT, std::pair<E,E>{clamp.from, clamp.to});
}


void RemoteFOCMotor::enable(const bool en){POST(en ? Command::ACTIVE: Command::INACTIVE);}
void RemoteFOCMotor::setNodeId(const uint8_t _id){}
void RemoteFOCMotor::setSpeedLimit(const real_t max_spd){POST(Command::SET_SPD_LMT,E(max_spd));}
void RemoteFOCMotor::setAccelLimit(const real_t max_acc){POST(Command::SET_ACC_LMT, E(max_acc));}
void RemoteFOCMotor::triggerCali(){POST(Command::TRG_CALI);}
void RemoteFOCMotor::reset(){POST(Command::RST);}
void RemoteFOCMotor::setNozzle(const real_t duty){POST(duty ? Command::NOZZLE_ON : Command::NOZZLE_OFF);}

void RemoteFOCMotor::readCan(){

}