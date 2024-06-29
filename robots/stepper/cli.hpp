#ifndef __STEPPER_CLI_HPP__

#define __STEPPER_CLI_HPP__

#include "constants.hpp"
#include "statled.hpp"

namespace StepperUtils{

    #define VNAME(x) #x

    #define read_value(value)\
    {\
        DEBUG_PRINT("get", VNAME(value), "\t\t is", value);\
    }

    #define settle_value(value, args)\
    {\
        ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
        if(args.size() == 0){\
            read_value(value);\
        }else if(args.size() == 1){\
            value = decltype(value)(args[0]);\
            DEBUG_PRINT("set: ", VNAME(value), "\t\t to", args[0]);\
        }\
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
            DEBUG_PRINT("set: ", VNAME(value), "\t\t to", value);\
        }\
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
            DEBUG_PRINT("set: ", VNAME(value), "\t\t to", value);\
        }\
    }

    class Cli{
    protected:
        bool cali_debug_enabled = true;
        bool command_debug_enabled = false;
        bool run_debug_enabled = false;

        #define CALI_DEBUG(...)\
        if(cali_debug_enabled){\
        logger.println(__VA_ARGS__);};

        #define COMMAND_DEBUG(...)\
        if(command_debug_enabled){\
        logger.println(__VA_ARGS__);};

        #define RUN_DEBUG(...)\
        if(run_debug_enabled){\
        logger.println(__VA_ARGS__);};
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

        void parse_line(const String & line){
            if(line.length() == 0) return;
            auto tokens = split_string(line, ' ');
            auto command = tokens[0];
            tokens.erase(tokens.begin());
            parse_command(command, tokens);
        }
    public:
        Cli() = default;

        virtual void parse_command(const String & _command,const std::vector<String> & args){
            auto command = _command;
            command.toLowerCase();
            switch(hash_impl(command.c_str(), command.length())){
                case "reset"_ha:
                case "rst"_ha:
                case "r"_ha:
                    DEBUG_PRINT("rsting");
                    NVIC_SystemReset();
                    break;
                case "alive"_ha:
                case "a"_ha:
                    DEBUG_PRINT("chip is alive");
                    break;
                default:
                    DEBUG_PRINT("no command available:", command);
                    break;
            }
        }

        virtual void run(){
            if(DEBUGGER.available()){
                static String temp_str;
                while(DEBUGGER.available()){
                    char chr;
                    DEBUGGER.read(chr);
                    
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

    class Module{

    };
}

#endif