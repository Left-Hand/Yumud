#include "smc.h"

void SmartCar::parse_command(String & command,std::vector<String> &args){
    using NVCV2::Geometry::perspective_config;
    command.toLowerCase();
    command.alphanum();
    DEBUGGER.prints("command is: ", command);
    switch(hash_impl(command.c_str(), command.length())){
        
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
                DEBUG_PRINTLN("affine: invalid syntax");
            }
            break;
        case "turn"_ha:
        case "t"_ha:
        case "tc"_ha:
            if(args.size() == 0){
                read_value(turn_ctrl.kp);
                read_value(turn_ctrl.kd);

            }else if(args.size() <= 2){
                String vname = args.front();
                args.erase(args.begin());

                switch(hash_impl(vname.c_str(), vname.length())){
                    case "kp"_ha:
                    case "p"_ha:
                        settle_value(turn_ctrl.kp,args);
                        break;
                    case "kd"_ha:
                    case "d"_ha:
                        settle_value(turn_ctrl.kd,args);
                        break;
                    default:
                        break;
                }
            }else{
                DEBUG_PRINTLN("turnctrl: invalid syntax");
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
                DEBUG_PRINTLN("turnctrl: invalid syntax");
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
                DEBUG_PRINTLN("affine: invalid syntax");
            }
            break;
        // case "ss"_ha:
        // case "show"_ha:
        //     settle_value(config.show_status, args);
            // break;
        case "dpv"_ha:
        case "dough"_ha:
            settle_clamped_value(config.dpv, args, 0.0, 4.0);
            break;
        case "sh"_ha:
        case "sfheight"_ha:
            settle_clamped_value(config.safety_seed_height, args, 0, 20);
            break;
        case "asw"_ha:
        case "awidth"_ha:
            settle_clamped_value(config.align_space_width,args, 0, 20);
            break;
        case "fm"_ha:
            read_value(config.frame_ms);
            break;
        case "pt"_ha:
            settle_value(config.positive_threshold,args);
            break;
        case "et"_ha:
            settle_value(config.edge_threshold,args);
            break;
        case "help"_ha:
        case "h"_ha:
            DEBUG_PRINTLN("no help available");
            break;
        case "rst"_ha:
        case "r"_ha:
            NVIC_SystemReset();
            break;
        case "st"_ha:
            DEBUG_PRINTLN()
        case "en"_ha:
        case "e"_ha:
            config.enable_flag = true;
            DEBUG_PRINTLN("enabled");
            break;
        default:
            DEBUG_PRINTLN("no command available");
            break;
    }
}