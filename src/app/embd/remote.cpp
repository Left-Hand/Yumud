#include "remote.hpp"

using Command = StepperEnums::Command;
using RunStatus = StepperEnums::RunStatus;

#define MSG(cmd, ...) CanMsg{(((uint32_t)(node_id) << 7) | (uint8_t)(cmd)), __VA_ARGS__}

// #define DEBUG_MSG(msg) logger.println( msg.length(), msg.isRemote(), msg.to<real_t>())
#define DEBUG_MSG(msg)

#define POST(cmd, ...)\
auto msg = MSG(cmd, __VA_ARGS__);\
DEBUG_MSG(msg);\
can.write(msg);\

// #define POST(cmd, value) POST(cmd, value)

// #define POST(cmd, value) POST(cmd, value, real_t)

#define REQUEST_VALUE(cmd, value)\
can.write(MSG(cmd));\
return value;\

// #define REQUEST_VALUE(cmd, value) REQUEST_VALUE(cmd, value, real_t)


bool RemoteStepper::loadArchive(const bool outen){
    POST(Command::LOAD)
    // can.write({(uint32_t)0x00});
    // can.write(MSG(Command::LOAD));
    return true;
}


void RemoteStepper::saveArchive(const bool outen){POST(Command::SAVE);}
void RemoteStepper::removeArchive(const bool outen){POST(Command::RM);}
bool RemoteStepper::autoload(const bool outen){return true;}

void RemoteStepper::setTargetVector(const real_t pos){}
void RemoteStepper::setTargetCurrent(const real_t current){POST(Command::TRG_CURR, current);}
void RemoteStepper::setTargetSpeed(const real_t speed){POST(Command::TRG_SPD, speed);}
void RemoteStepper::setTargetPosition(const real_t pos){POST(Command::TRG_POS, pos);}
void RemoteStepper::setTargetTrapezoid(const real_t pos){POST(Command::TRG_TPZ, pos);}
void RemoteStepper::setOpenLoopCurrent(const real_t current){POST(Command::SET_OLP_CURR, current);}
void RemoteStepper::setCurrentClamp(const real_t max_current){POST(Command::CLAMP_CURRENT, max_current);}
void RemoteStepper::locateRelatively(const real_t pos){POST(Command::LOCATE, pos);}

bool RemoteStepper::isActive() const{return true;}
const volatile RunStatus & RemoteStepper::status(){REQUEST_VALUE(Command::STAT, run_status);}

real_t RemoteStepper::getSpeed() const{REQUEST_VALUE(Command::GET_SPD, spd);}
real_t RemoteStepper::getPosition() const{REQUEST_VALUE(Command::GET_SPD, pos);}
real_t RemoteStepper::getCurrent() const{REQUEST_VALUE(Command::GET_CURR, curr);}

void RemoteStepper::setTargetPositionClamp(const Range & clamp){}
void RemoteStepper::enable(const bool en){}
void RemoteStepper::setNodeId(const uint8_t _id){}
void RemoteStepper::setSpeedClamp(const real_t max_spd){}
void RemoteStepper::setAccelClamp(const real_t max_acc){}

void RemoteStepper::triggerCali(){POST(Command::CALI);}

void RemoteStepper::reset(){POST(Command::RST);}

void RemoteStepper::mt(const bool en){
    do{
        if(en){
            POST(Command::SERVO_ON);
        }else{
            POST(Command::SERVO_OFF);
        }
    }while(false);
}