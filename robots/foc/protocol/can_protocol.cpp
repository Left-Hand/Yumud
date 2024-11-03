#include "robots/foc/focmotor.hpp"
#include "robots/foc/misc/motor_enums.hpp"
// #include "hal/bus/can/can.hpp"

using namespace yumud;
using namespace yumud::foc;
using namespace yumud::foc::MotorUtils;

void FOCMotor::CanProtocol::parseCanmsg(const CanMsg & msg){
    #define EXECUTER_BIND(cmd, method, ...)\
    case cmd:\
        method(__VA_ARGS__);\
        break;\

    #define VERIFIED_BIND(cmd, method, x, ...)\
    case cmd:\
        if(static_cast<decltype(x)>(msg) == x) method(__VA_ARGS__);\
        break;\

    #define SETTER_BIND_TYPE(cmd, method, type)\
    case cmd:\
        method(type(msg));\
        break;\
    
    #define SETTER_BIND_VALUE(cmd, value)\
    case cmd:\
        value = (decltype(value)(msg));\
        break;\

    #define SETTER_BIND_ONE(cmd, method) SETTER_BIND_TYPE(cmd, method, E)

    #define GETTER_BIND(cmd, ...)\
    case cmd:\
        if(msg.isRemote()){\
            can.write(CanMsg(tx_id, std::make_tuple(__VA_ARGS__)));\
        }\
        break;\
    
    Command command = (Command)(msg.id() & 0x7F);
    const uint16_t tx_id = (((uint16_t)(motor.getNodeId()) << 7) | (uint8_t)(command));

    switch(command){
        SETTER_BIND_ONE(   Command::SET_TRG_VECT,   motor.setTargetVector)
        SETTER_BIND_ONE(   Command::SET_TRG_CURR,   motor.setTargetCurrent)
        SETTER_BIND_ONE(   Command::SET_TRG_POS,    motor.setTargetPosition)
        SETTER_BIND_ONE(   Command::SET_TRG_DELTA,    motor.setTargetPosition)
        SETTER_BIND_ONE(   Command::SET_TRG_SPD,    motor.setTargetSpeed)
        SETTER_BIND_ONE(   Command::SET_TRG_TEACH,  motor.setTargetTeach)
        EXECUTER_BIND(Command::FREEZE,        motor.freeze)

        SETTER_BIND_ONE(   Command::LOCATE,         motor.locateRelatively)
        SETTER_BIND_ONE(   Command::SET_OPEN_CURR,  motor.setOpenLoopCurrent)
        SETTER_BIND_ONE(   Command::SET_CURR_LMT,   motor.setCurrentLimit)
        SETTER_BIND_TYPE(   Command::SET_POS_LMT,   motor.setPositionLimit, E_2)
        SETTER_BIND_ONE(   Command::SET_SPD_LMT,    motor.setSpeedLimit)
        SETTER_BIND_ONE(   Command::SET_ACC_LMT,    motor.setAccelLimit)

        GETTER_BIND(         Command::GET_POS,       E(motor.getPosition()))
        GETTER_BIND(         Command::GET_SPD,       E(motor.getSpeed()))
        GETTER_BIND(         Command::GET_ACC,       E(0))//TODO
        GETTER_BIND(         Command::GET_CURR,      E(motor.getCurrent()))
        GETTER_BIND(         Command::GET_ALL,       E_4(motor.getCurrent(), motor.getSpeed(), motor.getPosition(), motor.getAccel()))

        EXECUTER_BIND(Command::TRIG_CALI,      motor.triggerCali)

        EXECUTER_BIND(Command::SAVE,          motor.saveArchive)
        EXECUTER_BIND(Command::LOAD,          motor.loadArchive)
        EXECUTER_BIND(Command::CLEAR,         motor.removeArchive)

        // EXECUTER_BIND(Command::RST,           motor.reset)
        VERIFIED_BIND(Command::RST,           motor.reset, uint8_t(0xff))
        EXECUTER_BIND(Command::INACTIVE,      motor.enable, false)
        EXECUTER_BIND(Command::ACTIVE,        motor.enable, true)
        EXECUTER_BIND(Command::SET_NODEID,    motor.setNodeId, uint8_t(msg))

        default:
            break;
    }

    #undef SETTER_BIND
    #undef SETTER_BIND_VALUE
    #undef SETTER_BIND_ONE
    #undef SETTER_BIND_VALUE_REAL
}
