#include "smc.h"




void SmartCar::parse_command(String & command,std::vector<String> &args){
    using NVCV2::Geometry::perspective_config;
    command.toLowerCase();
    command.alphanum();

    if(command.length() == 0) return;
    DEBUG_PRINTS("command is: \t", command);
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
            read_value(turn_ctrl.kp);
            read_value(turn_ctrl.kd);
            break;
        case "tc.p"_ha:
            settle_value(turn_ctrl.kp,args);
            break;
        case "tc.d"_ha:
            settle_value(turn_ctrl.kd,args);
            break;

        case "s"_ha:
        case "side"_ha:
        case "sc"_ha:
            read_value(side_ctrl.kp);
            read_value(side_ctrl.ki);
            read_value(side_ctrl.kd);
            read_value(side_ctrl.intergal_clamp);
            break;
        case "sc.kp"_ha:
        case "sc.p"_ha:
            settle_value(side_ctrl.kp,args);
            break;
        case "sc.ki"_ha:
        case "sc.i"_ha:
            settle_value(side_ctrl.ki,args);
            break;
        case "sc.kd"_ha:
        case "sc.d"_ha:
            settle_value(side_ctrl.kd,args);
            break;
    
        case "vc.kp"_ha:
        case "vc.p"_ha:
            settle_value(velocity_ctrl.kp,args);
            break;
        case "vc.kd"_ha:
        case "vc.d"_ha:
            settle_value(velocity_ctrl.kd,args);
            break;
    
        case "sc.cl"_ha:
        case "sc.c"_ha:
            settle_value(side_ctrl.intergal_clamp,args);
            break;

        case "ts"_ha:
            settle_value(target_speed,args);
            break;

        case "hm"_ha:
            DEBUG_PRINTLN("handmode on");
            switches.hand_mode = true;
            body.enable();
            motor_strength.reset();
            break;
        case "am"_ha:
            DEBUG_PRINTLN("automode on");
            switches.hand_mode = false;
            body.enable();
            motor_strength.reset();
            break;
            
        case "start"_ha:
            start();
            break;
        
        case "stop"_ha:
            stop();
            break;

        case "motor"_ha:
        case "ms"_ha:
            read_value(motor_strength.left);
            read_value(motor_strength.right);
            read_value(motor_strength.hri);
            read_value(motor_strength.chassis);
            break;

        case "ms.l"_ha:
        case "left"_ha:
            settle_value(motor_strength.left,args);
            break;
        case "ms.r"_ha:
        case "right"_ha:
            settle_value(motor_strength.right,args);
            break;
        case "ms.c"_ha:
        case "chassis"_ha:
        case "cha"_ha:
            settle_value(motor_strength.chassis,args);
            break;
        case "ms.h"_ha:
        case "hri"_ha:
            settle_value(motor_strength.hri,args);
            break;

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

        case "pde"_ha:
            settle_value(switches.plot_de, args);
            break;

        case "bench"_ha:
        case "bm"_ha:
            read_value(benchmark.cap)
            read_value(benchmark.pers)
            read_value(benchmark.gray)
            read_value(benchmark.bina)
            read_value(benchmark.frame_ms);

            DEBUG_PRINTS("fps is:", 1000/benchmark.frame_ms);
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
        case "acc"_ha:
            DEBUG_PRINTS("acc", mpu.getAccel());
            break;
        case "gyro"_ha:
            DEBUG_PRINTS("gyro", mpu.getGyro());
            break;
        case "mag"_ha:
            DEBUG_PRINTS("mag:", qml.getMagnet());
            break;
        case "st"_ha:
        case "stat"_ha:
            DEBUG_PRINTS("stats:", RunStatus::_from_integral_nothrow(int(runStatusReg)));
            break;

        case "en"_ha:
        case "e"_ha:
            flags.enable_trig= true;
            break;


        case "de"_ha:
        case "d"_ha:
            flags.disable_trig = true;
            break;

        case "res"_ha:
            reset();
            DEBUG_PRINTLN("cleared");
            break;

        case "alive"_ha:
        case "a"_ha:
            DEBUG_PRINTLN("aliving");
            break;

        case "flag"_ha:
            DEBUG_PRINTLN("flags: ", toString(int(flags.data),2));
            break;

        case "int"_ha:
            if(args.size()){
                camera.setExposureValue(int(args[0]));
                DEBUG_PRINTLN("set exposure value", args[0]);
            }
            break;
        default:
            DEBUG_PRINTLN("no command available");
            break;
    }
}