#pragma once

#include <unordered_set>
#include <map>
#include "src/testbench/tb.h"

#include "../drivers/Camera/MT9V034/mt9v034.hpp"
#include "../drivers/IMU/Gyroscope/HMC5883L/hmc5883l.hpp"
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

#include "smc_debug.h"

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


namespace SMC{

static constexpr real_t full_duty = 0.85;
static constexpr RGB565 white = 0xffff;
static constexpr RGB565 black = 0;
static constexpr RGB565 red = RGB565(31,0,0);
static constexpr RGB565 green = RGB565(0,63,0);
static constexpr RGB565 blue = RGB565(0,0,31);

static constexpr uint ctrl_freq = 50;
static constexpr real_t inv_ctrl_ferq = 1.0 / ctrl_freq;

static constexpr uint window_y = 32;
static constexpr Vector2i window_half_size = {20, 20};
static constexpr real_t startup_meters = 0.6;

class SmartCar;

struct ElementLocker{
    real_t remain_time = 0;
    real_t remain_travel = 0;
};

struct ElementHolder{
protected:
    SmartCar & owner;
    bool invoked = false;
    
    ElementType next_element_type = ElementType::NONE;
    uint8_t next_element_status = 0;
    LR next_element_side = LR::LEFT;

    AlignMode next_align_mode = AlignMode::LEFT;

    ElementLocker m_locker;

    real_t unlock_t = 0;
    real_t unlock_travel = 0;

    void invoke();
public:

    ElementHolder(SmartCar & _owner);

    void update();

    void reset();

    void request(const ElementType new_element_type, const uint8_t new_element_status, const LR new_element_side, const AlignMode, const ElementLocker & locker);

    bool is_locked() const;
};


struct Measurer{

public:
    MPU6050 mpu;
    HMC5883L qml;
    ABEncoderTimer  enc     {timer1};
    Odometer        odo     {enc};



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

    real_t travel;
    real_t dir_error;

    Rangei road_window;

    Vector2i seed_pos;
protected:
    void set_drift(const Quat & _accel_drift, const Vector3 & _gyro_drift, const Quat & _magent_drift){
        drift.accel = _accel_drift;
        drift.gyro = _gyro_drift;
        drift.magnet = _magent_drift;
    }

    real_t now_spd;
    Rangei ccd_range;
    real_t dir;

    void update_gesture(){
        mpu.update();
        qml.update();

        msm.accel = drift.accel.xform(Vector3(mpu.getAccel()));
        msm.gyro = (Vector3(mpu.getGyro()) - drift.gyro);
        msm.magnet = drift.magnet.xform(Vector3(qml.getMagnet()));

        real_t delta_t = t - last_t;
        last_t = t;
        angle = angle + (get_omega() * delta_t);
    }

    void update_front_speed(){
        static constexpr real_t wheel_l = 0.182;
        odo.update();

        travel = odo.getPosition() * wheel_l;
        static real_t last_travel = travel;
        
        real_t pos_delta = travel - last_travel;
        last_travel = travel;

        now_spd = pos_delta * ctrl_freq;

    }

    real_t angle;
    real_t last_t;
public:

    void reset_angle(){
        angle = 0;
        last_t = t;
    }

    real_t get_angle() const {
        return angle;
    }
    
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

        enc.init();
        odo.inverse(true);
    }

    void update(){
        update_gesture();
        update_front_speed();
    }

    void update_ccd(const Rangei & _ccd_range){
        ccd_range = _ccd_range;
    }

    void update_dir(const real_t & _dir){
        dir = _dir;
    }

    auto get_dir(){
        if(travel < startup_meters) return real_t(PI/2);
        return dir;
    }

    auto get_front_speed() const {
        return now_spd;
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

    real_t get_omega() const{
        return msm.gyro.z;
    }

    auto get_view(const Image<Binary, Binary> & src){
        auto window_center = Vector2i(seed_pos.x, window_y);
        return src.clone(Rect2i::from_center(window_center, window_half_size));
    }


    real_t get_lane_offset(const AlignMode align_mode, const real_t padding_meters = 0.12) const{
        if(travel < startup_meters) return 0;
        //ccd 部分的比例和透视部分的比例不一样 将就用
        static constexpr real_t k = 200;

        auto conv = [&](const int pixel) -> real_t{
            return -(1/k) * (pixel - 94);
        };

        int padding_pixels = int(k * padding_meters);

        switch(align_mode){
            case AlignMode::LEFT:
                return conv(ccd_range.from + padding_pixels);
            case AlignMode::RIGHT:
                return conv(ccd_range.to - padding_pixels);
            case AlignMode::BOTH:
                return conv(ccd_range.get_center());
            default:
                return 0;
        }
    }

    auto get_road_length_meters() const {
        return WorldUtils::distance(road_window.length());
    }

    auto get_travel() const {
        return travel;
    }

};


struct DetectResult{
    bool detected;
    LR side = LR::LEFT;

    DetectResult(const bool _detected, const LR _side = LR::LEFT):
        detected(_detected), side(_side){;}    

    operator bool() const {
        return detected;
    }        
};


class SmartCar:public SmcCli{
protected:
    void printRecordedRunStatus();

    std::tuple<Point, Rangei> get_entry(const ImageReadable<Binary> & src);

    BkpItem & powerOnTimesReg = bkp[2];
    BkpItem & flagReg = bkp[3];

    Gpio & beep_gpio = portB[2];


    Flags flags;
    GlobalConfig config;
    SetPoints setp;

    MotorStrength motor_strength;
    Benchmark benchmark;
    RingConfig ring_config;

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
    I2cSw i2c       {portB[3], portB[5]};
    
    MT9V034 camera  {sccb};

    ElementHolder element_holder{*this};

    bool started = false;
    bool is_blind = false;

    Key start_key   {portE[3], false};
    Key stop_key    {portE[2], false};

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

    void update_holder();

    bool is_startup() const {
        return measurer.get_travel() < startup_meters;
    }
protected:
    void parse_command(String &, std::vector<String> & args) override;
public:
    DetectResult update_detect(const DetectResult & result){
        return result;
    }


    void sw_element(const ElementType element_type, const auto element_status, const LR element_side, const AlignMode align_mode, const ElementLocker & locker = {0,0}){
        element_holder.request(element_type, (uint8_t)element_status, element_side, align_mode, locker);
    };

    bool is_locked()const{
        return element_holder.is_locked();
    }

    // void sw_align(const AlignMode align_mode){
    //     if(align_mode != switches.align_mode){
    //         switches.align_mode = align_mode;
    //         DEBUG_PRINTLN("sw ali", align_mode);
    //     }
    // }

    Switches switches;
    Measurer measurer{i2c};
    void main();
};

};

void smc_main();