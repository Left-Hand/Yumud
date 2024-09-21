#pragma once

#include "sys/core/system.hpp"

#include "robots/foc/stepper/motor_utils.hpp"

#include "sys/string/string.hpp"

#include <vector>


#define CLI_DEBUG

#ifdef CLI_DEBUG
#undef CLI_DEBUG
#define CLI_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__)
#else
#define CLI_DEBUG(...)
#endif

#ifdef CH32V30X
#define CLI_REPLY(cmd, ...) ch9141.prints(cmd, ##__VA_ARGS__)
#define CLI_PRINTS(...) CLI_DEBUG(__VA_ARGS__) ch91
#elif defined(CH32V203)
#define CLI_REPLY(cmd, ...) logger.prints(cmd, ##__VA_ARGS__)
#define CLI_PRINTS(...) CLI_DEBUG(__VA_ARGS__)
#endif

#define read_value(value)\
{\
    CLI_DEBUG("get", #value, "\t\t is", value);\
    break;\
}

#define trigger_method(method, ...)\
{\
    method(__VA_ARGS__);\
    break;\
}

#define settle_method(method, args, type)\
{\
    if(args.size() == 0){\
        CLI_DEBUG("no arg");\
    }else if(args.size() == 1){\
        method(type(args[0]));\
        CLI_DEBUG("method: ", #method);\
    }\
    break;\
}

#define settle_value(value, args)\
{\
    ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
    if(args.size() == 0){\
        read_value(value);\
    }else if(args.size() == 1){\
        value = decltype(value)(args[0]);\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", args[0]);\
    }\
    break;\
}\

#define settle_positive_value(value, args)\
{\
    auto temp_value = decltype(value)(args[0]);\
    ASSERT_WITH_RETURN((temp_value >= 0), "arg max should be greater than zero");\
    if(args.size() == 0){\
        read_value(value);\
    }else if(args.size() == 1){\
        value = temp_value;\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", value);\
    }\
    break;\
}\


#define settle_clamped_value(value, args, mi, ma)\
{\
    auto temp_value = decltype(value)(args[0]);\
    ASSERT_WITH_RETURN((temp_value >= mi), "arg < ", mi, "\t failed to settle");\
    ASSERT_WITH_RETURN((temp_value < ma), "arg >= ", ma, "\t failed to settle");\
    if(args.size() == 0){\
        read_value(value);\
    }else if(args.size() == 1){\
        value = temp_value;\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", value);\
    }\
    break;\
}\

class FOCMotor;

class AsciiProtocol{
private:
    std::vector<String> split_string(const String& input, char delimiter);
    String temp;
protected:
    FOCMotor & motor;
    IOStream & logger;

    using Command = MotorUtils::Command;
public:
    AsciiProtocol(FOCMotor & _motor, IOStream & _logger):motor(_motor), logger(_logger){;}

    virtual void parseTokens(const String & _command,const std::vector<String> & args);
    void parseLine(const String & _line);
    void readString();
};


