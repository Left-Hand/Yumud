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
// SSD1306 oledDisPlayer(spiDrvOled);
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

    TIM3->CH4CVR = TIM3->ATRLR >> 1;
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
        //         // digitalWrite(BCK, HIGH);
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



namespace StepperTest{

Printer & logger = uart1;

auto & forwardChannelB = timer1[1];
auto & backwardChannelB = timer1[2];
auto & forwardChannelA = timer1[3];
auto & backwardChannelA = timer1[4];

PwmChannel forwardPwmA(forwardChannelA);
PwmChannel forwardPwmB(forwardChannelB);

PwmChannel backwardPwmA(backwardChannelA);
PwmChannel backwardPwmB(backwardChannelB);

auto & verfChannelA = timer3[3];
auto & verfChannelB = timer3[2];

PwmChannel pwmCoilB(verfChannelB);
PwmChannel pwmCoilA(verfChannelA);

Coil1 coilA(portA[10], portA[11], pwmCoilA);
Coil1 coilB(portA[8], portA[9], pwmCoilB);
// TB67H450 coilA(forwardChannelA, backwardChannelA, verfChannelA);
// TB67H450 coilB(forwardChannelB, backwardChannelB, verfChannelB);

SVPWM2 svpwm(coilA, coilB);

SpiDrv mt6816_drv(spi1, 0);
MT6816 mt6816(mt6816_drv);

auto odo = OdometerPoles(mt6816);

real_t target_pos;



constexpr uint32_t foc_freq = 36000;
constexpr uint32_t est_freq = 1800;
constexpr uint32_t est_devider = foc_freq / est_freq;
constexpr float foc_execute_duty = 0.3;

constexpr float openloop_current_limit = 0.3;
constexpr float cali_current = 0.4;



constexpr uint32_t foc_period_micros = 1000000 / foc_freq;
uint32_t foc_pulse_micros;


real_t est_speed = real_t();
real_t raw_pos = real_t();
real_t est_pos = real_t();
real_t est_elecrad = real_t();

real_t run_current = real_t();
real_t run_elecrad_addition = real_t();

struct Targets{
    real_t curr;
    real_t speed;
    real_t pos;
    real_t time;
};

Targets targets;

struct Setpoints{

};


struct StallObserver{
    real_t max_position_error;
    real_t inspect_enable_speed_threshold;
    uint32_t entry_time_ms;
    uint32_t max_sustain_time_ms;

    bool update(const real_t & target_position, const real_t & measured_position, const real_t & measured_speed){
        if(abs(measured_speed) < inspect_enable_speed_threshold){ // stall means low speed
            if(entry_time_ms == 0){
                if(abs(target_position - measured_position) > max_position_error){ //
                    entry_time_ms = millis();
                }
            }else{
                return (millis() - entry_time_ms> max_sustain_time_ms);
            }

        }else{
            entry_time_ms = 0;
        }
        return false;
    }
};
struct SpeedCtrl{
    real_t kp = real_t(0.03);
    real_t ki;

    real_t kp_clamp = real_t(0.1);
    real_t intergal;
    real_t intergal_clamp;
    real_t ki_clamp;


    real_t current_output;
    real_t elecrad_offset_output;

    real_t elecrad_addition;
    real_t elecrad_addition_clamp = real_t(0.8);


    bool inited = false;
    void update(const real_t & goal_speed,const real_t & measured_speed){
        if(!inited){
            inited = true;
            elecrad_offset_output = real_t(0);
            elecrad_addition = real_t(0);
        }

        real_t error = goal_speed - measured_speed;
        if(goal_speed < 0) error = - error;
        // real_t abs_error = abs(error);

        real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
        intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
        real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

        elecrad_addition = CLAMP(elecrad_addition + kp_contribute + ki_contribute, real_t(0), elecrad_addition_clamp);

        real_t elecrad_offset = real_t(PI / 2) + elecrad_addition;
        if(goal_speed < 0) elecrad_offset = -elecrad_offset;

        current_output = targets.curr;
        elecrad_offset_output = elecrad_offset;
    }

    real_t getElecradOffset(){
        return elecrad_offset_output;
    }

    real_t getCurrent(){
        return current_output;
    }
};

std::vector<String> splitString(const String& input, char delimiter) {
    std::vector<String> result;

    int startPos = 0;
    int endPos = input.indexOf(delimiter, startPos);

    while (endPos != -1) {
        String token = input.substring(startPos, endPos);
        result.push_back(token.c_str());

        startPos = endPos + 1;
        endPos = input.indexOf(delimiter, startPos);
    }

    if (startPos < input.length()) {
        String lastToken = input.substring(startPos);
        result.push_back(lastToken.c_str());
    }

    return result;
}

void parseCommand(const char & argc, const std::vector<String> & argv){
    switch(argc){
        case 'C':
            if(argv.size() == 0) goto syntax_error;
            targets.curr = real_t(argv[0]);
            break;
        case 'E':
            if(argv.size() == 0) goto syntax_error;
            run_elecrad_addition = real_t(argv[0]);
            break;
        case 'S':
            if(argv.size() == 0) goto syntax_error;
            targets.speed = real_t(argv[0]);
            break;
        case 'R':
            __disable_irq();
            NVIC_SystemReset();
        syntax_error:
            // logger.println("SyntexError", argc);
            break;
        default:
            break;
    }
}
void parseLine(const String & line){
    if(line.length() == 0) return;
    auto tokens = splitString(line, ' ');
    auto argc = tokens[0][0];
    tokens.erase(tokens.begin());
    parseCommand(argc, tokens);
}


void setCurrent(const real_t & _current, const real_t & _elecrad){
    coilA = cos(_elecrad) * _current;
    coilB = sin(_elecrad) * _current;
}

real_t openloop_elecrad;
        static SpeedCtrl ctrl;
    
static void run(){
        uint32_t foc_begin_micros = nanos();
        // setCurrent(ctrl.getCurrent(), est_elecrad + ctrl.getElecradOffset());
        setCurrent(ctrl.getCurrent(), est_elecrad + run_elecrad_addition);

        odo.update();

        raw_pos = odo.getPosition();

        static bool first_entry = true;
        if(first_entry){
            first_entry = false;
            est_pos = raw_pos;
            est_speed = real_t();
        }

        static real_t last_raw_pos = odo.getPosition();
        real_t delta_raw_pos = raw_pos - last_raw_pos;
        last_raw_pos = raw_pos;

        // constexpr float fix_dynamic_delay_speed_threshold = 10.0;
        // constexpr float fix_dynamic_delay_min_delta_pos = fix_dynamic_delay_speed_threshold / foc_freq;


        static uint32_t est_cnt = 0;
        est_cnt++;

        static real_t est_delta_raw_pos_intergal = real_t();
        static real_t est_pos_lantency = real_t();

        if(est_cnt == est_devider){ // est happens
            real_t est_speed_new = est_delta_raw_pos_intergal * (int)est_freq;

            est_speed.value = (est_speed_new.value + est_speed.value * 3) >> 2;
            est_pos_lantency = est_speed / (int)foc_freq;
            // est_pos_lantency = real_t(0);
            // est_pos_lantency.value = est_speed.value / foc_freq;

            est_delta_raw_pos_intergal = real_t();
            est_cnt = 0;

            ctrl.update(real_t(targets.speed), est_speed);
            // ctrl.current_output = real_t(0.2);
            // ctrl.elecrad_offset_output = real_t(PI / 2);
        }else{
            est_delta_raw_pos_intergal += delta_raw_pos;
        }

        est_elecrad = odo.getElecRad();

        // if(true){//takes no effect
        //     est_pos = raw_pos;

        //     // est_pos -= position_offs[odo.position2pole(raw_pos)];

        //     // if(abs(delta_raw_pos) > fix_dynamic_delay_min_delta_pos){
        //     if(true){
        //         est_pos += est_pos_lantency;
        //     }if(false){
        //         est_pos += real_t(0);
        //     }

        // }else{
            // _iq fix = (est_speed.value >> 2) >> 15;
            // _iq fix = 30;
            // est_pos = raw_pos + real_t(fix);
            // est_pos.value = raw_pos.value + 10;
        est_pos = raw_pos;
        // }


        // setCurrent(real_t(0.2), est_elecrad + real_t(2.37));
        // setCurrent(run_current, est_elecrad + run_elecrad_addition);
        // setCurrent(run_current, frac(frac(t) * 50) * TAU);
        uint32_t foc_end_micros = nanos();
        foc_pulse_micros = foc_end_micros - foc_begin_micros;

        {
            struct PositionCtrl{
                real_t kp;
                real_t ki;
                real_t kd;
                real_t ks;

                real_t kp_clamp;
                real_t intergal;
                real_t ki_clamp;
                real_t kd_enable_speed_threshold;//minimal speed for activation kd
                real_t kd_clamp;
                real_t ks_enable_speed_threshold; // minimal speed for activation ks
                real_t ks_clamp;

                real_t target_position;
                real_t target_speed;

                real_t current_slew_rate;

                real_t last_error;

                real_t update(const real_t & measured_position, const real_t & measuresd_speed){
                    real_t error = target_position - measured_position;

                    real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);

                    if(error * last_error < 0){ // zero intergal when reach target
                        intergal = real_t(0);
                    }else{
                        intergal += error;
                    }

                    real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

                    real_t kd_contribute;
                    if(abs(measuresd_speed) > kd_enable_speed_threshold){ // enable kd only highspeed
                        kd_contribute =  CLAMP(- measuresd_speed * kp, -kp_clamp, kp_clamp);
                    }else{
                        kd_contribute = real_t(0);
                    }

                    real_t speed_error = target_speed - measuresd_speed;
                    real_t ks_contribute;
                    if(abs(speed_error) > ks_enable_speed_threshold){
                        ks_contribute = CLAMP(speed_error * ks, -ks_clamp, ks_clamp);
                    }else{
                        ks_contribute = real_t(0);
                    }

                    last_error = error;

                    current_slew_rate = kp_contribute + ki_contribute + kd_contribute + ks_contribute;
                    return current_slew_rate;
                }
            };

            static PositionCtrl positionCtrl;
        }

        {
            // struct SpeedCtrl{
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
            // };
        }
}

enum class RunStatus{
    NORMAL,
    BEEP,
    CALI,
    BORKEN
};

using DoneFlag = bool;
using InitFlag = bool;

DoneFlag cali_prog(const InitFlag & init_flag){
    enum class SubState{
        INIT,
        PRE_FORWARD,
        FORWARD,
        PRE_BACKWARD,
        BACKWARD,
        ANALYSIS,
        EXAMINE,
        STOP,
        DONE
    };

    constexpr int forwardpreturns = 15;
    constexpr int forwardturns = 100;
    constexpr int backwardpreturns = 15;
    constexpr int backwardturns = 100;
    constexpr int turnmircos = 64;
    constexpr int dur = 200;

    static SubState sub_state = SubState::INIT;
    static int cnt = -1;
    static real_t openloop_elecrad;
    static real_t openloop_position;
    static real_t openloop_elecrad_step;
    static int pole;

    if(init_flag){
        sub_state = SubState::INIT;
        cnt = 0;
    }

    auto sw_state = [&sub_state, &cnt](const SubState & new_state){
        sub_state = new_state;
        cnt = 0;
    };


    if(cnt < 0){
        return false;
    }{

        switch(sub_state){
            case SubState::INIT:
                odo.reset();
                odo.update();
                openloop_elecrad = odo.getRawLapPosition() * 50 * TAU;
                openloop_position;
                openloop_elecrad_step = real_t(TAU / turnmircos);
                pole = odo.getRawPole();
                break;

            case SubState::PRE_FORWARD:
                openloop_elecrad += openloop_elecrad_step;
                setCurrent(real_t(cali_current), openloop_elecrad);

                if(cnt > forwardpreturns * turnmircos){
                    sw_state(SubState::FORWARD);
                }
                break;
            case SubState::FORWARD:

                if(cnt % turnmircos == 0){
                    odo.update();
                    odo.addCaliPoint(real_t(openloop_position), pole);
                    openloop_position += 0.02;
                    pole++;
                }

                if(cnt > forwardturns * turnmircos){
                    sw_state(SubState::DONE);
                }
                break;

            case SubState::PRE_BACKWARD:
                break;
            case SubState::BACKWARD:
                break;
            case SubState::STOP:
                setCurrent(real_t(), real_t());
                break;
            case SubState::ANALYSIS:
                odo.runCaliAnalysis();
                break;
            case SubState::EXAMINE:
                for(auto & item : odo.cali_map){
                    logger << item << endl;
                }
                break;
            case SubState::DONE:
                return true;
                break;
            default:
                break;
        }
    }
    return false;
}

void selfcheck_prog(){

}
void tone_prog{

};


void stepper_test(){

    uart1.init(115200 * 4);

    logger.setEps(4);

    timer1.init(4096, 1, Timer::TimerMode::CenterAlignedDownTrig);
    timer1.enableArrSync();
    // forwardChannelA.enableSync();
    // forwardChannelB.enableSync();
    // backwardChannelA.enableSync();
    // backwardChannelB.enableSync();

    // forwardChannelA.setPolarity(false);
    // forwardChannelB.setPolarity(false);
    // backwardChannelA.setPolarity(false);
    // backwardChannelB.setPolarity(false);

    // forwardChannelA.init();
    // forwardChannelB.init();
    // backwardChannelA.init();
    // backwardChannelB.init();

    timer3.init(144000);
    timer3.enableArrSync();

    // verfChannelA.enableSync();
    // verfChannelB.enableSync();
    // verfChannelA.setPolarity(true);
    // verfChannelB.setPolarity(true);

    // verfChannelA.init();
    // verfChannelB.init();

    svpwm.init();

    coilA.setClamp(real_t(1));
    coilB.setClamp(real_t(1));

    coilA.init();
    coilB.init();

    spi1.init(18000000);
    spi1.bindCsPin(portA[15], 0);



    odo.init();

    adc1.init(
        {},{
            AdcChannelConfig{.channel = AdcChannels::CH3, .sample_cycles = AdcSampleCycles::T71_5},
            AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T71_5}
        });
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    // timer3[4] = 0;
    adc1.setPga(AdcOnChip::Pga::X64);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    TIM3->CH4CVR = TIM3->ATRLR >> 1;
    adc1.enableCont();
    adc1.enableScan();
    adc1.enableAutoInject();

    auto & bled = portC[13];
    bled.OutPP();


    if(false){
        constexpr int turnmircos = 64;
        constexpr int dur = 600;

        odo.reset();
        logger.println("testing coil..");

        Coil2PConcept * coil = nullptr;

        for(uint8_t coil_index = 0; coil_index < 2; coil_index++){

            switch(coil_index){
                default:
                case 0:
                    coil = &coilA;
                    break;
                case 1:
                    coil = &coilB;
                    break;
            }

            odo.update();
            real_t min_pos = odo.getPosition();
            real_t max_pos = min_pos;

            logger.println("Coil", coil_index, " test started");

            coilA.setDuty(real_t(0));
            coilB.setDuty(real_t(0));
            delay(10);

            for(int i = 0; i < turnmircos; i++){
                coil->setDuty(cos(i * real_t((PI / turnmircos))));
                delayMicroseconds(dur);

                odo.update();
                real_t current_pos = odo.getPosition();

                min_pos = MIN(current_pos, min_pos);
                max_pos = MAX(current_pos, max_pos);
            }

            coil->setDuty(real_t(0));

            real_t abs_diff_pos = ABS(max_pos - min_pos);
            if(abs_diff_pos < real_t(0.005)){

                logger.println("Coil", coil_index, " not connected, Please Check");
                // logger.println("MAX diff", abs_diff_pos);
            }else{
                logger.println("Coil", coil_index, " connected well");
                // logger.println("MAX diff", abs_diff_pos);
            }
        odo.inverse(true);
        }
    }
    odo.inverse();
    if(false){
        constexpr float tone_current = 0.2;

        logger.println("tone");

        struct Tone{
            uint32_t freq_hz;
            uint32_t sustain_ms;
        };

        constexpr int freq_G4 = 392;
        constexpr int freq_A4 = 440;
        constexpr int freq_B4 = 494;
        constexpr int freq_C5 = 523;
        constexpr int freq_D5 = 587;
        constexpr int freq_E5 = 659;
        constexpr int freq_F5 = 698;
        constexpr int freq_G5 = 784;

        Tone tones[] = {
            {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            
            {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
            {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
            {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
            
            {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
            {.freq_hz = freq_C5,.sustain_ms = 100},  // 1
            {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
            {.freq_hz = freq_G4,.sustain_ms = 100}   // 5
        };
        for(auto & tone : tones){
            uint32_t play_begin_ms = millis();
            uint32_t tone_period_us = 1000000 / tone.freq_hz;
            tone_period_us /= 2;
            while(millis() - play_begin_ms < tone.sustain_ms){
                // svpwm.setDQCurrent(Vector2(real_t(tone_current), real_t(0)), real_t());
                setCurrent(tone_current, real_t(0));
                delayMicroseconds(tone_period_us);
                setCurrent(tone_current, real_t(PI/2));
                // svpwm.setDQCurrent(Vector2(real_t(0), real_t(tone_current)), real_t());
                delayMicroseconds(tone_period_us);
            }
        }
    }


    if(true){
        odo.reset();
        // logger.println("Cali And Direction Identify..");

        constexpr int forwardpreturns = 15;
        constexpr int forwardturns = 100;
        constexpr int backwardpreturns = 15;
        constexpr int backwardturns = 100;
        constexpr int turnmircos = 64;
        constexpr int dur = 200;



        odo.update();
        real_t openloop_elecrad = odo.getRawLapPosition() * 50 * TAU;

        real_t openloop_position;
        // real_t openloop_position_step = real_t(TAU / turnmircos);
        real_t openloop_elecrad_step = real_t(TAU / turnmircos);
        int pole = odo.getRawPole();

        for(int i = -forwardpreturns * turnmircos;i < forwardturns * turnmircos; i++){
            odo.update();
            // real_t openloop_position = openloop_elecrad / (50 * TAU);
            if (i % turnmircos == 0){
                if(i >= 0){
                    if(i == 0){openloop_position = odo.getRawLapPosition();}
                        // logger.println(openloop_position, odo.getRawLapPosition(),odo.warp_around_zero( openloop_position - odo.getRawLapPosition()));

                    odo.addCaliPoint(real_t(openloop_position), pole);
                }
                openloop_position += 0.02;
                pole++;
            }
            openloop_elecrad += openloop_elecrad_step;
            // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), openloop_elecrad);
            setCurrent(real_t(cali_current), openloop_elecrad);
            // logger.println(openloop_position, odo.getRawLapPosition(), openloop_position - odo.getRawLapPosition());
            delayMicroseconds(dur);
        }

        delay(20);
        // pole -= backwardpreturns;
        for(int i = -backwardpreturns * turnmircos;i < backwardturns * turnmircos; i++){
            odo.update();
            if (i % turnmircos == 0){
                if(i >= 0){
                    odo.addCaliPoint(real_t(openloop_position), pole);
                                // logger.println(openloop_position, odo.getRawLapPosition(),odo.warp_around_zero( openloop_position - odo.getRawLapPosition()));

                }
                openloop_position -= 0.02;
                pole--;
            }
            openloop_elecrad -= openloop_elecrad_step;

            setCurrent(real_t(cali_current), openloop_elecrad);
            delayMicroseconds(dur);
        }


        // svpwm.setDQCurrent(Vector2(), real_t());
        setCurrent(real_t(), real_t());

        odo.runCaliAnalysis();
        for(auto & item : odo.cali_map){
            logger << item << endl;
        }
        // while(true);
    }



    if(false){
        // odo.reset();
        // logger.println("Cali And Direction Identify..");

        // constexpr int forwardpreturns = 2;
        // constexpr int forwardturns = 5;
        // constexpr int backwardpreturns = 2;
        // constexpr int backwardturns = 5;
        // constexpr int turnmircos = 64;
        // constexpr int dur = 600;
        // constexpr float cali_current = 0.6;

        // real_t elecrad = real_t(0);
        // real_t elecrad_step = real_t(TAU / turnmircos);
        // real_t percent = real_t(1.0 / (forwardturns + backwardturns));

        // struct{
        //     public:

        //     real_t start_pos;
        //     real_t after_forward_pos;
        //     real_t after_backward_pos;

        //     const real_t max_diff_tolerance = real_t(0.05);

        //     bool isForwardBackwardFluent(const int & forward_total_turns, const int & backward_total_turns){
        //         real_t forward_trip = after_forward_pos - start_pos;
        //         real_t backward_trip = after_backward_pos - after_forward_pos;

        //         if(ABS(forward_trip) < max_diff_tolerance || ABS(backward_trip) < max_diff_tolerance){
        //             logger.println("Stuck happend while froward and backward");
        //             logger.println("Forward trip:", forward_trip);
        //             logger.println("Backward trip:", backward_trip);
        //             return false;
        //         }

        //         if(ABS(ABS(forward_trip) * backward_total_turns - ABS(backward_trip) * forward_total_turns)
        //              > max_diff_tolerance * forward_total_turns * backward_total_turns){
        //             logger.println("Bump happend while froward and backward");
        //             logger.println("Forward trip:", forward_trip);
        //             logger.println("Backward trip:", backward_trip);
        //             logger.println(ABS(ABS(forward_trip) * backward_total_turns - ABS(backward_trip) * forward_total_turns));
        //             logger.println(max_diff_tolerance * forward_total_turns * backward_total_turns);
        //             return false;
        //         }

        //         if(forward_trip * backward_trip >= 0){
        //             logger.println("Direction still while forward and backward");
        //             return false;
        //         }

        //         return true;
        //     }

        //     bool needToInverse(){
        //         real_t forward_trip = after_forward_pos - start_pos;
        //         // real_t backward_trip = after_backward_pos - after_forward_pos;
        //         return forward_trip < 0;
        //     }
        // }pos_trend;

        // // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
        // setCurrent(real_t(cali_current), elecrad);
        // delay(10);
        // odo.update();
        // pos_trend.start_pos = odo.getPosition();
        // delay(10);

        // for(int i = -forwardpreturns * turnmircos;i < forwardturns * turnmircos; i++){

        //     if(i % turnmircos == 0 && i >= 0){//measureable
        //         odo.locateElecrad(percent);
        //     }

        //     elecrad += elecrad_step;
        //     // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
        //     setCurrent(real_t(cali_current), elecrad);
        //     delayMicroseconds(dur);
        // }

        // delay(10);
        // odo.update();
        // pos_trend.after_forward_pos = odo.getPosition();
        // delay(10);

        // for(int i = -backwardpreturns * turnmircos; i < backwardturns * turnmircos; i++){

        //     if(i % turnmircos == 0 && i >= 0){//measureable
        //         odo.locateElecrad(percent);
        //     }

        //     elecrad -= elecrad_step;
        //     // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), elecrad);
        //     setCurrent(real_t(cali_current), elecrad);
        //     delayMicroseconds(dur);
        // }

        // delay(10);
        // odo.update();
        // pos_trend.after_backward_pos = odo.getPosition();
        // delay(10);

        // logger.println("Cali Done..");
        // logger.println("elecrad offset is", odo.getElecRadOffset());
        // pos_trend.isForwardBackwardFluent(forwardpreturns + forwardturns, backwardpreturns + backwardturns);

    }

    // svpwm.setDQCurrent(Vector2(real_t(cali_current), real_t(0)), real_t(0));
    setCurrent(real_t(cali_current), real_t());
    delay(200);
    // odo.locateElecrad();
    odo.locateRelatively(real_t(0));
    setCurrent(real_t(), real_t());


    timer4.init(foc_freq);
    timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));



    // uint32_t foc_begin_micros;
    // uint32_t foc_end_micros;

    timer4.bindCb(Timer::IT::Update, std::function<void(void)>(run));

    delay(200);
    // motor.enable();
    // motor.trackPos(real_t(0));
    // motor.setMaxCurrent(real_t(0.45));

    logger.setEps(4);

    while(true){
        // real_t total = real_t(3);
        // static real_t freq = real_t(10);
        // static real_t freq_dir = real_t(1);
        // const real_t freq_delta = real_t(20);
        // if(freq > real_t(300)) freq_dir = real_t(-1);
        // else if(freq < real_t(4)) freq_dir = real_t(1);
        // static real_t last_t = t;
        // real_t delta = (t - last_t);
        // freq += delta * freq_dir * freq_delta;
        // last_t = t;
        // static real_t ang = real_t(0);
        // ang += freq * delta;
        // real_t target = (total / freq) * sin(ang);

        // target_pos = sign(frac(t) - 0.5);
        // target_pos = sin(t);
        // logger.println(odo.getPosition(), est_pos, est_speed, ctrl.elecrad_offset_output, odo.getRawLapPosition(), odo.getLapPosition());
        // logger.println(est_speed, targets.curr, run_elecrad_addition);
        // , est_speed, t, odo.getElecRad(), openloop_elecrad);

        static String temp_str = "";

        if(logger.available()){
            char chr = logger.read();
            if(chr == '\n'){
                temp_str.trim();
                // logger.println(temp_str);
                if(temp_str.length()) parseLine(temp_str);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }
        Sys::Clock::reCalculateTime();
    }
    while(true);
}
};


namespace SpreadCycle{

constexpr float coil_inductor_mH = 0.83;
constexpr float coil_resistor_Ohm = 2.24;
constexpr float busbar_voltage = 9.0;

constexpr float c1 = busbar_voltage / coil_inductor_mH;
constexpr float coil_current = 0.23;
constexpr float c2 = - coil_current * coil_resistor_Ohm / coil_inductor_mH;

PwmChannel coil_pwm_p(timer1[3]);
PwmChannel coil_pwm_n(timer1[4]);

real_t dual_duty[2] = {
    real_t(0.3),
    real_t(0.14)
};

uint16_t dual_cvr[2] = {699,2299};

real_t chopper_current = real_t(0.2);

uint32_t chopper_run_cnt = 0;
struct {
    bool odd_is_forward:1;
    bool even_is_forward:1;
}chopper_run_mode;

void chopper_run(){

    if(chopper_run_cnt == 0){
        chopper_run_mode.odd_is_forward = true;
        chopper_run_mode.even_is_forward = false;
        chopper_run_cnt = 1;
        return;
    }

    chopper_run_cnt++;
    bool cycle_is_odd = chopper_run_cnt % 2;

    bool cycle_is_forward = cycle_is_odd ? chopper_run_mode.odd_is_forward : chopper_run_mode.even_is_forward;
    // bool cycle_is_forward = cycle_is_odd;

    // int pwm_duty_p = cycle_is_forward ? 0 : dual_cvr[cycle_is_odd];
    // int pwm_duty_n = cycle_is_forward ? dual_cvr[cycle_is_odd] : 0;
    int pwm_duty = dual_cvr[cycle_is_odd];
    // int backward_duty = dual_cvr[1];
    if(cycle_is_forward){
        timer1[3] = pwm_duty;
        timer1[4] = 0;
    }else{
        timer1[3] = 0;
        timer1[4] = pwm_duty;
    }

}


void chopper_test(){
    uart1.init(115200 * 8, Uart::Mode::TxRx);
    Printer & logger = uart1;
    logger.setSpace(",");
    logger.setEps(4);

    timer1.init(36000);
    timer1.enableArrSync();
    // timer1.enableCvrSync();
    timer1[3].enableSync();
    timer1[4].enableSync();
    timer1[3].setPolarity(false);
    timer1[4].setPolarity(false);

    timer1[3].init();
    timer1[4].init();

    coil_pwm_p.setClamp(real_t(0.4));
    coil_pwm_n.setClamp(real_t(0.4));

    timer3.init(36000);
    timer3[2].init();
    timer3[3].init();
    timer3[2].setPolarity(true);
    timer3[3].setPolarity(true);

    timer3[2] = real_t(0.4);
    timer3[3] = real_t(0.4);



    timer1.bindCb(Timer::IT::Update, std::function<void(void)>(chopper_run));
    timer1.enableIt(Timer::IT::Update, NvicPriority(0, 0));

    while(true){
        logger.println(int(timer1[3]), int(timer1[4]));
    }
}



};
int main(){
    Sys::Misc::prework();
    // stepper_app();
    // stepper_app_new();
    StepperTest::stepper_test();
    // chassis_app();
    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    osc_test();
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
