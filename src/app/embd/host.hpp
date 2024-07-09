#pragma once

#include "src/testbench/tb.h"
#include "embd.h"

#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "drivers/LightSensor/TCS34725/tcs34725.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "nvcv2/mnist/mnist.hpp"

#include "remote.hpp"
#include "imgtrans/img_trans.hpp"

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
    RemoteStepper stepper_w;
    RemoteStepper stepper_x;
    RemoteStepper stepper_y;
    RemoteStepper stepper_z;
    RemoteSteppers steppers;
    I2cSw       i2c{portD[2], portC[12]};
    MT9V034     camera{i2c};
    VL53L0X     vl{i2c};
    TCS34725    tcs{i2c};
    CH9141      ch9141{uart7, portC[1], portD[3]};
    Transmitter trans{usbfs};


    struct{
        uint8_t bina_threshold = 200;
        uint8_t diff_threshold = 200;
        // uint8_t 
    };
public:
    EmbdHost(IOStream & _logger, Can & _can):
            CliAP(_logger, _can),
            stepper_w{_logger, _can, 0}, 
            stepper_x{_logger, _can, 1},
            stepper_y{_logger, _can, 2},
            stepper_z{_logger, _can, 3},
            steppers(
                stepper_w,
                stepper_x,
                stepper_y,
                stepper_z
            ){;}

    void parse_command(const uint8_t id, const Command & cmd, const CanMsg & msg);
    void parse_command(const String & _command,const std::vector<String> & args);
    void main();
    void run() override;
};





void host_main();