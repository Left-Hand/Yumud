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
        logger.println("get", VNAME(value), "\t\t is", value);\
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
            logger.println("no arg");\
        }else if(args.size() == 1){\
            method(type(args[0]));\
            logger.println("method", #method, "called");\
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
            logger.println("set: ", VNAME(value), "\t\t to", args[0]);\
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
            logger.println("set: ", VNAME(value), "\t\t to", value);\
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
            logger.println("set: ", VNAME(value), "\t\t to", value);\
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

            if (startPos < input.length()) {
                String lastToken = input.substring(startPos);
                result.push_back(lastToken.c_str());
            }

            return result;
        }

        void parse_line(const String & _line){
            if(line.length() == 0) return;
            bool ends = (_line.lastIndexOf('\n') > 0);
            String line = _line;
            line.alphanum();

            static String temp;
            temp += line;

            if(ends){
                if(temp.alphanum()){
                    auto tokens = split_string(temp, ' ');
                    auto command = tokens[0];
                    tokens.erase(tokens.begin());
                    parse_command(command, tokens);
                }
                temp = "";
            }
        }
    protected:
        IOStream & logger;
        Can & can;
        uint8_t node_id;
        using Command = StepperEnums::Command;
    public:
        Cli(IOStream & _logger, Can & _can, const uint8_t _node_id):logger(_logger), can(_can), node_id(_node_id){;}

        virtual void parse_command(const String & _command,const std::vector<String> & args){
            auto command = _command;
            command.toLowerCase();
            switch(hash_impl(command.c_str(), command.length())){
                case "reset"_ha:
                case "rst"_ha:
                case "r"_ha:
                    logger.println("rsting");
                    NVIC_SystemReset();
                    break;
                case "alive"_ha:
                case "a"_ha:
                    logger.println("chip is alive");
                    break;
                default:
                    logger.println("no command available:", command);
                    break;
            }
        }

        virtual void run(){
            read_str();
            read_can();
        }
    protected:
        virtual void read_can() = 0;
    private:
        void read_str(){
            if(logger.available()){
                static String temp_str;
                while(logger.available()){
                    char chr;
                    logger.read(chr);
                    
                    if(chr == '\n'){
                        temp_str.alphanum();

                        if(temp_str.length()) parse_line(temp_str);

                        temp_str = "";
                    }else{
                        temp_str.concat(chr);
                    }
                }
            }
        }
    };

    class CliSTA : public Cli{
    private:
        void read_can() override;
    public:
        CliSTA(IOStream & _logger, Can & _can, const uint8_t _node_id):Cli(_logger, _can, _node_id){;}
        virtual void parse_command(const Command command, const CanMsg & msg){
            switch(command){
                case Command::RST:
                    Sys::Misc::reset();
                    break;
                default:
                    break;
            }
        }

        using Cli::parse_command;
    };

    class CliAP : public Cli{
    private:
        void read_can() override{
            if(can.available()){
                const CanMsg & msg = can.read();
                uint8_t id = msg.id() & 0b1111;
                Command cmd = (Command)(msg.id() >> 4);
                parse_command(id, cmd, msg);
            }
        }
    public:
        CliAP(IOStream & _logger, Can & _can):Cli(_logger, _can, 0x0f){;}
        virtual void parse_command(const uint8_t id, const Command & cmd, const CanMsg & msg) = 0;
        using Cli::parse_command;
    };

}

#endif