#pragma once

#include "../sys/core/system.hpp"

#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "../drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "../drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "../drivers/LightSensor/TCS34725/tcs34725.hpp"
#include "../drivers/Camera/MT9V034/mt9v034.hpp"

#include "../nvcv2/mnist/mnist.hpp"

#include "imgtrans/img_trans.hpp"

#include "robots/foc/stepper/constants.hpp"
#include "robots/foc/stepper/cli.hpp"

#include "machine/machine.hpp"
#include "actions/action_queue.hpp"

#ifdef CH32V30X
using StepperUtils::CliAP;

class EmbdHost:public CliAP{
    using NodeId = StepperUtils::NodeId;

    RemoteFOCMotor stepper_w;
    RemoteFOCMotor stepper_x;
    RemoteFOCMotor stepper_y;
    RemoteFOCMotor stepper_z;

    Machine steppers;

    I2cSw       i2c{portD[2], portC[12]};
    MT9V034     camera{i2c};
    VL53L0X     vl{i2c};
    TCS34725    tcs{i2c};
    CH9141      ch9141{uart7, portC[1], portD[3]};
    Transmitter trans{usbfs};

    struct{
        uint8_t bina_threshold = 60;
        uint8_t diff_threshold = 170;
    };

    ActionQueue actions;

    Gpio & run_led = portC[14];
    Gpio & busy_led = portC[15];
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

    void parseCommand(const NodeId id, const Command cmd, const CanMsg & msg);
    void parseTokens(const String & _command,const std::vector<String> & args);
    void main();
    void run();
    void reset();
    void cali();

    void do_move(const Vector2 & from, const Vector2 & to);
    void do_pick(const Vector2 & from);
    void do_drop(const Vector2 & to);

    void do_idle(const Vector2 & to);
    void do_blink(const uint dur);


    enum class ActMethod{
        NONE = 0,
        HUI,
        LISA,
        GRAB,
        INTER,
        REP
    };
    
    ActMethod act_method = ActMethod::NONE;
    void set_demo_method(const ActMethod new_method);
    void act();
    void tick();
};

#endif


void host_main();