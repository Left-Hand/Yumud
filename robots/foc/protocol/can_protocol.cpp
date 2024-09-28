#include "robots/foc/focmotor.hpp"


CanProtocolConcept::NodeId CanProtocolConcept::getDefaultNodeId(){
    auto chip_id = Sys::Chip::getChipIdCrc();
    switch(chip_id){
        case 3273134334:
            return 3;
        case 341554774:
            return 2;
        case 4079188777:
            return 1;
        case 0x551C4DEA:
            return  3;
        case 0x8E268D66:
            return 1;
        default:
            return 0;
    }
}



void FOCMotor::CanProtocol::parseCanmsg(const CanMsg & msg){
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
            can.write(CanMsg(tx_id, std::make_tuple(__VA_ARGS__)));\
        }\
        break;\
    
    Command command = (Command)(msg.id() & 0x7F);
    const uint16_t tx_id = (((uint16_t)(motor.getNodeId()) << 7) | (uint8_t)(command));

    switch(command){

        SET_VALUE_BIND(Command::SET_TARGET, motor.target)

        SET_METHOD_BIND_ONE(   Command::SET_TRG_VECT,   motor.setTargetVector)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_CURR,   motor.setTargetCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_POS,    motor.setTargetPosition)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_SPD,    motor.setTargetSpeed)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_TPZ,    motor.setTargetTrapezoid)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_TEACH,  motor.setTargetTeach)
        SET_METHOD_BIND_EXECUTE(Command::FREEZE,        motor.freeze)

        SET_METHOD_BIND_ONE(   Command::LOCATE,         motor.locateRelatively)
        SET_METHOD_BIND_ONE(   Command::SET_OPEN_CURR,  motor.setOpenLoopCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_CURR_LMT,   motor.setCurrentLimit)
        SET_METHOD_BIND_TYPE(   Command::SET_POS_LMT,   motor.setPositionLimit, E_2)
        SET_METHOD_BIND_ONE(   Command::SET_SPD_LMT,    motor.setSpeedLimit)
        SET_METHOD_BIND_ONE(   Command::SET_ACC_LMT,    motor.setAccelLimit)

        GET_BIND_VALUE(         Command::GET_POS,       E(motor.getPosition()))
        GET_BIND_VALUE(         Command::GET_SPD,       E(motor.getSpeed()))
        GET_BIND_VALUE(         Command::GET_ACC,       E(0))//TODO
        GET_BIND_VALUE(         Command::GET_CURR,      E(motor.getCurrent()))
        GET_BIND_VALUE(         Command::GET_ALL,       E_4(motor.getCurrent(), motor.getSpeed(), motor.getPosition(), motor.getAccel()))

        SET_METHOD_BIND_EXECUTE(Command::TRG_CALI,      motor.triggerCali)

        SET_METHOD_BIND_EXECUTE(Command::SAVE,          motor.saveArchive)
        SET_METHOD_BIND_EXECUTE(Command::LOAD,          motor.loadArchive)
        SET_METHOD_BIND_EXECUTE(Command::CLEAR,         motor.removeArchive)

        SET_METHOD_BIND_EXECUTE(Command::RST,           motor.reset)
        // GET_BIND_VALUE(         Command::STAT,          motor.(uint8_t)run_status);
        SET_METHOD_BIND_EXECUTE(Command::INACTIVE,      motor.enable, false)
        SET_METHOD_BIND_EXECUTE(Command::ACTIVE,        motor.enable, true)
        SET_METHOD_BIND_EXECUTE(Command::SET_NODEID,    motor.setNodeId, uint8_t(msg))

        default:
            break;
    }

    #undef SET_METHOD_BIND
    #undef SET_VALUE_BIND
    #undef SET_METHOD_BIND_ONE
    #undef SET_VALUE_BIND_REAL
}
