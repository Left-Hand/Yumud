#pragma once

#include "src/testbench/tb.h"
#include "embd.h"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"
#include "remote.hpp"

#include "stepper/constants.hpp"
#include "stepper/cli.hpp"

using StepperUtils::CliAP;

class EmbdHost:public CliAP{
    // void parseCommand(const uint8_t & id, const Command cmd, const CanMsg & msg){
        // logger.setSpace(" ");
        // logger.setEps(10);
        // switch(cmd){
        // case Command::GET_HP:
        //     logger.println("H", id, msg[0], msg[1]);
        //     break;
        // case Command::GET_WEIGHT:
        //     logger.println("W", id, msg[0]);
        //     break;
        // case Command::ATTACK_GET_ID:
        //     attack_id = msg[0];
        //     logger.println("I A", msg[0]);
        //     break;
        // case Command::PEDESTRIAN_GET_ID:
        //     pedestrian_id = msg[0];
        //     logger.println("I C", msg[0]);
        //     break;
        // case Command::DEFENSE_GET_ID:
        //     defense_id = msg[0];
        //     logger.println("I D", msg[0]);
        //     break;
        // default:
        //     break;
        // }
    // }
public:
    EmbdHost(IOStream & _logger, Can & _can):CliAP(_logger, _can){;}

    void parse_command(const uint8_t id, const Command & cmd, const CanMsg & msg);
};





void host_main();