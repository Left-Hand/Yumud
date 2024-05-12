#include "misc.h"
#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;
#include "src/fwwb_compents/fwwb_inc.h"
#include "dsp/pll.hpp"
#include "src/opa/opa.hpp"

constexpr uint32_t SPI1_BaudRate = (144000000/32);
constexpr uint32_t SPI2_BaudRate = (144000000/8);

#define I2C_BaudRate 400000


#include "src/device/Encoder/Estimmator.hpp"
#include "src/device/Memory/EEPROM/AT24CXX/at24c02.hpp"

// I2cSw i2cSw(i2cScl, i2cSda);


// SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
// SpiDrv spiDrvOled = SpiDrv(spi2, 0);
// SpiDrv spiDrvFlash = SpiDrv(spi1, 0);

// SpiDrv spiDrvRadio = SpiDrv(spi1, 0);
// I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
// I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
// I2cDrv i2cDrvAdc = I2cDrv(i2c1, 0x90);
// I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);
// 
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x36 << 1);
// I2cDrv i2cDrvQm = I2cDrv(i2cSw, 0x1a);
// I2cDrv i2cDrvBm = I2cDrv(i2cSw, 0xec);
// I2cDrv i2cDrvMt = I2cDrv(i2cSw, 0x0C);
// ST7789 tftDisplayer(SpiDrvLcd);
// SSD13XX oledDisPlayer(spiDrvOled);
// MPU6050 mpu(i2cDrvMpu);
// SGM58031 ext_adc(i2cDrvAdc);
// LT8920 radio(spiDrvRadio);
// TCS34725 tcs(i2cDrvTcs);
// VL53L0X vlx(i2cDrvVlx);
// PCF8574 pcf(i2cDrvPcf);
// AS5600 mags(i2cDrvAS);

// W25QXX extern_flash(spiDrvFlash);

// AS5600 mag_sensor(i2cDrvAS);
// QMC5883L earth_sensor(i2cDrvQm);
// BMP280 prs_sensor(i2cDrvBm);
// MT6701 mt_sensor(i2cDrvMt);


extern "C" void TimBase_IRQHandler(void) __interrupt;


namespace Shaders{
__fast_inline RGB565 ShaderP(const Vector2i & pos){
    static int cnt = 0;
    cnt++;
    return pos.x + pos.y + cnt;
}

__fast_inline RGB565 ShaderUV(const Vector2 & UV){
    if((UV - Vector2(0.5, 0.5)).length_squared() <= real_t(0.25)){
        return RGB565::RED;
    }else{
        return RGB565::BLUE;
    }
}

__fast_inline RGB565 Mandelbrot(const Vector2 & UV){
    Complex c(lerp(UV.x, real_t(0.5), real_t(-1.5)), lerp(UV.y, real_t(-1), real_t(1)));
    Complex z;
    uint8_t count = 0;

    while ((z < real_t(4)) && (count < 23))
    {
        z = z*z + c;
        count = count + 1;
    }
    return count * 100;
}
};




using Sys::t;

// static Printer & logger = uart2;













//     // flash.load(temp);
//     while(true) __WFI;
//     // while(true){
//         // logger.println("Hi");
//         // delay(100);
//     // }

// }


// class Pmdc{
// protected:
//     PwmChannel & pwm_f;
//     PwmChannel & pwm_b;
// public:
//     Pmdc(PwmChannel & _pwm_f, PwmChannel & _pwm_b):pwm_f(_pwm_f), pwm_b(_pwm_b){;}


// }

#include "src/device/Encoder/ABEncoder.hpp"
#include "src/device/Encoder/OdometerLines.hpp"
#include "src/adc/adcs/adc1.hpp"

static auto pos_pid = PID_t<real_t>(3.1, 0.5, 1.02, 1.0);
static auto curr_pid = PID_t<real_t>(20.0, 0.0, 0.0, 1.0);
static auto pos2curr_pid = PID_t<real_t>(11.0, 0.05, 1.1, 1.0);
static real_t omega = real_t(0.3);


void pmdc_test(){

    uart2.init(115200 * 8, Uart::Mode::TxRx);
    Printer & logger = uart2;
    logger.setSpace(",");
    logger.setEps(4);

    timer3.init(36000);
    timer3[1].setPolarity(true);
    timer3[2].setPolarity(true);
    // timer3[1].setPolarity(false);
    // timer3[2].setPolarity(false);
    auto pwmL = PwmChannel(timer3[2]);
    auto pwmR = PwmChannel(timer3[1]);
    pwmL.init();
    pwmR.init();

    Coil2 motor = Coil2(pwmL, pwmR);
    motor.init();
    // tim1ch1n
    // Exti
    // auto trigGpioA = portA[1];
    // auto trigGpioB = portA[4];
    ABEncoderExti enc(portA[4], portA[1], NvicPriority(0, 7));
    // enc.init();

    // auto odo = Odometer(enc);
    OdometerLines odo(enc, 1100);
    Estimmator est(odo);
    // constexpr auto a = ((uint32_t)(16384  << 16) / 1000) >> 16;

    // AdcChannelConfig{.}
    adc1.init(
        {
            AdcChannelConfig{.channel = AdcChannels::TEMP, .sample_cycles = AdcSampleCycles::T55_5}
        },
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T55_5}
        });
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    // timer3[4] = 0;
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    TIM3->CH4CVR = TIM3->ATRLR >> 1;
    adc1.enableCont();
    // adc1.enableScan();
    adc1.enableAutoInject();

    // adc1.enableRightAlign(false);
    real_t motor_curr;
    // adc1.start();
    // adc1.swStartRegular();
    // adc1.swStartInjected();

    est.init();


    motor.enable();
    real_t duty;
    // real_t duty(0);
    real_t target_curr;
    real_t motor_curr_temp;
    real_t target_pos;
    LowpassFilter_t<real_t, real_t> lpf(10);
    String temp_str;

    constexpr int closeloop_freq = 1000;
    Gpio & t_watch = portA[5];
    t_watch.OutPP();
    timer4.init(closeloop_freq);
            real_t sense_uni;
    timer4.bindCb(Timer::IT::Update, [&](){
        t_watch.set();
        est.update(t);
        // duty = CLAMP(duty +), -1, 1);
        // motor.setDuty( pos_pid.update(target, odo.getPosition(), est.getSpeed()));
        target_curr = pos2curr_pid.update(target_pos, odo.getPosition(), est.getSpeed());

        u16_to_uni(ADC1->IDATAR1 << 4, sense_uni);
        constexpr float sense_scale = (1000.0 / 680.0) * 3.3;
        motor_curr_temp = sign(duty) * sense_uni * sense_scale;
        // motor_curr = lpf.update(motor_curr_temp, t);4
        motor_curr = lpf.forward(motor_curr_temp, real_t(1.0 / closeloop_freq));

        duty = curr_pid.update(target_curr, motor_curr);
        motor = duty;
        t_watch.clr();
    });

    // timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
    // uint16_t adc_out;
    pos2curr_pid.setClamp(0.2);

    while(true){
        // est.update();
        // if(adc1.isInjectedIdle()) adc1.swStartInjected();
            // adc_out = ADC1->RDATAR;

        // }
        // target = 10 * sin(t / 4);
        // target = 4 * floor(t/3);
        // target_pos = real_t(0.12) * t;
        static real_t ang = real_t(0);
        static real_t last_t = real_t(0);

        // target_pos = 2 * sign(frac(ang += omega * (t - last_t)) - 0.5);
        target_pos = sin(ang +=  omega * (t - last_t));
        last_t = t;
        // motor = 0.7 * sin(t);
        // motor = duty;
        // motor = frac(t);
        // target = real_t(0.1);

        // uart2.println(target, est.getPosition(), est.getSpeed(), motor_curr, duty, lpf.update(motor_curr, t));
        // static auto prog = real_t(0); prog += real_t(0.01);
        // uart2.println(motor_curr, );
        // logger.println(odo.getPosition(),est.getSpeed(), target_pos, motor_curr, target_curr);
        // logger.println(duty, motor_curr_temp, motor_curr, odo.getPosition(), est.getSpeed(), 0);
        // logger.println(target_pos, odo.getPosition(), est.getSpeed(), motor_curr );
        // motor.setDuty(sin(t));
        // logger.println(ADC1->IDATAR1);
        // delay(2);

        logger.println(ADC1->IDATAR1);
        if(logger.available()){
            char chr = logger.read();
            if(chr == '\n'){
                temp_str.trim();
                // logger.println(temp_str);
                // if(temp_str.length()) parseLine(temp_str);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }
        Sys::Clock::reCalculateTime();

    }
}



template<typename T>
struct targAndMeasurePair_t{
    T target;
    T measure;
};

using targAndMeasurePair = targAndMeasurePair_t<real_t>;


real_t warp_to_halfpi(const real_t & x){
    real_t ret = x;
    while(ret > PI / 2) ret -= PI;
    while(ret < -PI / 2) ret += PI;
    return ret;
}


struct buckRuntimeValues{
    targAndMeasurePair curr;
    targAndMeasurePair volt;
};


// class Test:pub
void buck_test(){
    uart2.init(115200 * 8, Uart::Mode::TxRx);
    Printer & logger = uart2;
    logger.setSpace(",");
    logger.setEps(4);


    timer1.init(32000, Timer::TimerMode::Up);


    timer1.initBdtr(16);

    auto & ch = timer1.ch(1);
    auto & chn = timer1.chn(1);

    ch.setIdleState(true);
    // ch.init();
    chn.setIdleState(false);
    chn.init();

    auto buck_pwm = PwmChannel(ch);
    buck_pwm.setClamp(real_t(0.1), real_t(0.9));
    buck_pwm.init();

    adc1.init(
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T239_5}
        },
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T239_5}
            // AdcChannelConfig{.channel = AdcChannels::CH1, .sample_cycles = AdcSampleCycles::T239_5},
        });

    static constexpr int buck_freq = 1000;
    timer3.init(buck_freq);

    real_t adc_fl1, adc_fl2, adc_out, duty;
    buckRuntimeValues buck_rv;
    LowpassFilter_t<real_t, real_t> lpf(50);
    LowpassFilter_t<real_t, real_t> lpf2(30);

    // TIM3->CH4CVR = TIM3->ATRLR >> 1;
    // timer3.bindCb(Timer::IT::CC4, [&](){

    //     duty = real_t(0.3) + 0.14 * sin(4 * TAU * t);
    //     // duty = real_t(0.5);
    //     u16_to_uni(ADC1->IDATAR1<<4, adc_out);
    //     // adc_fl1 = lpf.forward(adc_out, real_t(1.0 / buck_freq));
    //     // adc_fl2 = lpf2.forward(adc_fl1, real_t(1.0 / buck_freq));

    //     // buck_rv.curr.measure = real_t(adc_fl2);
    //     buck_pwm = real_t(0.1);
    // });
    // timer3.enableIt(Timer::IT::CC4, NvicPriority(0, 0));

    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    // adc1.setPga(AdcOnChip::Pga::X1);
    adc1.enableCont();
    adc1.enableAutoInject(); // must be enabled for ext.inj

    opa2.init(1);

    timer1.enable();
    Pll pll;
    while(true){

        // if(adc1.isRegularIdle()){
        //     // adc1.refreshInjectedData();
        //     // adc1.swStartInjected();
        //     adc1.swStartRegular();
        // }

        // if(adc1.isInjectedIdle()){
        //     // adc1.refreshInjectedData();
        //     // adc1.swStartInjected();
        //     adc1.swStartInjected();
        // }
        // u16_to_uni(ADC1->RDATAR << 4, adc_out);


        static real_t last_t = t;
        static real_t angle = real_t(0);
        const real_t omega = 50 + 20 * sin(t);
        real_t delta_t = t - last_t;
        last_t = t;

        angle += delta_t * omega;
        real_t pll_input = sin(angle);
        real_t pll_output =  pll.update(pll_input,real_t(0.04));
        logger.println(pll_input, cos(pll_output), pll.omega, ADC1->IDATAR1);
        delay(1);
        Sys::Clock::reCalculateTime();
    }
}


#include "dsp/oscillator.hpp"
#include "src/device/Dac/TM8211/tm8211.hpp"



void osc_test(){
    uart2.init(115200 * 8, Uart::Mode::TxRx);
    Printer & logger = uart2;
    logger.setSpace(",");
    logger.setEps(4);
    Gpio & i2sSck = portA[1];
    Gpio & i2sSda = portA[0];
    Gpio & i2sWs = portA[4];
    i2sSck.OutPP();
    i2sSda.OutPP();
    i2sWs.OutPP();
    I2sSw i2sSw(i2sSck, i2sSda, i2sWs);
    i2sSw.init(114514);
    // I2sDrv i2sDrvTm = I2sDrv(i2sSw);
    // TM8211 extern_dac(i2sDrvTm);
    // extern_dac.

    // extern_dac.setDistort(0);
    // extern_dac.setRail(real_t(0.3), real_t(3.0));
    while(true){
        // real_t audio_out = sin(40*t);
        // real_t audio_volt = audio_out * 2 + 2.5;
        // extern_dac.setVoltage(frac(t), frac(t));
        // extern_dac.setChData(0, );
        // uint32_t data = 0x8000 + int(0x7fff * sin(400 * t));
        // data |= (data << 16);
        // static uint16_t cnt;
        // i2sSw.write((++cnt) << 16);
        // uint16_t data = int(frac(t) * 0xffff);
        static uint16_t data = 0;
        data+= 80;
        uint32_t data_out=  data<<16;
        i2sSw.write(data_out);

        // {
        //     i2sWs.clr();
        //     delayMicroseconds(10);
        //     for(int i = 16; i > 0; i--)
        //     {
        //         i2sSck.clr();
        //     delayMicroseconds(10);
        //         i2sSda = (data >> i) & 0x01;
        //     delayMicroseconds(10);
        //        // digitalWrite(BCK, HIGH);
        //         i2sSck.set();
        //     delayMicroseconds(10);
        //     }
        //                     i2sSda.clr();
        //     for(int i = 16; i > 0; i--)
        //     {
        //         i2sSck.clr();
        //     delayMicroseconds(10);

        //     // delayMicroseconds(10);
        //         // digitalWrite(BCK, HIGH);
        //         i2sSck.set();
        //     delayMicroseconds(10);
        //     }
        //     i2sWs.set();
        //     delayMicroseconds(10);
        //     i2sWs.clr();
        // }




        logger.println(data_out);
        // delayMicroseconds(100);
        Sys::Clock::reCalculateTime();
    }
}

#include "enum.h"
#include "json.hpp"

#include <bits/stl_numeric.h>
#include <optional>


struct GCODE{

};

struct G_MOVECODE:public GCODE{
    Vector3 pos;
};

struct G_SPINCODE:public G_MOVECODE{
    Vector3 org;
};

struct G0{
    std::optional<real_t> x;
    operator bool() const{
        return x.has_value();
    }
};

class CharOrdedParaments{
protected:
    std::array<real_t, 26> charOrdedParaments;
public:
    auto operator [](const char & _chr){
        if(_chr < 'z' or _chr > 'A') return charOrdedParaments.at(0);
        auto chr = std::toupper(_chr);
        return charOrdedParaments.at(chr - 'A');
    }
};

class GcodeQueue{
    std::array<real_t, 26> charOrdedParaments;


};

class GcodeParser{


    std::pair<char, real_t> fetchNewPair(){
        return {'e', 3.0};
    }
public:
    GcodeParser(){

    }
};

// namespace StepperTest{

// Printer & logger = uart1;

// auto & forwardChannelB = timer1[1];
// auto & backwardChannelB = timer1[2];
// auto & forwardChannelA = timer1[3];
// auto & backwardChannelA = timer1[4];

// PwmChannel forwardPwmA(forwardChannelA);
// PwmChannel forwardPwmB(forwardChannelB);

// PwmChannel backwardPwmA(backwardChannelA);
// PwmChannel backwardPwmB(backwardChannelB);

// auto & verfChannelA = timer3[3];
// auto & verfChannelB = timer3[2];

// PwmChannel pwmCoilB(verfChannelB);
// PwmChannel pwmCoilA(verfChannelA);


// Coil1 coilA(portA[10], portA[11],  verfChannelA);
// Coil1 coilB(portA[8], portA[9],  verfChannelB);

// SVPWM2 svpwm(coilA, coilB);

// SpiDrv mt6816_drv(spi1, 0);
// MT6816 mt6816(mt6816_drv);

// auto odo = OdometerPoles(mt6816);

// real_t target_pos;



// constexpr uint32_t foc_freq = 36000;
// constexpr int poles = 50;
// constexpr uint32_t est_freq = 1800;
// constexpr uint32_t est_devider = foc_freq / est_freq;
// // constexpr float foc_execute_duty = 0.3;
// constexpr float inv_poles = 0.02;
// constexpr float openloop_current_limit = 0.3;


// constexpr uint32_t foc_period_micros = 1000000 / foc_freq;
// uint32_t foc_pulse_micros;


// real_t est_speed = real_t();
// real_t raw_pos = real_t();
// real_t est_pos = real_t();
// real_t est_elecrad = real_t();

// real_t run_current = real_t();
// real_t run_elecrad = real_t();
// real_t run_elecrad_addition = real_t();


// struct SetPoints{

// };

// struct RunTimeValues{

// };

// struct Paraments{

// };


// struct Targets{
//     real_t curr = real_t();
//     real_t speed = real_t();
//     real_t pos = real_t();
//     real_t time = real_t();
// };

// Targets targets;

// struct Setpoints{

// };

// using Range = Range_t<real_t>;

// struct StallObserver{
//     real_t max_position_error;
//     real_t inspect_enable_speed_threshold;
//     uint32_t entry_time_ms;
//     uint32_t max_sustain_time_ms;

//     static constexpr uint32_t no_stall_time_ms = 0;

//     bool update(const real_t & target_position, const real_t & measured_position, const real_t & measured_speed){
//         auto current_time_ms = millis();
//         if(abs(measured_speed) < inspect_enable_speed_threshold){ // stall means low speed
//             if(entry_time_ms == no_stall_time_ms){
//                 if(abs(target_position - measured_position) > max_position_error){ //
//                     entry_time_ms = millis();
//                 }
//             }else{
//                 return (millis() - entry_time_ms> max_sustain_time_ms);
//             }
//         }else{
//             entry_time_ms = no_stall_time_ms;
//         }
//         return false;
//     }
// };

// struct CurrentCtrl{
// public:
//     real_t current_slew_rate = real_t(20.0 / foc_freq);
//     real_t current_output = real_t(0);
//     Range current_range{real_t(-0.4), real_t(0.4)};

//     real_t update(const real_t & current_setpoint){
//         real_t current_delta = CLAMP(current_setpoint - current_output, -current_slew_rate, current_slew_rate);
//         current_output = current_range.clamp(current_output + current_delta);
//         return current_output;
//     }
// };

// struct TorqueCtrl{
//     CurrentCtrl & currCtrl;

// };

// struct SpeedCtrl{
//     real_t kp = real_t(0.03);
//     real_t ki;

//     real_t kp_clamp = real_t(0.1);
//     real_t intergal;
//     real_t intergal_clamp;
//     real_t ki_clamp;



//     real_t current_output;
//     real_t elecrad_offset_output;

//     real_t elecrad_addition;
//     real_t elecrad_addition_clamp = real_t(0.8);


//     bool inited = false;
//     auto update(const real_t & goal_speed,const real_t & measured_speed){
//         if(!inited){
//             inited = true;
//             elecrad_offset_output = real_t(0);
//             elecrad_addition = real_t(0);
//         }

//         real_t error = goal_speed - measured_speed;
//         if(goal_speed < 0) error = - error;
//         // real_t abs_error = abs(error);

//         real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
//         intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
//         real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

//         elecrad_addition = CLAMP(elecrad_addition + kp_contribute + ki_contribute, real_t(0), elecrad_addition_clamp);

//         real_t elecrad_offset = real_t(PI / 2) + elecrad_addition;
//         if(goal_speed < 0) elecrad_offset = - elecrad_offset;

//         // current_output = targets.curr;
//         // elecrad_offset_output = elecrad_offset;

//         return std::make_tuple(current_output, elecrad_offset + odo.getElecRad());
//     }

//     real_t getElecradOffset(){
//         return elecrad_offset_output;
//     }

//     real_t getCurrent(){
//         return current_output;
//     }
// };


// struct PositionCtrl{
//     real_t kp;
//     real_t ki;
//     real_t kd;
//     real_t ks;

//     real_t kp_clamp;
//     real_t intergal;
//     real_t ki_clamp;
//     real_t kd_enable_speed_threshold;//minimal speed for activation kd
//     real_t kd_clamp;
//     real_t ks_enable_speed_threshold; // minimal speed for activation ks
//     real_t ks_clamp;

//     real_t target_position;
//     real_t target_speed;

//     real_t current_slew_rate = real_t(20.0 / foc_freq);
//     real_t current_minimal = real_t(0.05);
//     real_t current_clamp = real_t(0.3);

//     real_t err_to_current_ratio = real_t(20);

//     real_t current_output;
//     real_t elecrad_output;
//     real_t last_error;

//     auto update(const real_t & _target_position, const real_t & measured_position, const real_t & measuresd_speed){
//         target_position = _target_position;
//         real_t error = target_position - measured_position;

//         // real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);

//         // if((error.value ^ last_error.value) & 0x8000){ // clear intergal when reach target
//         //     intergal = real_t(0);0
//         // }else{
//         //     intergal += error;
//         // }

//         // real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

//         // real_t kd_contribute;
//         // if(abs(measuresd_speed) > kd_enable_speed_threshold){ // enable kd only highspeed
//         //     kd_contribute =  CLAMP(- measuresd_speed * kd, -kd_clamp, kd_clamp);
//         // }else{
//         //     kd_contribute = real_t(0);
//         // }

//         // real_t speed_error = target_speed - measuresd_speed;
//         // real_t ks_contribute;
//         // if(abs(speed_error) > ks_enable_speed_threshold){
//         //     ks_contribute = CLAMP(speed_error * ks, -ks_clamp, ks_clamp);
//         // }else{
//         //     ks_contribute = real_t(0);
//         // }

//         // last_error = error;

//         // real_t current_delta = CLAMP(kp_contribute + ki_contribute, -current_slew_rate, current_slew_rate);
//         // current_output = CLAMP(current_output + current_delta, -current_clamp, current_clamp);
//         // // return current_slew_rate;

//         if(true){
//             real_t abs_error = abs(error);
//             current_output = CLAMP(abs_error * err_to_current_ratio, current_minimal, current_clamp);
//             // if(abs(error) < inv_poles * 2){
//             if(false){
//                 // elecrad_offset_output = error * poles * TAU;
//                 elecrad_output = odo.position2rad(target_position);
//                 // elecrad_output = real_t(0);
//             }else{
//                 elecrad_output = est_elecrad + (error > 0 ? 2.5 : - 2.5);
//                 // elecrad_output = real_t(0);
//             }
//         }

//         return std::make_tuple(current_output, elecrad_output);
//     }
// };

// std::vector<String> splitString(const String& input, char delimiter) {
//     std::vector<String> result;

//     int startPos = 0;
//     int endPos = input.indexOf(delimiter, startPos);

//     while (endPos != -1) {
//         String token = input.substring(startPos, endPos);
//         result.push_back(token.c_str());

//         startPos = endPos + 1;
//         endPos = input.indexOf(delimiter, startPos);
//     }

//     if (startPos < input.length()) {
//         String lastToken = input.substring(startPos);
//         result.push_back(lastToken.c_str());
//     }

//     return result;
// }




// void setCurrent(const real_t & _current, const real_t & _elecrad){
//     coilA = cos(_elecrad) * _current;
//     coilB = sin(_elecrad) * _current;
// }

// real_t openloop_elecrad;
// static SpeedCtrl ctrl;
// static CurrentCtrl currCtrl;
// static PositionCtrl posctrl;


// // enum class RunStatus{
// //     INIT,
// //     INACTIVE,
// //     ACTIVE,
// //     BEEP,
// //     CALI,
// //     ERROR,
// //     EXCEPTION
// // };

// BETTER_ENUM(RunStatus, uint8_t,     
//     INIT = 0,
//     INACTIVE,
//     ACTIVE,
//     BEEP,
//     CALI,
//     ERROR,
//     EXCEPTION
// )

// RunStatus run_status = RunStatus::INIT;

// using DoneFlag = bool;
// using InitFlag = bool;

// enum class ErrorCode:uint8_t{
//     OK = 0,
//     COIL_A_DISCONNECTED,
//     COIL_A_NO_SIGNAL,
//     COIL_B_DISCONNECTED,
//     COIL_B_NO_SIGNAL,
//     ODO_DISCONNECTED,
//     ODO_NO_SIGNAL
// };

// ErrorCode error_code = ErrorCode::OK;
// String error_message;

// bool auto_shutdown_activation = true;
// bool auto_shutdown_actived = false;
// uint16_t auto_shutdown_timeout_ms = 200;
// uint16_t auto_shutdown_last_wake_ms = 0;

// bool shutdown_when_error_occurred;
// bool shutdown_when_exception_occurred;

// void shutdown(){
//     coilA.enable(false);
//     coilB.enable(false);
// }

// void wakeup(){
//     coilA.enable(true);
//     coilB.enable(true);
// }

// struct ShutdownFlag{
// protected:
//     bool state = false;
// public:

//     ShutdownFlag() = default;

//     auto & operator = (const bool & _state){
//         state = _state;
//         if(state) shutdown();
//         else wakeup();

//         return *this;
//     }

//     operator bool() const{
//         return state;
//     }
// };


// ShutdownFlag shutdown_flag;

constexpr bool cali_debug_enabled = false;
constexpr bool command_debug_enabled = true;
constexpr bool run_debug_enabled = true;

#define CALI_DEBUG(...)\
if(cali_debug_enabled){\
logger.println(__VA_ARGS__);};

#define COMMAND_DEBUG(...)\
if(command_debug_enabled){\
logger.println(__VA_ARGS__);};

#define RUN_DEBUG(...)\
if(run_debug_enabled){\
logger.println(__VA_ARGS__);};

// void throw_error(const ErrorCode & _error_code,const char * _error_message) {
//     error_message = String(_error_message);
//     run_status = RunStatus::ERROR;
//     if(shutdown_when_error_occurred){
//         shutdown_flag = true;
//     }
// }

// void throw_exception(const ErrorCode & ecode, const char * emessage){
//     error_message = String(emessage);
//     run_status = RunStatus::EXCEPTION;
//     if(shutdown_when_exception_occurred){
//         shutdown_flag = true;
//     }
// }


// void parseCommand(const char & argc, const std::vector<String> & argv){
//     switch(argc){
//         case 'C':
//             if(argv.size() == 0) goto syntax_error;
//             targets.curr = real_t(argv[0]);
//             COMMAND_DEBUG("SetCurrent to ", targets.curr);
        
//             break;
//         case 'E':
//             if(argv.size() == 0) goto syntax_error;
//             run_elecrad_addition = real_t(argv[0]);
//             break;
//         case 'S':
//             if(argv.size() == 0) goto syntax_error;
//             targets.speed = real_t(argv[0]);
//             COMMAND_DEBUG("SetSpeed to ", targets.curr);
//             break;
//         case 'D':
//             if(argv.size() == 0){
//                 COMMAND_DEBUG("ShutdownState: ", bool(shutdown_flag));
//             }else{
//                 COMMAND_DEBUG("Shutdown Command REcved");
//                 shutdown_flag = bool(argv[0]);
//             }
//             break;

//         case 'I':
//             if(argv.size() == 0){
//                 COMMAND_DEBUG("Run state:", run_status._to_string());
//             }
//             // else{
//             //     COMMAND_DEBUG("Run state switch Command Recved");
//             //     if(RunStatus::_is_valid(argv[0].c_str())){
//             //         run_status = RunStatus::_from_string_nocase(argv[0].c_str());
//             //     }
//             //     COMMAND_DEBUG("Run state switched to:", argv[0]);
//             // }
//             break;
//         case 'G':
//             if(argv.size() == 0){
//                 COMMAND_DEBUG("cali_debug_enabled", cali_debug_enabled);
//                 COMMAND_DEBUG("command_debug_enabled", command_debug_enabled);
//                 COMMAND_DEBUG("run_debug_enabled", run_debug_enabled);
//             }
            
//             // else if{argv.size() == 1}
//             break;
//         case 'R':
//             __disable_irq();
//             NVIC_SystemReset();
//         syntax_error:
//             COMMAND_DEBUG("SyntexError", argc);
//             break;
//         default:
//             break;
//     }
// }

// void parseLine(const String & line){
//     if(line.length() == 0) return;
//     auto tokens = splitString(line, ' ');
//     auto argc = tokens[0][0];
//     tokens.erase(tokens.begin());
//     parseCommand(argc, tokens);
// }


// static DoneFlag active_prog(const InitFlag & init_flag = false){

//     auto [curr_out, elecrad_out] = posctrl.update(10 * sin(t), est_pos, est_speed);
    

//     run_current = currCtrl.update(curr_out);
//     // run_current = real_t(0.2);
//     run_elecrad = elecrad_out;
//     // run_elecrad = TAU * frac(t);


//     setCurrent(run_current, run_elecrad + run_elecrad_addition);

//     // uint32_t foc_begin_micros = nanos();
//     odo.update();

//     raw_pos = odo.getPosition();

//     static real_t last_raw_pos;
//     static uint32_t est_cnt;


//     if(init_flag){
//         est_pos = raw_pos;
//         est_speed = real_t();
//         last_raw_pos = raw_pos;
//         est_cnt = 0;

//         setCurrent(real_t(0), real_t(0));
//     }

//     if(auto_shutdown_activation){
//         if(run_current){
//             auto_shutdown_actived = false;
//             wakeup();
//             auto_shutdown_last_wake_ms = millis();
//         }else{
//             if(millis() - auto_shutdown_last_wake_ms > auto_shutdown_timeout_ms){
//                 auto_shutdown_actived = true;
//                 shutdown();
//                 auto_shutdown_last_wake_ms = millis();
                
//             }
//         }
//     }

//     real_t delta_raw_pos = raw_pos - last_raw_pos;
//     last_raw_pos = raw_pos;

//     est_pos = raw_pos + delta_raw_pos;
//     est_elecrad = odo.getElecRad();
//     // Fixed();

//     {//estimate speed and update controller
//         static real_t est_delta_raw_pos_intergal = real_t();

//         est_cnt++;
//         if(est_cnt == est_devider){ // est happens
//             real_t est_speed_new = est_delta_raw_pos_intergal * (int)est_freq;

//             est_speed.value = (est_speed_new.value + est_speed.value * 3) >> 2;

//             est_delta_raw_pos_intergal = real_t();
//             est_cnt = 0;

//             ctrl.update(real_t(targets.speed), est_speed);
//         }else{
//             est_delta_raw_pos_intergal += delta_raw_pos;
//         }
//     }



//     // uint32_t foc_end_micros = nanos();
//     // foc_pulse_micros = foc_end_micros - foc_begin_micros;

//     {
//         // struct SpeedCtrl{
//         //     real_t kp;
//         //     real_t ki;
//         //     real_t kd;
//         //     real_t ks;

//         //     real_t kp_clamp;
//         //     real_t intergal;
//         //     real_t ki_clamp;
//         //     real_t kd_enable_speed_threshold;//minimal speed for activation kd
//         //     real_t kd_clamp;
//         //     real_t ks_enable_speed_threshold; // minimal speed for activation ks
//         //     real_t ks_clamp;
//         // };
//     }

//     return false;
// }

// DoneFlag cali_prog(const InitFlag & init_flag = false){
//     enum class SubState{
//         ALIGN,
//         INIT,
//         PRE_FORWARD,
//         FORWARD,
//         BREAK,
//         REALIGN,
//         PRE_BACKWARD,
//         BACKWARD,
//         PRE_LANDING,
//         LANDING,
//         STOP,
//         ANALYSIS,
//         EXAMINE,
//         DONE
//     };

//     constexpr int forwardpreturns = 15;
//     constexpr int forwardturns = 50;
//     constexpr int backwardpreturns = forwardpreturns;
//     constexpr int backwardturns = forwardturns;

//     constexpr int landingpreturns = 15;
//     constexpr int landingturns = 50;

//     constexpr int subdivide_micros = 256;
//     constexpr int align_ms = 100;
//     constexpr int break_ms = 100;
//     constexpr int stop_ms = 400;

//     constexpr float cali_current = 0.3;
//     constexpr float align_current = 0.4;


//     static SubState sub_state = SubState::DONE;
//     static uint32_t cnt = 0;
//     // static real_t openloop_elecrad_step = real_t(TAU / subdivide_micros);

//     static real_t raw_position_accumulate = real_t(0);
//     static real_t last_raw_lap_position = real_t(0);

//     static real_t landing_position_accumulate = real_t(0);

//     // using real = real_t;

//     static int openloop_pole;

//     static std::array<std::pair<real_t, real_t>, 50> forward_test_data;
//     static std::array<std::pair<real_t, real_t>, 50> backward_test_data;
//     static std::array<real_t, 50> elecrad_test_data;

//     static std::array<real_t, 50> forward_err;
//     static std::array<real_t, 50> backward_err;

//     if(init_flag){
//         sub_state = SubState::ALIGN;
//         cnt = 0;
//         return false;
//     }

//     auto sw_state = [](const SubState & new_state){
//         sub_state = new_state;
//         cnt = 0;
//     };

//     auto accumulate_raw_position = [](const real_t & raw_lap_position){
//         real_t deltaLapPosition = raw_lap_position - last_raw_lap_position;
//         if(deltaLapPosition > real_t(0.5f)){
//             deltaLapPosition -= real_t(1);
//         }else if (deltaLapPosition < real_t(-0.5f)){
//             deltaLapPosition += real_t(1);
//         }

//         raw_position_accumulate += deltaLapPosition;
//         last_raw_lap_position = raw_lap_position;
//     };

//     if(sub_state == SubState::DONE){
//         return true;
//     }

//     else{

//         switch(sub_state){
//             case SubState::ALIGN:
//                 setCurrent(real_t(align_current), real_t(0));
//                 if(cnt >= (int)((foc_freq / 1000) * align_ms)){
//                     sw_state(SubState::INIT);
//                 }
//                 break;
//             case SubState::INIT:
//                 odo.reset();
//                 odo.inverse();
//                 odo.update();

//                 sw_state(SubState::PRE_FORWARD);
//                 break;

//             case SubState::PRE_FORWARD:

//                 setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

//                 if(cnt >= forwardpreturns * subdivide_micros){
//                     odo.update();
//                     raw_position_accumulate = real_t(0);
//                     last_raw_lap_position = odo.getRawLapPosition();
//                     openloop_pole = odo.getRawPole();

//                     sw_state(SubState::FORWARD);
//                 }
//                 break;
//             case SubState::FORWARD:
//                 odo.update();
//                 accumulate_raw_position(odo.getRawLapPosition());

//                 setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

//                 if(cnt % subdivide_micros == 0){
//                     openloop_pole++;

//                     const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
//                     real_t openloop_position = real_t(cnt / subdivide_micros) * real_t(inv_poles);

//                     forward_test_data[cali_index].first = openloop_position;
//                     forward_test_data[cali_index].second = raw_position_accumulate;

//                     forward_err[cali_index] = openloop_position - raw_position_accumulate;
//                 }

//                 if(cnt >= forwardturns * subdivide_micros){
//                     sw_state(SubState::BREAK);
//                 }
//                 break;
//             case SubState::BREAK:
//                 setCurrent(real_t(cali_current), real_t(0));
//                 if(cnt >= (int)((foc_freq / 1000) * break_ms)){
//                     sw_state(SubState::REALIGN);
//                 }
//                 break;
//             case SubState::REALIGN:
//                 setCurrent(real_t(align_current), real_t(0));
//                 if(cnt >= (int)((foc_freq / 1000) * align_ms)){
//                     sw_state(SubState::PRE_BACKWARD);
//                 }
//                 break;
//             case SubState::PRE_BACKWARD:

//                 setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

//                 if(cnt >= backwardpreturns * subdivide_micros){
//                     odo.update();
//                     raw_position_accumulate = real_t(0);
//                     last_raw_lap_position = odo.getRawLapPosition();
//                     openloop_pole = odo.getRawPole();

//                     sw_state(SubState::BACKWARD);
//                 }
//                 break;

//             case SubState::BACKWARD:
//                 odo.update();
//                 accumulate_raw_position(odo.getRawLapPosition());

//                 setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

//                 if(cnt % subdivide_micros == 0){
//                     openloop_pole--;

//                     const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
//                     real_t openloop_position = -real_t(cnt / subdivide_micros) * real_t(inv_poles);

//                     backward_test_data[cali_index].first = openloop_position;
//                     backward_test_data[cali_index].second = raw_position_accumulate;

//                     backward_err[cali_index] = openloop_position - raw_position_accumulate;
//                 }

//                 if(cnt >= backwardturns * subdivide_micros){
//                     real_t forward_mean = std::accumulate(std::begin(forward_err), std::end(forward_err), real_t(0)) / int(forward_err.size());
//                     for(auto & forward_err_item : forward_err){
//                         forward_err_item -= forward_mean;
//                     }

//                     real_t backward_mean = std::accumulate(std::begin(backward_err), std::end(backward_err), real_t(0)) / int(backward_err.size());
//                     for(auto & backward_err_item : backward_err){
//                         backward_err_item -= backward_mean;
//                     }

//                     for(uint8_t i = 0; i < poles; i++){
//                         odo.cali_map[i] = mean(forward_err[i], backward_err[i]);
//                     }

//                     raw_position_accumulate = real_t(0);
//                     sw_state(SubState::PRE_LANDING);
//                 }
//                 break;
            
//             case SubState::PRE_LANDING:
//                 odo.update();

//                 setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
//                 if(cnt >= landingpreturns * subdivide_micros){
//                     landing_position_accumulate = real_t(0);
//                     openloop_pole = 0;
//                     odo.setElecRadOffset(real_t(0));
//                     sw_state(SubState::LANDING);
//                 }
//                 break;
//             case SubState::LANDING:
//                 odo.update();
//                 // accumulate_raw_position(odo.getRawLapPosition());

//                 setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
//                 if(cnt % subdivide_micros == 0){
//                     landing_position_accumulate += raw_position_accumulate;
//                     elecrad_test_data[openloop_pole] = odo.getElecRad();
//                     openloop_pole++;
//                 }

//                 if(cnt >= landingturns * subdivide_micros){
//                     sw_state(SubState::STOP);
//                 }
//             case SubState::STOP:
//                 odo.update();

//                 setCurrent(real_t(align_current), real_t(0));
//                 if(cnt >= (int)((foc_freq / 1000) * stop_ms)){
//                     sw_state(SubState::ANALYSIS);
//                 }

                
//                 break;
//             case SubState::ANALYSIS:
//                 // odo.runCaliAnalysis();
//                 // for(uint8_t i = 0; i < 50; i++){
//                 //     odo.cali_map[i] = mean(forward_test_data[i].second, backward_test_data[i].second);
//                 // }

//                 // for(auto & forward_err_item : forward_err){
//                 //     forward_err_item -= round(forward_err_item);
//                 // }

//                 // for(auto & backward_err_item : backward_err){
//                 //     backward_err_item -= round(backward_err_item);
//                 // }

//                 // {
//                 //     real_t forward_max = *std::max_element(std::begin(forward_err), std::end(forward_err));
//                 //     real_t forward_min = *std::min_element(std::begin(forward_err), std::end(forward_err));
//                 // }
//                 // {
//                 //     real_t landing_position_err = landing_position_accumulate / landingturns;
//                 //     constexpr int iter_times = 160;
//                 //     // for(uint8_t i = 0; i < iter_times; i ++){
//                 //     //     odo.update();
//                 //     //     // delayMicros
//                 //     //     // logger.println(odo.getRawLapPosition());
//                 //     //     delayMicroseconds(20);
//                 //     // }
//                 //     for(uint8_t i = 0; i < iter_times; i ++){
//                 //         odo.update();
//                 //         logger.println(odo.getRawLapPosition() - landing_position_err);
//                 //     }
//                 // }
//                 odo.update();
//                 // odo.fixElecRadOffset(odo.getLapPosition() - landing_position_accumulate / landingturns);

//                 // odo.addPostDynamicFixPosition(-odo.warp_around_zero(landing_position_accumulate / landingturns));
//                 // odo.fixElecRad();
//                 // odo.adjustZeroOffset();
//                 // setCurrent(real_t(align_current), real_t(PI / 2));
//                 // delay(100);
//                 // odo.update();
//                 // logger.println(l-anding_position_accumulate / landingturns, odo.getElecRad());

//                 for(auto & item : elecrad_test_data){
//                     logger.println(item);
//                 }

//                 odo.setElecRadOffset(std::accumulate(std::begin(elecrad_test_data), std::end(elecrad_test_data), real_t(0)) / real_t(int(elecrad_test_data.size())));
//                                 logger.println(odo.getElecRad());
//                 // setCurrent(real_t(align_current), real_t(0));
//                 // delay(100);
//                 // odo.update();
//                 // logger.println(odo.getRawLapPosition(), odo.getElecRad());

//                 // setCurrent(real_t(align_current), real_t(-PI / 2));
//                 // delay(100);
//                 // odo.update();
//                 // logger.println(odo.getRawLapPosition(), odo.getElecRad());

//                 // setCurrent(real_t(align_current), real_t(0));
//                 // delay(100);
//                 // odo.update();
//                 // logger.println(odo.getRawLapPosition(), odo.getElecRad());
//                 sw_state(SubState::EXAMINE);
//                 break;
//             case SubState::EXAMINE:
//                 // for(uint8_t i = 0; i < 50; i++){
//                 //     // logger << forward_test_data[i].first << ", " << forward_test_data[i].second << ", " << forward_err[i] << ", " << backward_test_data[i].first << ", " << backward_test_data[i].second << ", " <<  backward_err[i] << endl;
//                 //     logger.println(forward_err[i], backward_err[i]);
//                 // }

//                 // for(uint8_t i = 0; i < poles; i++){
                    
//                 // }
//                 // logger.setEps(4);
//                 // logger.setSpace(", ");
//                 // for(uint8_t i = 0; i < poles; i++){
//                 //     logger.println(forward_err.at(i), backward_err.at(i));
//                 // }

//                 odo.locateRelatively(real_t(0));
//                 setCurrent(real_t(0), real_t(0));
//                 sw_state(SubState::DONE);
//                 break;

//             case SubState::DONE:
//                 return true;
//                 break;
//             default:
//                 break;
//         }
//         cnt++;
//     }
//     return false;
// }

// DoneFlag selfcheck_prog(const InitFlag & init_flag){
//     constexpr int subdivide_micros = 2048;
//     // constexpr int dur = 600;

//     enum class SubState{
//         INIT,
//         TEST_A,
//         REINIT,
//         TEST_B,
//         ANALYSIS,
//         EXAMINE,
//         STOP,
//         DONE
//     };

//     static SubState sub_state = SubState::INIT;

//     static int cnt;

//     auto sw_state = [](const SubState & new_state){
//         sub_state = new_state;
//         cnt = 0;
//     };

//     if(init_flag){
//         sub_state = SubState::INIT;
//         cnt = 0;
//     }

//     static Range_t<real_t> move_range;

//     if(cnt < 0){
//         return false;
//     }

//     else{

//         switch(sub_state){
//             case SubState::INIT:
//                 odo.reset();
//                 odo.update();
//                 odo.update();
//                 move_range.start = odo.getPosition();
//                 move_range.end = move_range.start;
//                 break;
//             case SubState::TEST_A:
//                 coilA = cos(real_t(cnt) * real_t(PI / subdivide_micros));
                
//                 odo.update();
//                 move_range.merge(odo.getPosition());
//                 if(cnt > subdivide_micros) sw_state(SubState::TEST_B);
//             case SubState::TEST_B:
//             case SubState::ANALYSIS:
//             default:
//                 break;
//         }
//     }
//     return false;
// }

// // template<typename T>
// // class baseClass{
// // protected:
// //     T data;
// // public:
// //     void speak(){;}
// //     void run(){;}
// // };

// // class drivedClass:public baseClass<int>{
// // public:
// //     // using baseClass<int>;
// // };

// DoneFlag beep_prog(const InitFlag & init_flag = false){
//     // drivedClass dri;
//     // dri.run();
//     struct Tone{
//         uint32_t freq_hz;
//         uint32_t sustain_ms;
//     };

//     constexpr int freq_G4 = 392;
//     constexpr int freq_A4 = 440;
//     constexpr int freq_B4 = 494;
//     constexpr int freq_C5 = 523;
//     constexpr int freq_D5 = 587;
//     constexpr int freq_E5 = 659;
//     constexpr int freq_F5 = 698;
//     constexpr int freq_G5 = 784;

//     static const auto tones = std::to_array<Tone>({
//         {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
//         {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
//         {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
//         {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
//         {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
//         {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        
//         {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
//         {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
//         {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
//         {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
//         {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
//         {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        
//         {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
//         {.freq_hz = freq_C5,.sustain_ms = 100},  // 1
//         {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
//         {.freq_hz = freq_G4,.sustain_ms = 100},  // 5

//         {.freq_hz = freq_F5,.sustain_ms = 100}   // 6
//     });

//     constexpr float tone_current = 0.4;

//     static uint32_t cnt;
//     static uint32_t tone_index;
//     static uint32_t play_begin_ms;
//     // static Tone * tone = nullptr;

//     if(init_flag){
//         cnt = 0;
//         tone_index = 0;
//         play_begin_ms = millis();
//     }

//     if(millis() >= tones[tone_index].sustain_ms + play_begin_ms){ // play one note done
//         if(tone_index >= tones.size()) return true; // play done
//         else{
//             tone_index++;
//             play_begin_ms = millis();
//         }
//     }
    
//     {
//         // auto play_us = cnt * (1000000 / foc_freq);
//         const auto & tone = tones[tone_index];
//         auto tone_cnt = foc_freq / tone.freq_hz / 2;
//         // auto tone_ms = tones[tone_index].sustain_ms;
//         bool phase = (cnt / tone_cnt) % 2;
//         setCurrent(real_t(tone_current), phase ? real_t(0.5) : real_t(-0.5));
//         cnt++;
//     }
//     // for(const auto & tone : tones){
//     //     uint32_t play_begin_ms = millis();
//     //     uint32_t tone_period_us = 1000000 / tone.freq_hz;
//     //     tone_period_us /= 2;
//     //     while(millis() - play_begin_ms < tone.sustain_ms){
//     //         // svpwm.setDQCurrent(Vector2(real_t(tone_current), real_t(0)), real_t());
//     //         
//     //         delayMicroseconds(tone_period_us);
//     //         setCurrent(tone_current, );
//     //         // svpwm.setDQCurrent(Vector2(real_t(0), real_t(tone_current)), real_t());
//     //         delayMicroseconds(tone_period_us);
//     //     }
//     // }

//     return false;
// }

// #include "src/device/CommonIO/Led/rgbLed.hpp"

// class PanelLed{
// public:
//     enum class Method:uint8_t{
//         Sine = 0,
//         Saw,
//         Squ,
//         Tri
//     };
// protected:
//     RgbLedConcept<true> & led;

//     using Color = Color_t<real_t>;

//     Color color_a;
//     Color color_b;
//     uint16_t period;

//     Method method;
// public:
//     PanelLed(RgbLedConcept<true> & _led) : led(_led){;}

//     void init(){
//         led.init();
//     }

//     void setPeriod(const uint16_t & _period){
//         period = _period;
//     }

//     void setTranstit(const Color & _color_a, const Color & _color_b, const Method & _method){
//         color_a = _color_a;
//         color_b = _color_b;
//         method = _method;
//     }

//     void run(){
//         real_t ratio;
//         real_t _t = t * real_t(100000 / period) * real_t(0.01);
//         switch(method){
//         case Method::Saw:
//             ratio = frac(_t);
//             break;
//         case Method::Sine:
//             ratio = abs(2 * frac(_t) - 1);
//             break;
//         case Method::Squ:
//             ratio = sign(2 * frac(_t) - 1) * 0.5 + 0.5;
//             break;
//         case Method::Tri:
//             ratio = abs(2 * frac(_t) - 1);
//             break;
//         }

//         Color color_mux = color_a.linear_interpolate(color_b, ratio);
//         led = color_mux;
//     }
// };

// // class Panel{
// //     Panel
// // };
// RgbLedDigital<true> led_instance(portC[14], portC[15], portC[13]);
// PanelLed panel_led = PanelLed(led_instance);

// #include "nameof.hpp"


// void run(){
//     // logger.println("RUN");
//     switch(run_status){
//         case RunStatus::INIT:
//             run_status = RunStatus::CALI;
//             cali_prog(true);
//             break;
//         case RunStatus::CALI:
//             if(cali_prog()){
//                 // beep_prog(true);
//                 // run_status = RunStatus::BEEP;
//                 active_prog(true);
//                 run_status = RunStatus::ACTIVE;
//             }
//             break;
        
//         case RunStatus::BEEP:
//             if(beep_prog()){
//                 active_prog(true);
//                 run_status = RunStatus::ACTIVE;
//             }
//             break;

//         case RunStatus::INACTIVE:
//             run_status = RunStatus::ACTIVE;
//             break;
//         case RunStatus::ACTIVE:
//             if(active_prog()){
//                 run_status = RunStatus::INACTIVE;
//             }
//             break;
//         default:
//             break;
//     }
// }

// void stepper_test(){

//     uart1.init(115200 * 4);

//     logger.setEps(4);

//     timer1.init(4096, 1, Timer::TimerMode::CenterAlignedDownTrig);
//     timer1.enableArrSync();
//     // forwardChannelA.enableSync();
//     // forwardChannelB.enableSync();
//     // backwardChannelA.enableSync();
//     // backwardChannelB.enableSync();

//     // forwardChannelA.setPolarity(false);
//     // forwardChannelB.setPolarity(false);
//     // backwardChannelA.setPolarity(false);
//     // backwardChannelB.setPolarity(false);

//     // forwardChannelA.init();
//     // forwardChannelB.init();
//     // backwardChannelA.init();
//     // backwardChannelB.init();

//     timer3.init(1024, 1, Timer::TimerMode::CenterAlignedDownTrig);
//     timer3.enableArrSync();

//     // verfChannelA.enableSync();
//     // verfChannelB.enableSync();
//     // verfChannelA.setPolarity(true);
//     // verfChannelB.setPolarity(true);

//     // verfChannelA.init();
//     // verfChannelB.init();

//     svpwm.init();

//     coilA.setClamp(real_t(1));
//     coilB.setClamp(real_t(1));

//     coilA.init();
//     coilB.init();

//     spi1.init(18000000);
//     spi1.bindCsPin(portA[15], 0);



//     odo.init();

//     // cxxopts::Options options("test", "A brief description");

//     // options.add_options()
//     //     ("b,bar", "Param bar", cxxopts::value<std::string>())
//     //     ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
//     //     ("f,foo", "Param foo", cxxopts::value<int>()->default_value("10"))
//     //     ("h,help", "Print usage")
//     // ;

//     // const char * r[] =  {"mm", "xsa"};
//     // auto result = options.parse(2,r);

//     // if (result.count("help"))
//     // {
//     //   logger << options.help() << endl;
//     //   exit(0);
//     // }
//     // bool debug = result["debug"].as<bool>();
//     // std::string bar;
//     // if (result.count("bar"))
//     //   bar = result["bar"].as<std::string>();
//     // int foo = result["foo"].as<int>();

//     // adc1.init(
//     //     {},{
//     //         AdcChannelConfig{.channel = AdcChannels::CH3, .sample_cycles = AdcSampleCycles::T71_5},
//     //         AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T71_5}
//     //     });
//     // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
//     // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
//     // timer3[4] = 0;
//     // adc1.setPga(AdcOnChip::Pga::X64);
//     // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
//     // TIM3->CH4CVR = TIM3->ATRLR >> 1;
//     // adc1.enableCont();
//     // adc1.enableScan();
//     // adc1.enableAutoInject();

//     // auto & bled = portC[13];
//     // bled.OutPP();
//     panel_led.init();
//     if(false){
        
//     }


//     // logger << ex3.dump();

//     if(false){
//         // odo.reset();
//         // logger.println("Cali And Direction Identify..");

//         // constexpr int forwardpreturns = 2;
//         // constexpr int forwardturns = 5;
//         // constexpr int backwardpreturns = 2;
//         // constexpr int backwardturns = 5;
//         // constexpr int subdivide_micros = 64;
//         // constexpr int dur = 600;
//         // constexpr float cali_current = 0.6;

//         // real_t elecrad = real_t(0);
//         // real_t elecrad_step = real_t(TAU / subdivide_micros);
//         // real_t percent = real_t(1.0 / (forwardturns + backwardturns));

//         // struct{
//         //     public:

//         //     real_t start_pos;
//         //     real_t after_forward_pos;
//         //     real_t after_backward_pos;

//         //     const real_t max_diff_tolerance = real_t(0.05);

//         //     bool isForwardBackwardFluent(const int & forward_total_turns, const int & backward_total_turns){
//         //         real_t forward_trip = after_forward_pos - start_pos;
//         //         real_t backward_trip = after_backward_pos - after_forward_pos;

//         //         if(ABS(forward_trip) < max_diff_tolerance || ABS(backward_trip) < max_diff_tolerance){
//         //             logger.println("Stuck happend while froward and backward");
//         //             logger.println("Forward trip:", forward_trip);
//         //             logger.println("Backward trip:", backward_trip);
//         //             return false;
//         //         }

//         //         if(ABS(ABS(forward_trip) * backward_total_turns - ABS(backward_trip) * forward_total_turns)
//         //              > max_diff_tolerance * forward_total_turns * backward_total_turns){
//         //             logger.println("Bump happend while froward and backward");
//         //             logger.println("Forward trip:", forward_trip);
//         //             logger.println("Backward trip:", backward_trip);
//         //             logger.println(ABS(ABS(forward_trip) * backward_total_turns - ABS(backward_trip) * forward_total_turns));
//         //             logger.println(max_diff_tolerance * forward_total_turns * backward_total_turns);
//         //             return false;
//         //         }

//         //         if(forward_trip * backward_trip >= 0){
//         //             logger.println("Direction still while forward and backward");
//         //             return false;
//         //         }

//         //         return true;
//         //     }

//         //     bool needToInverse(){
//         //         real_t forward_trip = after_forward_pos - start_pos;
//         //         // real_t backward_trip = after_backward_pos - after_forward_pos;
//         //         return forward_trip < 0;
//         //     }
//         // }pos_trend;

//         // // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
//         // setCurrent(real_t(cali_current), elecrad);
//         // delay(10);
//         // odo.update();
//         // pos_trend.start_pos = odo.getPosition();
//         // delay(10);

//         // for(int i = -forwardpreturns * subdivide_micros;i < forwardturns * subdivide_micros; i++){

//         //     if(i % subdivide_micros == 0 && i >= 0){//measureable
//         //         odo.locateElecrad(percent);
//         //     }

//         //     elecrad += elecrad_step;
//         //     // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
//         //     setCurrent(real_t(cali_current), elecrad);
//         //     delayMicroseconds(dur);
//         // }

//         // delay(10);
//         // odo.update();
//         // pos_trend.after_forward_pos = odo.getPosition();
//         // delay(10);

//         // for(int i = -backwardpreturns * subdivide_micros; i < backwardturns * subdivide_micros; i++){

//         //     if(i % subdivide_micros == 0 && i >= 0){//measureable
//         //         odo.locateElecrad(percent);
//         //     }

//         //     elecrad -= elecrad_step;
//         //     // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
//         //     setCurrent(real_t(cali_current), elecrad);
//         //     delayMicroseconds(dur);
//         // }

//         // delay(10);
//         // odo.update();
//         // pos_trend.after_backward_pos = odo.getPosition();
//         // delay(10);

//         // logger.println("Cali Done..");
//         // logger.println("elecrad offset is", odo.getElecRadOffset());
//         // pos_trend.isForwardBackwardFluent(forwardpreturns + forwardturns, backwardpreturns + backwardturns);

//     }

//     // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), real_t(0));
//     // setCurrent(real_t(0.3), real_t());
//     // delay(200);
//     // odo.locateElecrad();

//     timer4.init(foc_freq);
//     timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
//     timer4.bindCb(Timer::IT::Update, std::function<void(void)>(run));

//     // delay(200);
//     // motor.enable();
//     // motor.trackPos(real_t(0));
//     // motor.setMaxCurrent(real_t(0.45));

//     logger.setEps(4);
//     panel_led.setPeriod(200);
//     panel_led.setTranstit(Color(), Color(0,1,0,0), PanelLed::Method::Squ);
//     while(true){
//         // real_t total = real_t(3);
//         // static real_t freq = real_t(10);
//         // static real_t freq_dir = real_t(1);
//         // const real_t freq_delta = real_t(20);
//         // if(freq > real_t(300)) freq_dir = real_t(-1);
//         // else if(freq < real_t(4)) freq_dir = real_t(1);
//         // static real_t last_t = t;
//         // real_t delta = (t - last_t);
//         // freq += delta * freq_dir * freq_delta;
//         // last_t = t;
//         // static real_t ang = real_t(0);
//         // ang += freq * delta;
//         // real_t target = (total / freq) * sin(ang);

//         // target_pos = sign(frac(t) - 0.5);
//         // target_pos = sin(t);
//         // logger.println(odo.getPosition(), est_pos, est_speed, ctrl.elecrad_offset_output, odo.getRawLapPosition(), odo.getLapPosition());
//         // logger.println(est_speed, est_pos, targets.curr, run_elecrad_addition);
//         // , est_speed, t, odo.getElecRad(), openloop_elecrad);
//         // logger << est_pos << est_speed << run_current << run_elecrad_addition << endl;
//         // logger.println(est_pos, est_speed, run_current, run_elecrad_addition);
//         RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);
//         static String temp_str = "";

//         // bool led_status = (millis() / 200) % 2;
//         // bled = led_status;
//         panel_led.run();

//         if(logger.available()){
//             char chr = logger.read();
//             if(chr == '\n'){
//                 temp_str.trim();
//                 // logger.println(temp_str);
//                 if(temp_str.length()) parseLine(temp_str);
//                 // logger.println(temp_str);
//                 temp_str = "";
//             }else{
//                 temp_str.concat(chr);
//             }
//         }

//         Sys::Clock::reCalculateTime();
//     }
//     while(true);
// }
// };


// namespace SpreadCycle{

// constexpr float coil_inductor_mH = 0.83;
// constexpr float coil_resistor_Ohm = 2.24;
// constexpr float busbar_voltage = 9.0;

// constexpr float c1 = busbar_voltage / coil_inductor_mH;
// constexpr float coil_current = 0.23;
// constexpr float c2 = - coil_current * coil_resistor_Ohm / coil_inductor_mH;

// PwmChannel coil_pwm_p(timer1[3]);
// PwmChannel coil_pwm_n(timer1[4]);

// real_t dual_duty[2] = {
//     real_t(0.3),
//     real_t(0.14)
// };

// uint16_t dual_cvr[2] = {699,2299};

// real_t chopper_current = real_t(0.2);

// uint32_t chopper_run_cnt = 0;
// struct {
//     bool odd_is_forward:1;
//     bool even_is_forward:1;
// }chopper_run_mode;

// void chopper_run(){

//     if(chopper_run_cnt == 0){
//         chopper_run_mode.odd_is_forward = true;
//         chopper_run_mode.even_is_forward = false;
//         chopper_run_cnt = 1;
//         return;
//     }

//     chopper_run_cnt++;
//     bool cycle_is_odd = chopper_run_cnt % 2;

//     bool cycle_is_forward = cycle_is_odd ? chopper_run_mode.odd_is_forward : chopper_run_mode.even_is_forward;
//     // bool cycle_is_forward = cycle_is_odd;

//     // int pwm_duty_p = cycle_is_forward ? 0 : dual_cvr[cycle_is_odd];
//     // int pwm_duty_n = cycle_is_forward ? dual_cvr[cycle_is_odd] : 0;
//     int pwm_duty = dual_cvr[cycle_is_odd];
//     // int backward_duty = dual_cvr[1];
//     if(cycle_is_forward){
//         timer1[3] = pwm_duty;
//         timer1[4] = 0;
//     }else{
//         timer1[3] = 0;
//         timer1[4] = pwm_duty;
//     }

// }


// void chopper_test(){
//     uart1.init(115200 * 8, Uart::Mode::TxRx);
//     Printer & logger = uart1;
//     logger.setSpace(",");
//     logger.setEps(4);

//     timer1.init(36000);
//     timer1.enableArrSync();
//     // timer1.enableCvrSync();
//     timer1[3].enableSync();
//     timer1[4].enableSync();
//     timer1[3].setPolarity(false);
//     timer1[4].setPolarity(false);

//     timer1[3].init();
//     timer1[4].init();

//     coil_pwm_p.setClamp(real_t(0.4));
//     coil_pwm_n.setClamp(real_t(0.4));

//     timer3.init(36000);
//     timer3[2].init();
//     timer3[3].init();
//     timer3[2].setPolarity(true);
//     timer3[3].setPolarity(true);

//     timer3[2] = real_t(0.4);
//     timer3[3] = real_t(0.4);



//     timer1.bindCb(Timer::IT::Update, std::function<void(void)>(chopper_run));
//     timer1.enableIt(Timer::IT::Update, NvicPriority(0, 0));

//     while(true){
//         logger.println(int(timer1[3]), int(timer1[4]));
//     }
// }



// };

#include "image/packedImage.hpp"
#include "src/bus/bus_inc.h"

int main(){

    Sys::Misc::prework();


    // stepper_app();
    // stepper_app_new();
    // StepperTest::stepper_test();

    // image.putseg_h8_unsafe(Vector2i{0,0}, 0x5a, true);
    // image.putseg_v8_unsafe(Vector2i{0,0}, 0x39, true);

    // for(auto & datum : data){
    //     logger.println(datum);
    // }

    uart2.init(115200, Uart::Mode::TxOnly);
    Printer & logger = uart2;
    logger.setEps(4);
    logger.setRadix(10);
    logger.setSpace(",");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    I2cDrv at24drv(i2csw, AT24C02::default_id);
    AT24C02 at24(at24drv);

    delay(200);
    at24.init();
    at24.store(at24.load(0) + 1, 0);
    delay(20);
    uart2.println(at24.load(0));
    while(true);
    // at24.
    pedestrian_app();
    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    // osc_test();
}

    // timer1.init(25600);

    // auto tim1ch1 = timer1[1];
    // tim1ch1.init();

    // auto tim1ch1n = timer1[-1];
    // tim1ch1n.init();

    // auto tim1ch2 = timer1[2];
    // tim1ch2.init();

    // auto tim1ch3 = timer1[3];
    // tim1ch3.init();

    // auto tim1ch4 = timer1[4];
    // tim1ch4.init();


    // timer1.initBdtr(AdvancedTimer::LockLevel::Off, 0);
    // timer1.enable();

    // auto pwmCoilP = PwmChannel(tim1ch3);
    // pwmCoilP.init();

    // auto pwmCoilN = PwmChannel(tim1ch4);
    // pwmCoilN.init();
    // uart2.init(UART2_Baudrate);
    // uart2.setEps(4);

    // Gpio Led = Gpio(GPIOC, Pin::_13);
    // Led.OutPP();
    // Gpio mosi_pin = Gpio(SPI1_MOSI_Port, (Pin)SPI1_MOSI_Pin);
    // Gpio miso_pin = Gpio(SPI1_MISO_Port, (Pin)SPI1_MISO_Pin);
    // Gpio sck_pin = Gpio(SPI1_SCLK_Port, (Pin)SPI1_SCLK_Pin);
    // GpioVirtual cs_pin = Gpio(SPI1_CS_Port, (Pin)SPI1_CS_Pin);
    // cs_pin.OutPP();
    // SpiSw spisw(sck_pin, mosi_pin, miso_pin, cs_pin);
    // spisw.init();
    // SpiDrv drv(spisw, 0);
    // BMI270 bmi(drv);
    // bmi.init();
    // bmi.flush();

    // Gpio sda_pin = Gpio(SPI1_MOSI_Port, (Pin)SPI1_MOSI_Pin);
    // Gpio scl_pin = Gpio(SPI1_SCLK_Port, (Pin)SPI1_SCLK_Pin);
    // I2cSw i2csw(scl_pin, sda_pin);
    // i2csw.init(40000);
    // I2cDrv i2cdrv(i2csw, 0xd0);
    // MPU6050 mpu(i2cdrv);
    // mpu.init();
    // Axis6 & imu = mpu;
    // Gpio TrigA = Gpio(GPIOC, Pin::_14);
    // Gpio TrigB = Gpio(GPIOC, Pin::_15);
    // TrigA.InPullUP();
    // TrigB.InPullUP();

    // Gpio a0 = Gpio(GPIOA, Pin::_0);
    // a0.InAnalog();
    // AdcChannelOnChip ac0(ADC1);

    // adc1.clearRegularQueue();
    // adc1.AddChannelToQueue(ac0);
    // adc1.init();
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    // adc1.start();

    // ExtiChannel channel(TrigA, NvicPriority(2,1));

    // // adc1.init();
    // real_t cnt;
    // auto cb = [&Led, &TrigB, &cnt](){cnt += real_t((bool(TrigB) ? 1 : -1)) / real_t(16384);};
    // channel.init();
    // channel.bindCb(cb);

    // uart1.init(115200);
    // VtfRequest(15EXTI15_10_IRQn, 0, cb);
    // GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* GPIOA ----> EXTI_Line0 */
    // EXTI_Init(&EXTI_InitStructure);



    // auto filter = BurrFilter_t<real_t>();
    // Vector3 accel;
    // real_t f0 = real_t(6);
    // real_t f_test = real_t(0.1);
    // LowpassFilter_t<real_t, real_t> lpf(f0);
    // HighpassFilter_t<real_t, real_t> hpf(f0);
    // auto lob = LinearObersver_t<real_t, real_t>();
    // auto lob2 = LinearObersver_t<real_t, real_t>();
    // real_t x;
    // real_t y;
    // real_t f;
    // while(true){
    //     real_t x = 6*sin(t * f_test * TAU);


    //     if(millis()%10 == 0) lob2.update(lob.update(real_t(x), t),t);

    //     uart2.println(x, lob2.predict(lob.predict(t),t) , lob.predict(t));
        // t = real_t(float(millis()) / float(10000));
        // Led = (millis() / 100) & 0b1;
        // if((millis() % 100) == 0){cb(); delay(1);}
    // }
    // }while(true){
    //     // bmi.getChipId();
    //     imu.flush();
    //     // real_t x, y, z;
    //     imu.getAccel(accel.x, accel.y, accel.z);
    //     // imu.getAccel(x,y,z);
    //     accel.normalize();

    //     uart2.println(accel.x,accel.y,accel.z, accel.length());

    //     // static bool state = false;
    //     // state = !state;
    //     // GPIO_WriteBit(GPIOC,GPIO_Pin_13, state);
    //     // delay(100);
    //     // reCalculateTime();
    //     real_t _t = real_t(int(micros() % 10000)) / real_t(10000 / TAU);
    //     // uint8_t _t = micros() % tablesize;
    //     // uint16_t cnt = 0;
    //     // uni_to_u16(_t, cnt);
    //     // pwmCoilP = (sin(t*100) / 2 + 0.5);
    //     real_t waves[3];
    //     // waves[0] = INVLERP(sin(t*100), -1, 1);
    //     waves[0] = sin(_t * 1);
    //     // waves[0] = sintable[_t];
    //     // waves[1] = INVLERP(sin(t*300) / 3, -1, 1);
    //     waves[1] = sin(_t * 3) / 3;
    //     // waves[1] = sintable[_t * 3] / 3;
    //     // waves[1] = real_t(0);
    //     waves[2] = sin(_t * 5) / 5;
    //     // waves[2] = sintable[_t * 5] / 5;
    //     // waves[2] = INVLERP(sin(t*500) / 5, -1, 1);
    //     auto temp = real_t(0);
    //     for(auto & wave : waves) temp += wave;
    //     // waves[0] + waves[1] + waves[2];
    //     //  + waves[1] + waves[2];
    //     pwmCoilP = temp / 2 + real_t(0.5);
    //     // uart2.println((float)real_t(pwmCoilP));

    // }
    // auto coil = Coil2(pwmCoilP, pwmCoilN);
    // coil.init();
    // coil.setDuty(real_t(-0.4));




    // auto pv = PortVirtual<8>();
    // pv.bindPin(Led, 0);
    // uart1.init(UART1_Baudrate);


    // Spi & spi = spisw;
    // <DECLTYPE(miso_pin, mosi_pin)>
    // SpiDrv spiDrvMagSensor = SpiDrv(spi, 0);
    // MA730 mag_sensor(spiDrvMagSensor);
    // spi.init(SPI1_BaudRate);

    // HC595<2> hc595(sck_pin, mosi_pin, cs_pin);

    // HC595Single hc595single(sck_pin, mosi_pin, cs_pin);
    // GpioVirtual gv = GpioVirtual(&hc595single, 0);
    // hc595single.init();
    // while(true){
        // Led = !Led;
        // static bool i = false;
        // i = !i;
        // delay (100);
        // static uint8_t cnt = 0;
        // mag_sensor.setDirection(true);
        // mag_sensor.getRawData();
        // spi.begin(0);
        // spi.write(0xA5);
        // static uint32_t ret;
        // hc595single.writeByIndex(0, i);
        // gv = !gv;
        // spi.transfer(ret, ret+1);
        // hc595.setContent({0, cnt});

        // sendReset(mosi_pin);

        // for(uint8_t _ = 0; _ < 3; _++)
        // for(uint8_t mask = 0x80; mask; mask >>= 1){
        //     sendCode(mosi_pin, bool(mask & cnt));
        // }

        // cnt++;
        // hc595single = 1;
        // uart2.println(ret);
        // spi.end();
        // cnt++;
        // pv.writeByIndex(0, i);
        // for(uint8_t _ = 0; _ < 32; _++)spi1.write(cnt++);
        // uint32_t dummy = 0;
        // Led = i;
    // }
    // while(true){
    //     real_t dutyX = 0.5 + 0.5 * cos(t);
    //     real_t dutyY = 0.5 + 0.5 * sin(t);
    //     servoY.setDuty(dutyY);
    //     servoX.setDuty(dutyX);
    //     reCalculateTime();
    //     // Led = true;
    //     // pv.set(Pin1);
    //     delay(20);
    //     uart2.println(acos(2 * (dutyX - 0.5)));
    //     // volatile String x = dutyX.toString(3);
    //     // Led = false;
    //     // pv.clr(Pin::_1);
    //     // delay(100);
    // }
    // tim1ch2.init();
    // tim1ch2 = real_t(0.8);


    // TIM2_GPIO_Init();
    // TIM_Encoder_Init(TIM2);

    // TIM4_GPIO_Init();
    // TIM_PWM_Init(TIM4, pwm_arr);

    // TIM3_GPIO_Init();
    // TIM_PWM_Init(TIM3, ir_arr);
    // ADC1_GPIO_Init();
    // ADC1_Init();

    // uart1.init(UART1_Baudrate);

    // adc1.init();
    // Basis_t<real_t> basis1(Vector3_t<real_t>(0, 0, 0), real_t(0.1));
    // uart2.init(576000);
    // can1.init(Can1::BaudRate::Mbps1);
    // i2c1.init(100000);
    // i2c1.setTimeout(320);
    // i2cSw.
    // mag_sensor.init();
    // earth_sensor.init();
    // prs_sensor.init();
    // mt_sensor.init();

    // Gpio sckPin(GPIOB, GPIO_Pin_4);
    // Gpio sdoPin(GPIOB, GPIO_Pin_5);


    // HX711 hx711(sckPin, sdoPin);
    // hx711.init();
    // hx711.setConvType(HX711::ConvType::A128);
    // while(true){
    //     while(!hx711.isIdle());
    //     Led = !Led;
    //     uint32_t weight = 0;
    //     hx711.getWeightData(weight);
    //     uart2.println(weight);
    // }
    // while(true){
    //     uart2.println(mt_sensor.getRawPosition());
    //     delay(20);
    //     Led = !Led;
    // }

    // while(true){
    //     while(!prs_sensor.isIdle());
    //     // delay(2);
    //     int32_t prs = 0;
    //     prs_sensor.getPressure(prs);
    //     real_t(8.0);
    //     uart2.println(prs);
    //     // delay(10);
    // }
    // while(true){
    //     while(!earth_sensor.isIdle());
    //     delay(2);
    //     real_t x, y, z;
    //     earth_sensor.getMag(x, y, z);
    //     uart2.println(x, y, z);
    //     delay(10);
    // }
    // bool tx_role = getChipId() == 6002379527825632205;

    // spi2.init(72000000);
    // spi1.init(18000000);

    // GLobal_Reset();
    // SysInfo_ShowUp(uart2);



    // bool use_tft = true;
    // bool use_mini = false;
    // if(use_tft){
    // if(use_mini){
    //     tftDisplayer.init();
    //     tftDisplayer.setDisplayArea(Rect2i(0, 0, 160, 80));
    //     tftDisplayer.setDisplayOffset(Vector2i(1, 26));
    //     tftDisplayer.setFlipX(true);
    //     tftDisplayer.setFlipY(false);
    //     tftDisplayer.setSwapXY(true);
    //     tftDisplayer.setFormatRGB(false);
    //     tftDisplayer.setFlushDirH(false);
    //     tftDisplayer.setFlushDirV(false);
    //     tftDisplayer.setInversion(true);
    // }else{
    //     tftDisplayer.init();
    //     tftDisplayer.setDisplayArea(Rect2i(0, 0, 240, 240));

    //     tftDisplayer.setFlipX(false);
    //     tftDisplayer.setFlipY(false);
    //     tftDisplayer.setSwapXY(false);
    //     tftDisplayer.setFormatRGB(true);
    //     tftDisplayer.setFlushDirH(false);
    //     tftDisplayer.setFlushDirV(false);
    //     tftDisplayer.setInversion(true);
    // }}else{
    //     oledDisPlayer.init();

    //     oledDisPlayer.setOffsetY(6);
    //     oledDisPlayer.setFlipX(false);
    //     oledDisPlayer.setFlipY(false);
    //     oledDisPlayer.setInversion(false);
    // }

    // mpu.init();
    // ext_adc.init();
    // // print("ext_adc ini")
    // ext_adc.setContMode(true);
    // ext_adc.setFS(SGM58031::FS::FS4_096);
    // ext_adc.setMux(SGM58031::MUX::P0NG);
    // ext_adc.setDataRate(SGM58031::DataRate::DR960);
    // ext_adc.startConv();
    // radio.init();
    // uart1.println("flashCapacity: ", extern_flash.getDeviceCapacity());
    // uint8_t * buf = new uint8_t[uart1.read()];
    // uart1.println(String((const char *)buf));
    // tcs.init();
    // tcs.setIntegration(48);
    // tcs.setGain(TCS34725::Gain::X60);
    // tcs.startConv();
    // vlx.init();
    // vlx.setContinuous(true);
    // vlx.setHighPrecision(false);
    // vlx.startConv();
    // mags.init();


    // mag_sensor.setPulsePerTurn(30);
    // Font6x8 font6x8;
    // Painter<RGB565> painter(&tftDisplayer, &font6x8);
    // uart1.setSpace(",");
    // uart2.setSpace(",");

    // painter.setColor(RGB565::BLACK);
    // painter.flush();
    // painter.setColor(RGB565::WHITE);
    // painter.drawString(Vector2i(0,0), String((int16_t)TIM2->CNT));
    // painter.drawString(Vector2i(0,8), String(0.2));
    // painter.drawString(Vector2i(0,16), String(0));
    // painter.drawString(Vector2i(0,24), String(3672));

    // uart2.println("initialized");
    // while(1){

        // uart2.println(mag_sensor.getRawAngle(), mag_sensor.getMagnitude(), mag_sensor.getMagStatus());
        // Led = !Led;
        // updatePosition();
        // real_t pos = motorPosition.accPosition * 10;
        // static PID pos_pid = PID(real_t(10), real_t(0), real_t(0));
        // pos_pid.setClamp(real_t(0.94));

        // real_t target = sin(t);
        // // real_t duty = CLAMP((target - pos) * 30, real_t(-0.94), real_t(0.94));
        // real_t duty = pos_pid.update(target, pos);
        // // real_t duty = floor(fmod(t, real_t(3))-1);
        // // real_t duty = sin(t * real_t(-2));
        // setMotorDuty(duty);
        // real_t position = t;
        // const real_t omega = real_t(10);
        // const real_t amplitude = real_t(1);
        // real_t rad = floor(omega * position / real_t(TAU/4)) * real_t(TAU/4);
        // real_t a = sin(rad) * amplitude;
        // real_t b = cos(rad) * amplitude;
        // uart1.println(a,b);
        // setABCoilDuty(a, b);
        // reCalculateTime();
        // blueLed = !blueLed;
        // setIrState((bool)blueLed);
        // if(ir_encoder.tick()){
        //     static uint8_t a = 0x00;
        //     ir_encoder.emit(a++, a);
        // }
        // setIrState(true);
        // delay(2);
        // delayMicroseconds(160);
        // uart1.println(millis());
        // t = real_t(TAU/4);
        // ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
        // while(ADC_GetFlagStatus(ADC1,ADC_FLAG_JEOC)==RESET);

        // uint16_t ad1 = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_1);
        // uint16_t ad2 = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_2);
        // uart1.println(ir_encoder.bit_prog, (uint8_t)ir_encoder.byte_prog, (uint8_t)ir_encoder.encode_prog, TIM3->CH1CVR);
//     }
// }
