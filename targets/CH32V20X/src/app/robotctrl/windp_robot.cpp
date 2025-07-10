#include <atomic>
#include <array>

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/bitflag.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"
#include "hal/gpio/gpio.hpp"

#include "drivers/Encoder/odometer.hpp"
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Storage/EEprom/AT24CXX/at24cxx.hpp"


#include "types/regions/range2/range2.hpp"
#include "types/vectors/vector2/Vector2.hpp"
#include "types/vectors/vector3/Vector3.hpp"
#include "types/vectors/quat/Quat.hpp"

#include "robots/repl/repl_service.hpp"
#include "digipw/prelude/abdq.hpp"

using namespace ymd;

namespace PhysicalConstants {
    static constexpr real_t L_s = 0.098_r;
    static constexpr real_t L = 0.679_r;
    static constexpr real_t g = 9.8_r;
    static constexpr real_t h = 0.038_r;
    static constexpr real_t H = L + h;
    static constexpr real_t W = L + h;
    static constexpr real_t m1 = 0.07_r;
    static constexpr real_t m2 = 0.05_r;
    static constexpr real_t M = m1+m2;
    static constexpr real_t x = (m1 + 2*m2)/(2*m1 + 2*m2)*L;
    static constexpr real_t J = m1*L*L/3 + m2*L*L;
}

static constexpr real_t cali_r(const real_t _x){
    real_t x = CLAMP2(_x, 1.0_r);
    return sign(x)*(pow(abs(x), 1.5_r)+0.022_r);
}


class JetPwm{
public:
    JetPwm(
        hal::TimerOC & pwm_ap,
        hal::TimerOC & pwm_an,
        hal::TimerOC & pwm_bp,
        hal::TimerOC & pwm_bn
    ):
        channel_a_(pwm_ap, pwm_an),
        channel_b_(pwm_bp, pwm_bn)
    {;}

    void init_channels(){
        channel_a_.inverse(EN);
        channel_b_.inverse(DISEN);

        static constexpr hal::TimerOcPwmConfig pwm_noinv_c_rg = {
            .cvr_sync_en = EN,
            .valid_level = HIGH
        };

        static constexpr hal::TimerOcPwmConfig pwm_inv_c_rg = {
            .cvr_sync_en = EN,
            .valid_level = LOW,
        };
        
        channel_a_.pos_channel().init(pwm_noinv_c_rg);
        channel_a_.neg_channel().init(pwm_noinv_c_rg);
        channel_b_.pos_channel().init(pwm_inv_c_rg);
        channel_b_.neg_channel().init(pwm_inv_c_rg);

    }

    void set_duty(const real_t duty_a, const real_t duty_b){
        channel_a_.set_duty(duty_a);
        channel_b_.set_duty(duty_b);
    }
private:

    hal::TimerOcPair channel_a_;
    hal::TimerOcPair channel_b_;
};

class RobotDynamics{

    void move(const Vector2<real_t> dir){
        static real_t left = 0;
        left = (left*0.2_r + CLAMP2(-dir.y,0.19_r)*0.8_r);
        static real_t right = 0;
        right = (right*0.2_r + CLAMP2(dir.x,0.19_r)*0.8_r);
        jetpwm_.set_duty(left,right);
    }
private:
    JetPwm jetpwm_;
};

static constexpr real_t DELTA = 0.01_r;

struct RobotMetaData{
    // Vector3 gravity = Vector3();
    Vector3<real_t> accel;
    Vector3<real_t> gyro;

    Quat<real_t> gravity_of_rs;
    Vector3<real_t> gyro_of_rs;



    // Vector2 targetPoint = Vector2();
    Vector2<real_t> dir;
    real_t rot_bias = 0;
};

struct TaskBase{
    struct Input{
        Vector2<real_t> a;
        Vector2<real_t> w;
        Vector2<real_t> p;
    };

    using Output = Vector2<real_t>;
};

struct TaskCenter:public TaskBase{
    constexpr Output calc(const Input & input){
        return ((-0.017_r*input.a)+(-0.42_r*input.w));
    }
};


struct TaskCircle:public TaskBase{

    constexpr Output calc(const Input & input){
        using namespace PhysicalConstants;
        const auto p = input.p;
        const auto a = input.a; 
        const auto w = input.w;
        const Vector2 vec_norm = p.normalized();
        const Vector2 vec_tan = vec_norm.rotated(real_t(PI*0.5));
        
        const real_t q = atan2(W, H);
        const real_t theta = acos(cos(p.x)*cos(p.y));


        const real_t targ_w = sin(q)*sqrt(g/(x*cos(q)));
        const real_t d_w = targ_w - abs(w.dot(vec_tan));
        
        const Vector2 dir_t = 0.23_r*d_w*vec_tan*sign(w.dot(vec_tan));
        const Vector2 dir_n = 2.2_r*(q-theta - 0.07_r*a.dot(vec_norm))*vec_norm*
            (pow(abs(q-theta),0.8_r));


        return dir_n + dir_t;
    }
};

struct TaskLine:public TaskBase{
    constexpr Output calc(const Input & input, const real_t theta){
        using namespace PhysicalConstants;

        const auto p = input.p;
        const auto a = input.a; 
        const auto w = input.w;

        const Vector2 vec_norm = Vector2(0.0_r, 1.0_r).rotated(theta);
        const Vector2 vec_tan = Vector2(1.0_r, 0.0_r).rotated(theta);
        
        const real_t d1 = input.w.cross(vec_tan);
        const real_t d2 = input.a.cross(vec_tan);
        const Vector2 dir_n = (0.027_r*d2 + 0.37_r*d1)*vec_norm;

        const real_t E_targ = M*g*x*(1.0_r - H / sqrt(H*H + W*W));
        
        const real_t E_p = M*g*x*(1.0_r - cos(p.x)*cos(p.y));
        const real_t E_k = 0.5_r * J * (w.project(vec_tan)).length_squared();
        const real_t E = E_p + E_k;

        Vector2 dir_t = 15_r*(E_targ - E + 0.00037_r*abs(a.dot(vec_tan))) 
            * vec_tan * sign(w.dot(vec_tan));
        return dir_n + dir_t;
    }
};


#if 0

#include <Arduino.h>
#include <SSD1315_128_64.hpp>
#include <array>
#include <SSD1315_72_40.hpp>
#include <SSD1315_128_32.hpp>
#include <Vector2.hpp>
#include <PID.hpp>
#include <math.h>
#include <Basis.hpp>
#include <MPU6050.hpp>
#include <Basis.hpp>
#include <Wire.h>
#include <algorithm>

// #include <MPU6050_6Axis_MotionApps20.h>
// #include <VL53L0X.h>


const uint8_t pin_L_r = D8;
const uint8_t pin_LB = D7;
const uint8_t pin_R_r = D6;
const uint8_t pin_RB = D5;
const uint8_t pin_EN = D0;
const uint8_t pin_LK = D3;

#de_rine EPS 0.00_r
#de_rine XO_rS 0.05_r
#de_rine ZO_rS 0.20_r

MPU6050 mpu;
SSD1315_128_64 oled;



IRAM_ATTR void lock(){
    static bool last_state = false;
    // bool current_state = digitalRead(pin_LK)
    digitalWrite(pin_EN, !last_state);
    last_state = !last_state;
}


void sysCali(){
    using namespace GlobalCon_rig;
    static constexpr uint8_t n = 8;

    Vector3 temp_gravity = Vector3();
    Vector3 temp_gyro_of_rs = Vector3();
    
    for(uint8_t i = 0;i < n;i++){
        temp_gravity += mpu.get_accel();
        temp_gyro_of_rs += mpu.get_gyro();    
        delay(5);
    }
    Vector3 gravity = temp_gravity / (real_t)n;
    gravity_of_rs = Quat(Vector3(0,0,-1),gravity/gravity.length()).inverse();
    gyro_of_rs = temp_gyro_of_rs / (real_t)n;
}

void getPos(){
    using namespace GlobalCon_rig;
    using namespace Measurements;
    
    accel = gravity_of_rs.x_rorm(mpu.get_accel_orignal());
    accel.x = -accel.x;
    Serial.println(accel.x);
    gyro = (mpu.get_gyro_orignal() - gyro_of_rs);

    static uint32_t last_us = 0;
    DELTA = (micros() - last_us)*1E-6;
    last_us = micros();

    w = Vector2(gyro.x, gyro.y);
    static Vector2 last_w = Vector2();
    Vector2 a_temp = (w - last_w)/DELTA;
    last_w = w;

    static constexpr real_t k1 = 0.85;
    static constexpr real_t k2 = 1.0 - k1;

    static Vector2 a_filted = Vector2(0,0);
    a_filted = k1*a_filted + k2*a_temp;
    a = a_filted;

    Vector3 g_fixed = Vector3(a.x, a.y, w.length_squared())*L_s + accel;
    Quat pose = Quat(g_fixed.normalized(), Vector3(0,0,-1));

    Basis pose_basis = pose; 
    Vector3 pos3 = pose_basis.get_euler_xyz();

    p = Vector2(-pos3.y, pos3.x);
}

// void 

void displayOled(){
    using namespace GlobalCon_rig;
    Serial.println("T");
    static uint32_t framen = 0;
    framen ++;

    Frame & frame = *oled.fetch_rrame();
    frame.clear();
    
    frame.drawStringRow(Vector2i(0,0),"x:"+String(accel.x));
    frame.drawStringRow(Vector2i(0,1),"y:"+String(accel.y));
    frame.drawStringRow(Vector2i(0,2),"z:"+String(accel.z));
    frame.drawStringRow(Vector2i(0,3),"l:"+String(accel.length()));

    frame.drawStringRow(Vector2i(64,0),"x:"+String(gyro.x));
    frame.drawStringRow(Vector2i(64,1),"y:"+String(gyro.y));
    frame.drawStringRow(Vector2i(64,2),"z:"+String(gyro.z));
    frame.drawStringRow(Vector2i(64,3),"l:"+String(gyro.length()));

    // frame.drawStringRow(Vector2i(0,4),"x:"+String(pos.x));
    // frame.drawStringRow(Vector2i(0,5),"y:"+String(pos.y));
    // frame.drawStringRow(Vector2i(0,7),"l:"+String(pos.length()));

    frame.drawChar(Vector2i(96, 48), '*');
    static std::vector<Vector2i> tails;
    i_r(framen % 4 == 0)tails.push_back(Vector2i(96-32*p.y, 48-32*p.x));
    i_r(tails.size()>=8) tails.erase(tails.begin());
    for (Vector2i tail:tails){
        i_r (tail!= tails.back()) frame.drawChar(tail,'.');
        else frame.drawChar(tail, '+');
    }

    frame.drawChar(Vector2i(32-32*dir.y, 48-32*dir.x), '+');
    oled.pushStream();
}

void setup() {
    initConn();
    delay(20);
    oled.init();
    mpu.init();
    delay(20);
    // sysCali();

}


void loop() {
    static real_t theta = 0.0_r;
    theta += 0.001_r;
    getPos();
    displayOled();
    // Wire.setClock
    // displayOled();
    // move(Vector2(-0.05,0.05));
    // controlPoint();

    i_r (Serial.available()) {  // 如果串口上有可用数据
        String input = Serial.readStringUntil('\n');  // 读取一行字符，以换行符为分隔符
        uint8_t first_separator = input.indexO_r(' ');  // 找到第一个逗号的位置
        uint8_t second_separator = input.indexO_r(' ', first_separator + 1);  // 找到第二个逗号的位置
        uint8_t cnt = (uint8_t)(first_separator != -1) + (uint8_t)(first_separator != -1);
        
        switch (cnt)
        {
        case 0:
            
            break;
        
        de_rault:
            break;
        }
    }
    
    taskCenter();
    // taskLine(0, 0.15_r);
    // taskCircle(0.18_r);
    // taskCircle();
}

#endif