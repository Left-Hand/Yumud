#ifndef __CLI_HPP__

#define __CLI_HPP__

#include "finder.hpp"

namespace SMC{
using namespace NVCV2;
constexpr uint32_t hash_impl(char const * str , size_t size){
    uint32_t hash = 5381;

    for (size_t i = 0; i < size; i++) {
        hash = ((hash << 5) + hash) ^ str[i]; /* hash * 33 + c */
    }

    return hash;
}

constexpr uint32_t operator "" _ha(char const* p, size_t size)  {
    return hash_impl(p, size);
}

class SmcCli{

    #define VNAME(x) #x
    


    #define read_value(value)\
    {\
        DEBUG_PRINTLN("get", VNAME(value), "\t\t is", value);\
    }

    
    #define settle_value(value, args)\
    {\
        ASSERT_WITH_RETURN(bool(args.size() <= 1), "invalid syntax");\
        if(args.size() == 0){\
            read_value(value);\
        }else if(args.size() == 1){\
            value = decltype(value)(args[0]);\
            DEBUG_PRINTLN("set: ", VNAME(value), "\t\t to", args[0]);\
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
            DEBUG_PRINTLN("set: ", VNAME(value), "\t\t to", value);\
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
            DEBUG_PRINTLN("set: ", VNAME(value), "\t\t to", value);\
        }\
    }
    

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

protected:
    virtual void parse_command(String & command, std::vector<String> & args) = 0;
    void parse_line(const String & line){
        if(line.length() == 0) return;
        auto tokens = split_string(line, ' ');
        auto command = tokens[0];
        tokens.erase(tokens.begin());
        parse_command(command, tokens);
    }
public:
    void run(){
        // if(DEBUGGER.available()){
        //     static String temp_str;
        //     temp_str.reserve(DEBUGGER.available() + 2);
        //     DEBUGGER.println("ava", DEBUGGER.available());
        //     while(DEBUGGER.available()){
        //         char chr;
        //         DEBUGGER.read(chr);
        //         DEBUGGER.println('c', int(chr));
                
        //         if(chr == '\n'){
        //             temp_str.alphanum();
        //             DEBUGGER << "you input:" << temp_str;
        //             if(temp_str.length()) parse_line(temp_str);

        //             temp_str = "";
        //         }else{
        //             temp_str.concat(chr);
        //         }
        //     }
        //     DEBUGGER.println("temp", temp_str, DEBUGGER.available());
        // }

        // auto & logger = DEBUGGER;
        // if(logger.available()){
        //     delay(1);
        //     auto str = logger.readString();
        //     DEBUGGER << "you input:" << str;
        //     parse_line(str);
        // }
        // {
        //     static auto last_millis = millis();
        //     if(millis() - last_millis > 20){
        //         auto & debugger = DEBUGGER;
        //         // auto ava = debugger.available();
        //         if(debugger.available()){
        //             delay(1);
        //             auto str = debugger.readString();
        //             str.alphanum();
        //             debugger.println(str);
        //         }
        //         last_millis = millis();
        //     }
        // }
        // auto & logger = DEBUGGER;
        // if(logger.available()){
        //     static String temp_str;
        //     while(logger.available()){
        //         char chr;
        //         logger.read(chr);
                
        //         if(chr == '\n'){
        //             temp_str.alphanum();

        //             if(temp_str.length()) parse_line(temp_str);

        //             temp_str = "";
        //         }else{
        //             temp_str.concat(chr);
        //         }
        //     }
        // }
    }
};

};


#endif