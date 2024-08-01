#ifndef __STEPPER_CLI_HPP__

#define __STEPPER_CLI_HPP__

#include "../sys/core/system.hpp"
#include "constants.hpp"
#include "statled.hpp"

#include "../hal/bus/can/can.hpp"

namespace StepperUtils{

    #define VNAME(x) #x

    #define read_value(value)\
    {\
        DEBUG_PRINTS("get", VNAME(value), "\t\t is", value);\
        break;\
    }

    #define trigger_method(method, ...)\
    {\
        method(__VA_ARGS__);\
        break;\
    }

    #define settle_method(method, args, type)\
    {\
        ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
        if(args.size() == 0){\
            DEBUG_PRINTS("no arg");\
        }else if(args.size() == 1){\
            method(type(args[0]));\
            DEBUG_PRINTS("method", #method, "called");\
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
            DEBUG_PRINTS("set: ", VNAME(value), "\t\t to", args[0]);\
        }\
        break;\
    }

    #define settle_positive_value(value, args)\
    {\
        ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
        auto temp_value = decltype(value)(args[0]);\
        ASSERT_WITH_RETURN((temp_value >= 0), "arg max should be greater than zero");\
        if(args.size() == 0){\
            read_value(value);\
        }else if(args.size() == 1){\
            value = temp_value;\
            DEBUG_PRINTS("set: ", VNAME(value), "\t\t to", value);\
        }\
        break;\
    }

    #define settle_clamped_value(value, args, mi, ma)\
    {\
        ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
        auto temp_value = decltype(value)(args[0]);\
        ASSERT_WITH_RETURN((temp_value >= mi), "arg < ", mi, "\t failed to settle");\
        ASSERT_WITH_RETURN((temp_value < ma), "arg >= ", ma, "\t failed to settle");\
        if(args.size() == 0){\
            read_value(value);\
        }else if(args.size() == 1){\
            value = temp_value;\
            DEBUG_PRINTS("set: ", VNAME(value), "\t\t to", value);\
        }\
        break;\
    }

    class Cli{
    private:
        std::vector<String> split_string(const String& input, char delimiter) {
            std::vector<String> result;

            int startPos = 0;
            int endPos = input.indexOf(delimiter, startPos);

            while (endPos != -1) {
                if(not(endPos - startPos <= 1 and input[startPos] == delimiter)){
                    String token = input.substring(startPos, endPos);
                    result.push_back(token.c_str());
                }
                startPos = endPos + 1;
                endPos = input.indexOf(delimiter, startPos);
            }

            if (startPos < (int)input.length()) {
                String lastToken = input.substring(startPos);
                result.push_back(lastToken.c_str());
            }

            return result;
        }


    protected:
        IOStream & logger;
        Can & can;
        uint8_t node_id;
        using Command = StepperEnums::Command;
    public:
        Cli(IOStream & _logger, Can & _can, const uint8_t _node_id):logger(_logger), can(_can), node_id(_node_id){;}

        virtual void parseTokens(const String & _command,const std::vector<String> & args){
            auto command = _command;
            command.toLowerCase();
            // DEBUG_PRINTLN("command is:", command);
            switch(hash_impl(command.c_str(), command.length())){
                case "reset"_ha:
                case "rst"_ha:
                case "r"_ha:
                    DEBUG_PRINTS("rsting");
                    NVIC_SystemReset();
                    break;
                case "alive"_ha:
                case "a"_ha:
                    DEBUG_PRINTS("chip is alive");
                    break;
                default:
                    DEBUG_PRINTS("no command available:", command);
                    break;
            }
        }

        void parseLine(const String & _line);
        virtual void readCan() = 0;
    };

    class CliSTA : public Cli{
    public:
        void readCan() override{
            if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.id() >> 7;
            Command cmd = (Command)(msg.id() & 0x7F);
            if(id == 0 || id == node_id){
                parseCommand(cmd, msg);
            }
    }
        }
        CliSTA(IOStream & _logger, Can & _can, const uint8_t _node_id):Cli(_logger, _can, _node_id){;}
        virtual void parseCommand(const Command command, const CanMsg & msg){
            switch(command){
                case Command::RST:
                    Sys::Misc::reset();
                    break;
                default:
                    break;
            }
        }
    };

    class CliAP : public Cli{
    public:
        void readCan() override{
            if(can.available()){
                const CanMsg & msg = can.read();
                uint8_t id = msg.id() & 0b1111;
                Command cmd = (Command)(msg.id() >> 4);
                parseCommand(id, cmd, msg);
            }
        }
        CliAP(IOStream & _logger, Can & _can):Cli(_logger, _can, 0x0f){;}
        virtual void parseCommand(const uint8_t id, const Command & cmd, const CanMsg & msg) = 0;
    };

}

#endif