#pragma once

#include "arg_parser.hpp"
#include "sys/string/string.hpp"
#include "sys/debug/debug_inc.h"

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

#define VNAME(x) #x

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
    if(args.size() == 1){\
        CLI_DEBUG("no arg");\
    }else if(args.size() == 2){\
        method(type(args[1]));\
        CLI_DEBUG("method: ", #method);\
    }\
    break;\
}

#define settle_value(value, args)\
{\
    ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
    if(args.size() == 1){\
        read_value(value);\
    }else if(args.size() == 2){\
        value = decltype(value)(args[1]);\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", args[0]);\
    }\
    break;\
}\

#define settle_positive_value(value, args)\
{\
    auto temp_value = decltype(value)(args[1]);\
    ASSERT_WITH_RETURN((temp_value >= 1), "arg max should be greater than zero");\
    if(args.size() == 1){\
        read_value(value);\
    }else if(args.size() == 2){\
        value = temp_value;\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", value);\
    }\
    break;\
}\


#define settle_clamped_value(value, args, mi, ma)\
{\
    auto temp_value = decltype(value)(args[1]);\
    ASSERT_WITH_RETURN((temp_value >= mi), "arg < ", mi, "\t failed to settle");\
    ASSERT_WITH_RETURN((temp_value < ma), "arg >= ", ma, "\t failed to settle");\
    if(args.size() == 1){\
        read_value(value);\
    }else if(args.size() == 2){\
        value = temp_value;\
        CLI_DEBUG("set: ", VNAME(value), "\t\t to", value);\
    }\
    break;\
}\

namespace ymd{

class AsciiProtocolConcept{
protected:
    ArgParser parser;

    InputStream & is;
    OutputStream & os;
    virtual void parseArgs(const Strings & args);
public:
    AsciiProtocolConcept(IOStream & _logger):is(_logger), os(_logger){}
    AsciiProtocolConcept(InputStream & _is, OutputStream & _os):is(_is), os(_os){}

    void update(){
        auto args = parser.update(is);
        if(args.size()){
            parseArgs(args);
            parser.clear();
        }
    }
};

};