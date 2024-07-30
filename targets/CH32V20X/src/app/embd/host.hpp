#pragma once

#include "../sys/core/system.hpp"

#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "../drivers/Wireless/Radio/CH9141/CH9141.hpp"
#include "../drivers/Proximeter/VL53L0X/vl53l0x.hpp"
#include "../drivers/LightSensor/TCS34725/tcs34725.hpp"
#include "../drivers/Camera/MT9V034/mt9v034.hpp"

#include "../nvcv2/mnist/mnist.hpp"

#include "remote.hpp"
#include "imgtrans/img_trans.hpp"

#include "stepper/constants.hpp"
#include "stepper/cli.hpp"


#include "actions/actions.hpp"

#ifdef CH32V30X
using StepperUtils::CliAP;

class EmbdHost:public CliAP{

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

    static constexpr real_t x_scale = 1.0/40;
    static constexpr real_t y_scale = 1.0/40;
    static constexpr real_t z_scale = 1.0/2;
    struct{
        uint8_t bina_threshold = 60;
        uint8_t diff_threshold = 170;
    };

    ActionQueue actions;
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

    void parseCommand(const uint8_t id, const Command & cmd, const CanMsg & msg);
    void parseTokens(const String & _command,const std::vector<String> & args);
    void main();
    void run();
    void reset();
    void cali();

    void line_mm(const Line & _line);
    void point_mm(const Vector2 & _point);
    void z_mm(const real_t & _z);

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
};

#endif


void host_main();