#pragma once

#include <unordered_set>
#include <map>
#include "src/testbench/tb.h"

#include "../drivers/Camera/MT9V034/mt9v034.hpp"
#include "../drivers/IMU/Gyroscope/QMC5883L/qmc5883l.hpp"
#include "../drivers/Display/DisplayerInterface.hpp"
#include "../drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "../drivers/Encoder/ABEncoder.hpp"

#include "../targets/CH32V20X/src/testbench/tb.h"
#include "../hal/bkp/bkp.hpp"

#include "../types/quat/Quat_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"

#include "ctrl.hpp"
#include "body.hpp"
#include "elements.hpp"
#include "fans.hpp"

#include "finder.hpp"
#include "cli.hpp"
#include "config.hpp"


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
    PROCESSING_INPUT,
    PROCESSING_EVENTS,
    PROCESSING_IMG_BEGIN,
    PROCESSING_IMG_END,
    PROCESSING_SEED_BEG,
    PROCESSING_SEED_END,
    PROCESSING_FINDER,
    PROCESSING_FINDER_END,
    PROCESSING_COAST,
    PROCESSING_COAST_END,
    PROCESSING_DP_BEG,
    PROCESSING_DP_END,
    PROCESSING_VEC_BEG,
    PROCESSING_VEC_END,
    PROCESSING_CORNER_BEG,
    PROCESSING_CORNER_END,
    PROCESSING_SEGMENT,
    PROCESSING_SEGMENT_END,
    PROCESSING_FANS,
    PROCESSING_FANS_END,
    END_ROUND
)

using namespace SMC;

class SmartCar:public SmcCli{
public:
    // static constexpr int road_minimal_length = 8;
protected:
    void recordRunStatus(const RunStatus status);
    void printRecordedRunStatus();

    std::tuple<Point, Rangei> get_entry(const ImageReadable<Binary> & src);
    BkpItem & runStatusReg = bkp[1];
    BkpItem & powerOnTimesReg = bkp[2];
    BkpItem & flagReg = bkp[3];
    Gpio & beep_gpio = portB[2];

    Switches switches;
    Flags flags;
    GlobalConfig config;
    SetPoints setp;

    MotorStrength motor_strength;
    Benchmark benchmark;

    HriFanPair hri_fan{vl_fan, vr_fan};
    SideFan left_fan    {timer4.oc(2), timer4.oc(1)};
    SideFan right_fan  {timer5.oc(3), timer5.oc(4)};

    SideFan vl_fan      {timer4.oc(3), timer4.oc(4)};//pair ok
    SideFan vr_fan      {timer5.oc(2), timer5.oc(1)};//pair ok


    RigidBody body{motor_strength, left_fan, right_fan, hri_fan};

    TurnCtrl turn_ctrl;
    SideCtrl side_ctrl;
    VelocityCtrl velocity_ctrl;
    SideVelocityObserver side_velocity_observer;
    CentripetalCtrl centripetal_ctrl;

    DisplayInterfaceSpi SpiInterfaceLcd {{spi2, 0}, portD[7], portB[7]};
    ST7789 tftDisplayer {SpiInterfaceLcd, Vector2i(240, 240)};
    Painter<RGB565> painter = Painter<RGB565>{};

    I2cSw sccb      {portD[2], portC[12]};
    I2cSw i2c     {portB[3], portB[5]};
    
    MT9V034 camera  {sccb};
    ABEncoderTimer  enc     {timer1};
    Odometer        odo     {enc};

    static constexpr real_t full_duty = 0.85;
    static constexpr RGB565 white = 0xffff;
    static constexpr RGB565 black = 0;
    static constexpr RGB565 red = RGB565(31,0,0);
    static constexpr RGB565 green = RGB565(0,63,0);
    static constexpr RGB565 blue = RGB565(0,0,31);

    static constexpr uint ctrl_freq = 50;
    static constexpr real_t inv_ctrl_ferq = 1.0 / ctrl_freq;



    struct Measurer{

    public:
        MPU6050 mpu;
        QMC5883L qml;
        struct{
            Quat accel;
            Vector3 gyro;
            Quat magnet;
        }drift;
        
        struct{
            Vector3 accel;
            Vector3 gyro;
            Vector3 magnet;
        }msm;

    protected:
        void set_drift(const Quat & _accel_drift, const Vector3 & _gyro_drift, const Quat & _magent_drift){
            drift.accel = _accel_drift;
            drift.gyro = _gyro_drift;
            drift.magnet = _magent_drift;
        }

    public:
        Measurer(I2c & i2c):mpu{i2c}, qml{i2c}{;}

        void cali(){
            static constexpr int cali_times = 100;

            Vector3 temp_gravity = Vector3();
            Vector3 temp_gyro_offs = Vector3();
            Vector3 temp_magent = Vector3();
            
            for(int i = 0; i < cali_times; ++i){
                temp_gravity += Vector3(mpu.getAccel());
                temp_gyro_offs += Vector3(mpu.getGyro());    
                temp_magent += Vector3(qml.getMagnet());
                delay(5);
            }

            Vector3 g = temp_gravity / cali_times;
            Vector3 m = temp_magent / cali_times;

            set_drift(
                    Quat(Vector3(0,0,-1),g/g.length()).inverse(),
                    temp_gyro_offs / cali_times,
                    Quat(Vector3(0,0,-1), m/m.length()).inverse()
            );
        }

        void init(){
            mpu.init();
            qml.init();
        }

        void update(){
            mpu.update();
            qml.update();

            msm.accel = drift.accel.xform(Vector3(mpu.getAccel()));
            msm.gyro = (Vector3(mpu.getGyro()) - drift.gyro);
            msm.magnet = drift.magnet.xform(Vector3(qml.getMagnet()));
        }

        auto get_accel() const{
            return msm.accel;
        }

        auto get_gyro() const{
            return msm.gyro;
        }

        auto get_magent() const{
            return msm.magnet;
        }

        auto get_omega() const{
            return msm.gyro.z;
        }

    };

    Measurer measurer{i2c};

    Measurement msm;

    Key start_key   {portE[2], false};
    Key stop_key    {portE[3], false};

    void ctrl();

    void start();
    void stop();

    void reset();
    
    void init_debugger();
    void init_periphs();

    void init_lcd();

    void init_sensor();
    void init_camera();

    void init_it();

    void cali(){measurer.cali();}

    void init_fans();

    void update_sensors();
    void parse();

    void update_beep(const bool);
protected:
    void parse_command(String &, std::vector<String> & args) override;
public:
    void main();
};

void smc_main();