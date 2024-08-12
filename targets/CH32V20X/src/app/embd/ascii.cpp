#include "host.hpp"
#include "robots/foc/stepper/protocol/ascii_protocol.hpp"

static constexpr real_t squ_len = 96;
static constexpr Vector2 pos_begin = {111, 46};
static constexpr Vector2 pos_end = pos_begin + Vector2{squ_len,squ_len};
static constexpr Vector2 pos_center = Vector2(pos_begin) + Vector2(squ_len / 2, squ_len / 2);
static constexpr Vector2 pos_pending = Vector2(pos_center) - Vector2(80, 0);
static constexpr real_t pi_4 = real_t(PI/4);
static constexpr real_t pi_2 = real_t(PI/2);

static Vector2 get_predefined_positions(uint8_t index){
    // xymm 110.5 30
    // xymm 208.5 126
    index = index - 1;
    const auto x_i = 1 - (real_t(index % 3) / 2);
    const auto y_i = real_t(index / 3) / 2;

    return Vector2(pos_begin.x + (pos_end.x - pos_begin.x) * x_i, pos_begin.y + (pos_end.y - pos_begin.y) * y_i);
}


static Vector2 get_predefined_positions(real_t rad){
    auto rad_90 = fmod(rad + pi_4, pi_2) - pi_4;
    auto distance = (squ_len / 2) / cos(rad_90);
    return pos_center + Vector2{-distance, 0}.rotated(rad);
}

void EmbdHost::parseTokens(const String & _command,const std::vector<String> & args){
    auto command = _command;
    command.toLowerCase();
    switch(hash_impl(command.c_str(), command.length())){
        case "exp"_ha:
            settle_method(camera.setExposureValue, args, int)
        case "bina"_ha:
            settle_value(bina_threshold, args)
        case "diff"_ha:
            settle_value(diff_threshold, args)
        case "xymm"_ha:
            if(args.size() == 2){
                steppers.soft_mm(Vector2(args[0], args[1]));
            }
            break;
        case "zmm"_ha:
            if(args.size() == 1){
                steppers.z_mm(real_t(args[0]));
            }
        case "xyz"_ha:
            if(args.size() == 3){
                steppers.x.setTargetPosition(real_t(args[0]));
                steppers.y.setTargetPosition(real_t(args[1]));
                steppers.z.setTargetPosition(real_t(args[2]));
            }
            break;

        case "abort"_ha:
            actions.abort();
            break;

        case "clear"_ha:
            actions.clear();
            break;
        
        case "xyzt"_ha:
            if(args.size() == 3){
                steppers.x.setTargetTrapezoid(real_t(args[0]));
                steppers.y.setTargetTrapezoid(real_t(args[1]));
                steppers.z.setTargetTrapezoid(real_t(args[2]));
            }
            break;

        case "xy"_ha:
            if(args.size() == 2){
                steppers.x.setTargetPosition(real_t(args[0]));
                steppers.y.setTargetPosition(real_t(args[1]));
            }
            break;

        case "xyt"_ha:
            if(args.size() == 2){
                steppers.x.setTargetTrapezoid(real_t(args[0]));
                steppers.y.setTargetTrapezoid(real_t(args[1]));
            }
            break;
    
        case "dz"_ha:
            if(args.size() == 1){
                stepper_z.setTargetSpeed(real_t(args[0]));
            }

        case "dxy"_ha:
            if(args.size() == 2){
                steppers.x.setTargetSpeed(real_t(args[0]));
                steppers.y.setTargetSpeed(real_t(args[1]));
                DEBUG_PRINTLN(args[0], args[1]);
            }
            break;

        case "nz"_ha:
            if(args.size() == 1){
                steppers.y.setNozzle(int(args[0]));
            }
            break;

        case "dxyz"_ha:
            if(args.size() == 3){
                steppers.x.setTargetSpeed(real_t(args[0]));
                steppers.y.setTargetSpeed(real_t(args[1]));
                steppers.z.setTargetSpeed(real_t(args[2]));
            }
            break;
    
        case "gon"_ha:
            trigger_method(steppers.soft_mm, get_predefined_positions(args.size() ? int(args[0]) : 0));
        case "goa"_ha:
            trigger_method(steppers.soft_mm, get_predefined_positions(args.size() ? real_t(args[0]) : 0));
        case "gbn"_ha:
            trigger_method(steppers.do_move, pos_pending, get_predefined_positions(num_result));
        case "gba"_ha:
            trigger_method(steppers.do_move, pos_pending, get_predefined_positions(april_result));
        case "xp"_ha:
            settle_method(steppers.x.setTargetPosition, args, real_t)
        case "yp"_ha:
            settle_method(steppers.y.setTargetPosition, args, real_t);
        case "zp"_ha:
            settle_method(steppers.z.setTargetPosition, args, real_t);
        case "wp"_ha:
            settle_method(steppers.w.setTargetPosition, args, real_t);
        case "xc"_ha:
            settle_method(steppers.x.setTargetCurrent, args, real_t);
        case "yc"_ha:
            settle_method(steppers.y.setTargetCurrent, args, real_t);
        case "zc"_ha:
            settle_method(steppers.z.setTargetCurrent, args, real_t);
        case "wc"_ha:
            settle_method(steppers.w.setTargetCurrent, args, real_t);  
        case "xs"_ha:
            settle_method(steppers.x.setTargetSpeed, args, real_t);
        case "ys"_ha:
            settle_method(steppers.y.setTargetSpeed, args, real_t);
        case "zs"_ha:
            settle_method(steppers.z.setTargetSpeed, args, real_t);
        case "ws"_ha:
            settle_method(steppers.w.setTargetSpeed, args, real_t);
        case "xt"_ha:
            settle_method(steppers.x.setTargetTrapezoid, args, real_t);
        case "yt"_ha:
            settle_method(steppers.y.setTargetTrapezoid, args, real_t);
        case "zt"_ha:
            settle_method(steppers.z.setTargetTrapezoid, args, real_t);
        case "wt"_ha:
            settle_method(steppers.w.setTargetTrapezoid, args, real_t);
        case "xh"_ha:
            settle_method(steppers.x.locateRelatively, args, real_t);
        case "yh"_ha:
            settle_method(steppers.y.locateRelatively, args, real_t);
        case "zh"_ha:
            settle_method(steppers.z.locateRelatively, args, real_t);
        case "xm"_ha:
            settle_method(steppers.x.setCurrentLimit, args, real_t);
        case "ym"_ha:
            settle_method(steppers.y.setCurrentLimit, args, real_t);
        case "zm"_ha:
            settle_method(steppers.z.setCurrentLimit, args, real_t);
        case "cali"_ha:
            trigger_method(steppers.w.triggerCali);
        case "ld"_ha:
            trigger_method(steppers.w.loadArchive, true);
        case "rm"_ha:
            trigger_method(steppers.w.removeArchive, true);
        case "sv"_ha:
            trigger_method(steppers.w.saveArchive, true);
        case "rst"_ha:
            trigger_method(resetAll);
        case "nne"_ha:
            trigger_method(set_demo_method, ActMethod::NONE);
        case "hui"_ha:
            trigger_method(set_demo_method, ActMethod::HUI);
        case "lisa"_ha:
            trigger_method(set_demo_method, ActMethod::LISA);
        case "grab"_ha:
            trigger_method(set_demo_method, ActMethod::GRAB);
        case "inte"_ha:
            trigger_method(set_demo_method, ActMethod::INTER);
        case "rep"_ha:
            trigger_method(set_demo_method, ActMethod::REP);
        case "usbon"_ha:
            trigger_method(trans.enable, true);
        case "usboff"_ha:
            trigger_method(trans.enable, false);
        case "frz"_ha:
            trigger_method(stepper_w.freeze);
        case "inspect"_ha:
            trigger_method(steppers.do_inspect);
        case "center"_ha:
            trigger_method(steppers.soft_mm, pos_center
            );

        case "teach"_ha:{
            const bool sw = args.size() ? bool(args[0].toInt()) : true;
            if(sw){trigger_method(steppers.entry_teach)}
            else{trigger_method(steppers.exit_teach)}
        }

        case "replay"_ha:{
            const bool sw = args.size() ? bool(args[0].toInt()) : true;
            if(sw) {trigger_method(steppers.entry_replay)}
            else {trigger_method(steppers.exit_replay)}
        }

        case "look"_ha:{
            trigger_method(steppers.look);
        }

        case "busy"_ha:
            DEBUG_PRINTLN(actions.pending());
            break;
            
        case "idle"_ha:
            if(args.size() == 2){
                steppers.do_idle({args[0], args[1]});
            }else if(args.size() == 0){
                steppers.do_idle();
            }
            break;

        case "move"_ha:
            if(args.size() == 4){
                steppers.do_move({args[0], args[1]}, {args[2], args[3]});
            }
            break;

        case "drop"_ha:
            if(args.size() == 2){
                steppers.do_place({args[0], args[1]});
            }else if(args.size() == 0){
                steppers.do_place({steppers.x_axis.readMM(), steppers.y_axis.readMM()});
            }
            break;

        case "pick"_ha:
            if(args.size() == 2){
                steppers.do_pick({args[0], args[1]});
            }
            break;
        default:
            CliAP::parseTokens(_command, args);
            break;
    }
}