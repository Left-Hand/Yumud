#ifndef __CLI_HPP__

#define __CLI_HPP__

#include "debug.h"
#include "paraments.hpp"
#include "finder.hpp"

namespace SMC{
namespace CLI{
    using namespace NVCV2;

    #define VNAME(x) #x
    
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

    // template<typename T>
    // void read_value(T & value){
    #define read_value(value)\
    {\
        DEBUG_PRINT("get", VNAME(value), "\t\t is", value);\
    }

    // template<typename T>
    // void settle_value(T & value, const std::vector<String> & args)
    
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

    void parse_command(String command, std::vector<String> args){
        using NVCV2::Geometry::affine_config;
        using NVCV2::Geometry::perspective_config;
        command.toLowerCase();
        switch(hash_impl(command.c_str(), command.length())){
            case "affine"_ha:
                if(args.size() == 0){
                    read_value(affine_config.a);
                    read_value(affine_config.b);
                    read_value(affine_config.c);
                    read_value(affine_config.d);
                    read_value(affine_config.e);
                    read_value(affine_config.q);
                }else if(args.size() <= 2){
                    String vname = args.front();
                    args.erase(args.begin());

                    switch(hash_impl(vname.c_str(), vname.length())){
                        case "a"_ha:
                            settle_value(affine_config.a,args);
                            break;
                        case "b"_ha:
                            settle_value(affine_config.b,args);
                            break;
                        case "c"_ha:
                            settle_value(affine_config.c,args);
                            break;
                        case "d"_ha:
                            settle_value(affine_config.d,args);
                            break;
                        case "e"_ha:
                            settle_value(affine_config.e,args);
                            break;
                        case "q"_ha:
                            settle_value(affine_config.q,args);
                            break;
                        default:
                            break;
                    }
                }else{
                    DEBUG_PRINT("affine: invalid syntax");
                }
                break;
            
                case "perspective"_ha:
                case "pers"_ha:
                if(args.size() == 0){
                    read_value(perspective_config.H1);
                    read_value(perspective_config.H2);
                    read_value(perspective_config.H3);
                    read_value(perspective_config.H5);
                    read_value(perspective_config.H6);                    
                    read_value(perspective_config.H8);
                }else if(args.size() <= 2){
                    String vname = args.front();
                    args.erase(args.begin());

                    switch(hash_impl(vname.c_str(), vname.length())){
                        case "h1"_ha:
                            settle_value(perspective_config.H1,args);
                            break;
                        case "h2"_ha:
                            settle_value(perspective_config.H2,args);
                            break;
                        case "h3"_ha:
                            settle_value(perspective_config.H3,args);
                            break;
                        case "h5"_ha:
                            settle_value(perspective_config.H5,args);
                            break;
                        case "h6"_ha:
                            settle_value(perspective_config.H6,args);
                            break;
                        case "h8"_ha:
                            settle_value(perspective_config.H8,args);
                            break;
                        default:
                            break;
                    }
                }else{
                    DEBUG_PRINT("affine: invalid syntax");
                }
                break;
            case "turn"_ha:
            case "t"_ha:
            case "tc"_ha:
                if(args.size() == 0){
                    read_value(turn_ctrl.kp);
                    read_value(turn_ctrl.ki);
                    read_value(turn_ctrl.kd);
                    read_value(turn_ctrl.intergal_clamp);

                }else if(args.size() <= 2){
                    String vname = args.front();
                    args.erase(args.begin());

                    switch(hash_impl(vname.c_str(), vname.length())){
                        case "kp"_ha:
                        case "p"_ha:
                            settle_value(turn_ctrl.kp,args);
                            break;
                        case "ki"_ha:
                        case "i"_ha:
                            settle_value(turn_ctrl.ki,args);
                            break;
                        case "kd"_ha:
                        case "d"_ha:
                            settle_value(turn_ctrl.kd,args);
                            break;
                        case "cl"_ha:
                        case "c"_ha:
                            settle_value(turn_ctrl.intergal_clamp,args);
                            break;
                        default:
                            break;
                    }
                }else{
                    DEBUG_PRINT("turnctrl: invalid syntax");
                }
                break;
            case "s"_ha:
            case "side"_ha:
            case "sc"_ha:
                if(args.size() == 0){
                    read_value(side_ctrl.kp);
                    read_value(side_ctrl.ki);
                    read_value(side_ctrl.kd);
                    read_value(side_ctrl.intergal_clamp);

                }else if(args.size() <= 2){
                    String vname = args.front();
                    args.erase(args.begin());

                    switch(hash_impl(vname.c_str(), vname.length())){
                        case "kp"_ha:
                        case "p"_ha:
                            settle_value(side_ctrl.kp,args);
                            break;
                        case "ki"_ha:
                        case "i"_ha:
                            settle_value(side_ctrl.ki,args);
                            break;
                        case "kd"_ha:
                        case "d"_ha:
                            settle_value(side_ctrl.kd,args);
                            break;
                        case "cl"_ha:
                        case "c"_ha:
                            settle_value(side_ctrl.intergal_clamp,args);
                            break;
                        default:
                            break;
                    }
                }else{
                    DEBUG_PRINT("turnctrl: invalid syntax");
                }
                break;
            case "motor"_ha:
            case "ms"_ha:
                if(args.size() == 0){
                    read_value(motor_strength.left);
                    read_value(motor_strength.right);
                    read_value(motor_strength.hri);
                    read_value(motor_strength.chassis);
                }else if(args.size() <= 2){
                    String vname = args.front();
                    args.erase(args.begin());

                    switch(hash_impl(vname.c_str(), vname.length())){
                        case "l"_ha:
                        case "left"_ha:
                            settle_value(motor_strength.left,args);
                            break;
                        case "r"_ha:
                        case "right"_ha:
                            settle_value(motor_strength.right,args);
                            break;
                        case "c"_ha:
                        case "chassis"_ha:
                            settle_value(motor_strength.chassis,args);
                            break;
                        case "hri"_ha:
                        case "h"_ha:
                            settle_value(motor_strength.hri,args);
                            break;
                    }
                }else{
                    DEBUG_PRINT("affine: invalid syntax");
                }
                break;
            case "ss"_ha:
            case "show"_ha:
                settle_value(show_status, args);
                break;
            case "dpv"_ha:
            case "dough"_ha:
                settle_clamped_value(dpv, args, 0.0, 4.0);
                break;
            case "sh"_ha:
            case "sfheight"_ha:
                settle_clamped_value(safety_seed_height, args, 0, 20);
                break;
            case "asw"_ha:
            case "awidth"_ha:
                settle_clamped_value(align_space_width,args, 0, 20);
                break;
            case "fm"_ha:
                read_value(frame_ms);
                break;
            case "pt"_ha:
                settle_value(positive_threshold,args);
                break;
            case "et"_ha:
                settle_value(edge_threshold,args);
                break;
            case "help"_ha:
            case "h"_ha:
                DEBUG_PRINT("no help available");
                break;
            case "rst"_ha:
            case "r"_ha:
                NVIC_SystemReset();
                break;
            case "en"_ha:
            case "e"_ha:
                enable_flag = true;
                DEBUG_PRINT("enabled");
                break;
            default:
                DEBUG_PRINT("no command available");
                break;
        }
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

    void parse_line(const String & line){
        if(line.length() == 0) return;
        auto tokens = split_string(line, ' ');
        auto command = tokens[0];
        tokens.erase(tokens.begin());
        parse_command(command, tokens);
    }

    void run(){
        if(DEBUGGER.available()){
            static String temp_str;
            char chr = DEBUGGER.read();
            if(chr == '\n'){
                temp_str.trim();
                if(temp_str.length()) parse_line(temp_str);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }

        if(LOGGER.available()){
            static String temp_str;
            char chr = LOGGER.read();
            if(chr == '\n'){
                temp_str.trim();
                if(temp_str.length()) parse_line(temp_str);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
            // DEBUGGER.println(LOGGER.read());
        }
    }
};

};


#endif