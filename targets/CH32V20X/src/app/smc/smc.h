#pragma once

#include <unordered_set>
#include <map>
#include "src/testbench/tb.h"

#include "../drivers/Camera/MT9V034/mt9v034.hpp"
#include "../drivers/IMU/Gyroscope/QMC5883L/qmc5883l.hpp"
#include "../drivers/Display/DisplayerInterface.hpp"
#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "../targets/CH32V20X/src/testbench/tb.h"
#include "../hal/bkp/bkp.hpp"

#include "../types/quat/Quat_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"

#include "body.hpp"
#include "elements.hpp"
#include "fans.hpp"

#include "finder.hpp"
#include "cli.hpp"


struct Key{
protected:
    GpioConcept & m_gpio;
    const bool m_valid_level = false;
public:
    Key(GpioConcept & gpio, const bool valid_level):m_gpio(gpio), m_valid_level(valid_level){;}

    void init(){
        init(m_valid_level);
    }
    void init(const bool valid_level){
        if(valid_level == true){
            m_gpio.inpd();
        }else{
            m_gpio.inpu();
        }
    }

    operator bool()const{
        return m_gpio.read() == m_valid_level;
    }

    auto & io(){
        return m_gpio;
    }
};



BETTER_ENUM(RunStatus, uint8_t,
    NEW_ROUND = 0,
    PROCESSING_CLI,
    PROCESSING_SENSORS,
    PROCESSING_EVENTS,
    PROCESSING_IMG_BEGIN,
    PROCESSING_IMG_END,
    PROCESSING_FINDER,
    PROCESSING_FINDER_END,
    PROCESSING_COAST,
    PROCESSING_COAST_END,
    PROCESSING_DP_END,
    PROCESSING_DIR_END,
    PROCESSING_VA_END,
    PROCESSING_SEGMENT,
    PROCESSING_SEGMENT_END,
    PROCESSING_FANS,
    PROCESSING_FANS_END,
    END_ROUND
)

using namespace SMC;

class SmartCar:public SmcCli{
protected:
    static void recordRunStatus(const RunStatus & status);

    static void printRecordedRunStatus();

    SideFan left_fan    {timer4.oc(2), timer4.oc(1)};
    SideFan right_fan  {timer5.oc(3), timer5.oc(4)};

    SideFan vl_fan      {timer4.oc(3), timer4.oc(4)};//pair ok
    SideFan vr_fan      {timer5.oc(2), timer5.oc(1)};//pair ok

    ChassisFan chassis_left_fan {timer8.oc(1)};
    ChassisFan chassis_right_fan{timer8.oc(2)};

    HriFanPair hri_fan{vl_fan, vr_fan};
    ChassisFanPair chassis_fan{chassis_left_fan, chassis_right_fan};
    RigidBody body{motor_strength, left_fan, right_fan, hri_fan, chassis_fan};

    SpiDrv SpiDrvLcd = SpiDrv{spi2, 0};
    DisplayInterfaceSpi SpiInterfaceLcd {SpiDrvLcd, portD[7], portB[7]};
    ST7789 tftDisplayer {SpiInterfaceLcd, Vector2i(240, 240)};
    Painter<RGB565> painter = Painter<RGB565>{};
    I2cSw sccb      {portD[2], portC[12]};
    I2cSw i2csw     {portB[3], portB[5]};
    MT9V034 camera  {sccb};

    Quat accel_offs;
    Vector3 gyro_offs;
    Quat magent_offs;


    Vector3 accel;
    Vector3 gyro;
    Vector3 magent;
    real_t current_dir;

    Vector2i seed_pos;
    Rangei road_window;

    static constexpr RGB565 white = 0xffff;
    static constexpr RGB565 black = 0;
    static constexpr RGB565 red = RGB565(31,0,0);
    static constexpr RGB565 green = RGB565(0,63,0);
    static constexpr RGB565 blue = RGB565(0,0,31);

    real_t delta = real_t(0);
    real_t fps = real_t(0);

    MPU6050 mpu{i2csw};

    QMC5883L qml{i2csw};


    Key start_key   {portE[2], true};
    Key stop_key    {portE[3], true};


    void ctrl();
    
    void init_debugger();
    void init_periphs();

    void init_lcd();

    void init_sensor();
    void init_camera();

    void init_it();
public:
    void main();
};

void smc_main();