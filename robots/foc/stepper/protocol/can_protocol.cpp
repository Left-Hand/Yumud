#include "../stepper.hpp"
#include "can_protocol.hpp"


void FOCStepper::parseCommand(const Command command, const CanMsg & msg){
    using namespace CANProtocolUtils;

    const uint16_t tx_id = (((uint16_t)(node_id) << 7) | (uint8_t)(command));

    #define SET_METHOD_BIND_EXECUTE(cmd, method, ...)\
    case cmd:\
        method(__VA_ARGS__);\
        break;\

    #define SET_METHOD_BIND_TYPE(cmd, method, type)\
    case cmd:\
        method(type(msg));\
        break;\
    
    #define SET_VALUE_BIND(cmd, value)\
    case cmd:\
        value = (decltype(value)(msg));\
        break;\

    #define SET_METHOD_BIND_ONE(cmd, method) SET_METHOD_BIND_TYPE(cmd, method, E)

    #define GET_BIND_VALUE(cmd, ...)\
    case cmd:\
        if(msg.isRemote()){\
            can.write(CanMsg(tx_id, __VA_ARGS__));\
        }\
        break;\
    
    switch(command){

        SET_VALUE_BIND(Command::SET_TARGET, target)

        SET_METHOD_BIND_ONE(   Command::SET_TRG_VECT,  setTargetVector)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_CURR,  setTargetCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_POS,   setTargetPosition)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_SPD,   setTargetSpeed)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_TPZ,   setTargetTrapezoid)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_TEACH,   setTargetTeach)
        SET_METHOD_BIND_EXECUTE(Command::FREEZE,        freeze)

        SET_METHOD_BIND_ONE(   Command::LOCATE,        locateRelatively)
        SET_METHOD_BIND_ONE(   Command::SET_OPEN_CURR, setOpenLoopCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_CURR_LMT,  setCurrentLimit)
        SET_METHOD_BIND_TYPE(   Command::SET_POS_LMT,   setPositionLimit, E_2)
        SET_METHOD_BIND_ONE(   Command::SET_SPD_LMT,   setSpeedLimit)
        SET_METHOD_BIND_ONE(   Command::SET_ACC_LMT,   setAccelLimit)

        GET_BIND_VALUE(         Command::GET_POS,       E(getPosition()))
        GET_BIND_VALUE(         Command::GET_SPD,       E(getSpeed()))
        GET_BIND_VALUE(         Command::GET_ACC,       E(0))//TODO
        GET_BIND_VALUE(         Command::GET_CURR,      E(getCurrent()))
        GET_BIND_VALUE(         Command::GET_ALL,       E_4(getCurrent(), getSpeed(), getPosition(), getAccel()))

        SET_METHOD_BIND_EXECUTE(Command::TRG_CALI,          triggerCali)

        SET_METHOD_BIND_EXECUTE(Command::SAVE,          saveArchive)
        SET_METHOD_BIND_EXECUTE(Command::LOAD,          loadArchive)
        SET_METHOD_BIND_EXECUTE(Command::CLEAR,         removeArchive)

        SET_METHOD_BIND_EXECUTE(Command::NOZZLE_ON,     setNozzle, 1)
        SET_METHOD_BIND_EXECUTE(Command::NOZZLE_OFF,    setNozzle, 0)

        SET_METHOD_BIND_EXECUTE(Command::RST,           reset)
        GET_BIND_VALUE(         Command::STAT,          (uint8_t)run_status);
        SET_METHOD_BIND_EXECUTE(Command::INACTIVE,      enable, false)
        SET_METHOD_BIND_EXECUTE(Command::ACTIVE,        enable, true)
        SET_METHOD_BIND_EXECUTE(Command::SET_NODEID,    setNodeId, uint8_t(msg))

        default:
            CliSTA::parseCommand(command, msg);
            break;
    }

    #undef SET_METHOD_BIND
    #undef SET_VALUE_BIND
    #undef SET_METHOD_BIND_ONE
    #undef SET_VALUE_BIND_REAL
}
