#include "remote.hpp"

using Command = StepperEnums::Command;
using RunStatus = StepperEnums::RunStatus;

#define MSG(cmd) CanMsg{(((uint32_t)(node_id) << 7) | (uint8_t)(cmd))}

#define POST(cmd)\
can.write(MSG(cmd));

#define POST_VALUE(cmd, value, type)\
auto msg = MSG(cmd);\
can.write(msg.load<type>(value));\

#define POST_VALUE_REAL(cmd, value) POST_VALUE(cmd, value, real_t)

#define REQUEST_VALUE(cmd, value, type)\
can.write(MSG(cmd));\
return value;\

#define REQUEST_VALUE_REAL(cmd, value) REQUEST_VALUE(cmd, value, real_t)


bool RemoteStepper::loadArchive(const bool outen){
    POST(Command::LOAD)
    // can.write({(uint32_t)0x00});
    // can.write(MSG(Command::LOAD));
    return true;
}


void RemoteStepper::saveArchive(const bool outen){
    POST(Command::SAVE)
}
void RemoteStepper::removeArchive(const bool outen){
    POST(Command::RM)
}
bool RemoteStepper::autoload(const bool outen){
    // POST(Command::a)
    return true;
}

// void RemoteStepper::setTagretTrapezoid

void RemoteStepper::setTargetVector(const real_t pos){}
void RemoteStepper::setTargetCurrent(const real_t current){POST_VALUE_REAL(Command::TRG_CURR, current);}
void RemoteStepper::setTargetSpeed(const real_t speed){POST_VALUE_REAL(Command::TRG_SPD, speed);}
void RemoteStepper::setTargetPosition(const real_t pos){POST_VALUE_REAL(Command::TRG_POS, pos);}
void RemoteStepper::setTagretTrapezoid(const real_t pos){POST_VALUE_REAL(Command::TRG_TPZ, pos);}
void RemoteStepper::setOpenLoopCurrent(const real_t current){POST_VALUE_REAL(Command::SET_OLP_CURR, current);}
void RemoteStepper::setCurrentClamp(const real_t max_current){POST_VALUE_REAL(Command::CLAMP_CURRENT, max_current);}
void RemoteStepper::locateRelatively(const real_t pos){POST_VALUE_REAL(Command::LOCATE, pos);}

bool RemoteStepper::isActive() const{return true;}
const volatile RunStatus & RemoteStepper::status(){REQUEST_VALUE(Command::STAT, run_status, (RunStatus));}

real_t RemoteStepper::getSpeed() const{REQUEST_VALUE_REAL(Command::GET_SPD, spd);}
real_t RemoteStepper::getPosition() const{REQUEST_VALUE_REAL(Command::GET_SPD, pos);}
real_t RemoteStepper::getCurrent() const{REQUEST_VALUE_REAL(Command::GET_CURR, curr);}

void RemoteStepper::setTargetPositionClamp(const Range & clamp){}
void RemoteStepper::enable(const bool en){}
void RemoteStepper::setNodeId(const uint8_t _id){}
void RemoteStepper::setSpeedClamp(const real_t max_spd){}
void RemoteStepper::setAccelClamp(const real_t max_acc){}

void RemoteStepper::triggerCali(){}