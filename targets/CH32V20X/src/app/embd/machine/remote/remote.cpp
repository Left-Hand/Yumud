#include "remote.hpp"
#include "types/float/bf16.hpp"

using E = bf16;

using Command = StepperEnums::Command;
using RunStatus = StepperEnums::RunStatus;

#define MSG(cmd, ...) CanMsg{(((uint32_t)(node_id) << 7) | (uint8_t)(cmd)), __VA_ARGS__}

#define DEBUG_MSG(msg)

#define POST(cmd, ...)\
auto msg = MSG(cmd, __VA_ARGS__);\
DEBUG_MSG(msg);\
can.write(msg);\


#define REQUEST_VALUE(cmd, value)\
can.write(MSG(cmd));\
return value;\


bool RemoteStepper::loadArchive(const bool outen){POST(Command::LOAD)return true;}
void RemoteStepper::saveArchive(const bool outen){POST(Command::SAVE);}
void RemoteStepper::removeArchive(const bool outen){POST(Command::CLEAR);}

void RemoteStepper::setTargetVector(const real_t _pos){POST(Command::SET_TRG_VECT, E(M_clamp.clamp(_pos)));}
void RemoteStepper::freeze(){POST(Command::FREEZE);}
void RemoteStepper::setTargetCurrent(const real_t current){POST(Command::SET_TRG_CURR, E(current));}
void RemoteStepper::setTargetSpeed(const real_t speed){POST(Command::SET_TRG_SPD, E(speed));}
void RemoteStepper::setTargetPosition(const real_t _pos){POST(Command::SET_TRG_POS, E(M_clamp.clamp(_pos)));}
void RemoteStepper::setTargetTrapezoid(const real_t _pos){POST(Command::SET_TRG_TPZ, E(M_clamp.clamp(_pos)));}
void RemoteStepper::setOpenLoopCurrent(const real_t current){POST(Command::SET_OPEN_CURR, E(current));}
void RemoteStepper::setCurrentClamp(const real_t max_current){POST(Command::SET_CURR_LMT, E(max_current));}
void RemoteStepper::locateRelatively(const real_t _pos){POST(Command::LOCATE, E(_pos));}

bool RemoteStepper::isActive() const{return true;}
const volatile RunStatus & RemoteStepper::status(){REQUEST_VALUE(Command::STAT, run_status);}

real_t RemoteStepper::getSpeed() const{E _spd; REQUEST_VALUE(Command::GET_SPD, _spd);spd = _spd;}
real_t RemoteStepper::getPosition() const{E _pos; REQUEST_VALUE(Command::GET_SPD, _pos); pos = _pos;}
real_t RemoteStepper::getCurrent() const{E _curr; REQUEST_VALUE(Command::GET_CURR, curr); curr = _curr;}

void RemoteStepper::setPositionClamp(const Range & clamp){
    M_clamp = clamp;
    
    POST(Command::SET_POS_LMT, std::pair<E,E>{clamp.from, clamp.to});
}


void RemoteStepper::enable(const bool en){POST(en ? Command::ACTIVE: Command::INACTIVE);}
void RemoteStepper::setNodeId(const uint8_t _id){}
void RemoteStepper::setSpeedClamp(const real_t max_spd){POST(Command::SET_SPD_LMT,E(max_spd));}
void RemoteStepper::setAccelClamp(const real_t max_acc){POST(Command::SET_ACC_LMT, E(max_acc));}
void RemoteStepper::triggerCali(){POST(Command::TRG_CALI);}
void RemoteStepper::reset(){POST(Command::RST);}
void RemoteStepper::setNozzle(const real_t duty){POST(duty ? Command::NOZZLE_ON : Command::NOZZLE_OFF);}