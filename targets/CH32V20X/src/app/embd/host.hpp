#pragma once

#include "../sys/core/system.hpp"

#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "../drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "../drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "../drivers/LightSensor/TCS34725/tcs34725.hpp"
#include "../drivers/Camera/MT9V034/mt9v034.hpp"

#include "hal/bus/usb/usbfs/usbfs.hpp"

#include "imgtrans/img_trans.hpp"
#include "../nvcv2/mnist/mnist.hpp"

#include "robots/foc/remote/remote.hpp"
#include "drivers/CommonIO/Key/Key.hpp"
#include "machine/machine.hpp"

#ifdef CH32V30X
using MotorUtils::CliAP;
using namespace GpioUtils;

class EmbdHost:public CliAP{
    using NodeId = MotorUtils::NodeId;
    Trajectory trajectory;

    RemoteFOCMotor stepper_w;
    RemoteFOCMotor stepper_x;
    RemoteFOCMotor stepper_y;
    RemoteFOCMotor stepper_z;

    ActionQueue actions;
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

    Gpio & run_led = portC[14];
    Gpio & busy_led = portC[15];
    Gpio & empty_led = portC[13];
    Key toggle_key {portA[0], HIGH};

    uint num_result = 1;
    uint april_result = 0;
public:
    EmbdHost(IOStream & _logger, Can & _can):
            CliAP(_logger, _can),
            stepper_w{_logger, _can, 0}, 
            stepper_x{_logger, _can, 1},
            stepper_y{_logger, _can, 2},
            stepper_z{_logger, _can, 3},

            steppers(
                actions,
                trajectory,
                stepper_w,
                stepper_x,
                stepper_y,
                stepper_z
            ){;}

    void parseCommand(const NodeId id, const Command cmd, const CanMsg & msg);
    void parseTokens(const String & _command,const std::vector<String> & args);
    void main();
    void resetSlave();
    void resetAll();
    void cali();


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