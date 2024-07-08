#pragma once

#include "src/testbench/tb.h"
#include "embd.h"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"




#include "stepper/constants.hpp"
#include "stepper/cli.hpp"


class RemoteStepper{
public:
    // void loadArchive() = 0;
    // virtual void saveArchive() = 0;
    // virtual void removeArchive() = 0;
    // virtual bool autoload() = 0;

    // virtual void setTargetCurrent(const real_t current) = 0;
    // virtual void setTargetSpeed(const real_t speed) = 0;
    // virtual void setTargetPosition(const real_t pos) = 0;
    // virtual void setTagretTrapezoid(const real_t pos) = 0;
    // virtual void setOpenLoopCurrent(const real_t current) = 0;
    // virtual void setTargetVector(const real_t pos) = 0;
    // virtual void setCurrentClamp(const real_t max_current) = 0;
    // virtual void locateRelatively(const real_t pos = 0) = 0;

    // virtual bool isActive() const = 0;
    // virtual const volatile RunStatus & status() = 0;

    // virtual real_t getSpeed() const = 0;
    // virtual real_t getPosition() const = 0;
    // virtual real_t getCurrent() const = 0;

    // virtual void setTargetPositionClamp(const Range & clamp) = 0;
    // virtual void enable(const bool en = true) = 0;
    // virtual void setNodeId(const uint8_t _id) = 0;
    // virtual void setSpeedClamp(const real_t max_spd) = 0;
    // virtual void setAccelClamp(const real_t max_acc) = 0;

    // virtual void triggerCali() = 0;


};


// class EmbdHost:public CliAP{
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
// };





void host_main();