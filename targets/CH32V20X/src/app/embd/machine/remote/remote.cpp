#include "remote.hpp"

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

void RemoteStepper::setTargetVector(const real_t _pos){POST(Command::SET_TRG_VECT, M_clamp.clamp(_pos));}
void RemoteStepper::freeze(){POST(Command::FREEZE);}
void RemoteStepper::setTargetCurrent(const real_t current){POST(Command::SET_TRG_CURR, current);}
void RemoteStepper::setTargetSpeed(const real_t speed){POST(Command::SET_TRG_SPD, speed);}
void RemoteStepper::setTargetPosition(const real_t _pos){POST(Command::SET_TRG_POS, M_clamp.clamp(_pos));}
void RemoteStepper::setTargetTrapezoid(const real_t _pos){POST(Command::SET_TRG_TPZ, M_clamp.clamp(_pos));}
void RemoteStepper::setOpenLoopCurrent(const real_t current){POST(Command::SET_OPEN_CURR, current);}
void RemoteStepper::setCurrentClamp(const real_t max_current){POST(Command::SET_CURR_LMT, max_current);}
void RemoteStepper::locateRelatively(const real_t _pos){POST(Command::LOCATE, _pos);}

bool RemoteStepper::isActive() const{return true;}
const volatile RunStatus & RemoteStepper::status(){REQUEST_VALUE(Command::STAT, run_status);}

real_t RemoteStepper::getSpeed() const{REQUEST_VALUE(Command::GET_SPD, spd);}
real_t RemoteStepper::getPosition() const{REQUEST_VALUE(Command::GET_SPD, pos);}
real_t RemoteStepper::getCurrent() const{REQUEST_VALUE(Command::GET_CURR, curr);}

void RemoteStepper::setPositionClamp(const Range & clamp){
    M_clamp = clamp;
    POST(Command::SET_POS_LMT, clamp);
}


void RemoteStepper::enable(const bool en){POST(en ? Command::ACTIVE: Command::INACTIVE);}
void RemoteStepper::setNodeId(const uint8_t _id){}
void RemoteStepper::setSpeedClamp(const real_t max_spd){POST(Command::SET_SPD_LMT, max_spd);}
void RemoteStepper::setAccelClamp(const real_t max_acc){POST(Command::SET_ACC_LMT, max_acc);}
void RemoteStepper::triggerCali(){POST(Command::TRG_CALI);}
void RemoteStepper::reset(){POST(Command::RST);}
void RemoteStepper::setNozzle(const real_t duty){POST(duty ? Command::NOZZLE_ON : Command::NOZZLE_OFF);}
