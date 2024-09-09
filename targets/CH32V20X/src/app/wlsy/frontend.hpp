#ifndef __FORNTEND_HPP__

#define __FORNTEND_HPP__

#include "wlsy_inc.hpp"
#include "backend.hpp"
#include "scenes.hpp"
#include "cli.hpp"

namespace WLSY{
class FrontModule:public Cli{
public:
    BackModule & bm;
    MainScene & rts;

    int targ_power = 0;
    Range_t<uint16_t> power_range = {0, 40};
public:
    FrontModule(BackModule & _bm, MainScene & _rts):Cli(), bm(_bm), rts(_rts){;}

    void init(){

    }

    void run() override{
        Cli::run();
        rts.run();
    }

    void parse_command(const String & _command, const std::vector<String> & args) override{
        auto command = _command;
        command.toLowerCase();
        switch(hash_impl(command.c_str(), command.length())){
            case "watt"_ha:
            case "w"_ha:
                {
                    switch(args.size()){
                        case 0:
                            DEBUG_PRINTLN(bm.getInputWatt());
                            break;
                        case 1:
                            bm.setInputWatt(real_t(args[0]));
                            break;
                    }
                }
                break;
            case "start"_ha:
                bm.startIntergrator();
                break;
            case "stop"_ha:
                bm.stopIntergrator();
                break;
            case "main"_ha:
                rts.changeScene(SubSceneIndex::MAIN);
                break;
            case "input"_ha:
                rts.changeScene(SubSceneIndex::INPUT);
                break;
            case "peel"_ha:
                bm.peel();
                break;
            case "output"_ha:
                rts.changeScene(SubSceneIndex::OUTPUT);
                break;
            case "result"_ha:
                rts.changeScene(SubSceneIndex::EXAM);
                break;
            case "powerup"_ha:
                targ_power = power_range.clamp(targ_power + 5);
                bm.setInputWatt(targ_power);
                break;
            case "powerdown"_ha:
                targ_power = power_range.clamp(targ_power - 5);
                bm.setInputWatt(targ_power);
                break;
            case "temp"_ha:{
                auto info = bm.getInputModuleInfos();
                DEBUG_PRINTS(info.tmp_h, info.tmp_l);
                break;
            }
            default:
                Cli::parse_command(command, args);
                break;
        }
    }

};

}


#endif // !__FORNTEND_HPP__