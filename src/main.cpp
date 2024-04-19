#include "misc.h"
#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;
#include "src/fwwb_compents/fwwb_inc.h"

Gpio i2cScl = Gpio(GPIOB, (Pin)I2C_SW_SCL);
Gpio i2cSda = Gpio(GPIOB, (Pin)I2C_SW_SDA);

constexpr uint32_t SPI1_BaudRate = (144000000/32);
constexpr uint32_t SPI2_BaudRate = (144000000/8);

#define I2C_BaudRate 400000


// Gpio i2sSck = Gpio(GPIOB, I2S_SW_SCK);
// Gpio i2sSda = Gpio(GPIOB, I2S_SW_SDA);
// Gpio i2sWs = Gpio(GPIOB, I2S_SW_WS);

// I2cSw i2cSw(i2cScl, i2cSda);
// I2sSw i2sSw(i2sSck, i2sSda, i2sWs);

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
// I2sDrv i2sDrvTm = I2sDrv(i2sSw);
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
// TM8211 extern_dac(i2sDrvTm);
// W25QXX extern_flash(spiDrvFlash);

// AS5600 mag_sensor(i2cDrvAS);
// QMC5883L earth_sensor(i2cDrvQm);
// BMP280 prs_sensor(i2cDrvBm);
// MT6701 mt_sensor(i2cDrvMt);

extern "C" void TimBase_IRQHandler(void) __interrupt;

RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

real_t delta = real_t(0);
real_t fps = real_t(0);


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


constexpr uint16_t pwm_arr = 144000000/12000 - 1;
constexpr uint16_t ir_arr = 144000000/38000 - 1;

#define narrow 12
#define wide 12
void sendCode(Gpio & gpio,const bool & state){
    if(state){
        gpio.set();
        __nopn(120);
        gpio.clr();
        __nopn(32);
    }else{
        gpio.set();
        __nopn(32);
        gpio.clr();
        __nopn(120);
    }
}

void sendReset(Gpio & gpio){
    gpio.clr();
    delayMicroseconds(60);
    gpio.set();
}

enum class posModes:uint8_t{
    Lap, Continuous
};


using Sys::t;

static Printer & logger = uart2;








class Flash{
protected:


    using Page = uint32_t;
    using PageRange = Range_t<Page>;
    using Address = uint32_t;
    using AddressRange = Range_t<Address>;

    static constexpr Page page_size = 256;
    static constexpr Address base_address = 0x08000000;

    Page page_count;
    PageRange page_range;

    uint32_t pre_clock;

    void settleClock(){
        pre_clock = Sys::Clock::getAHBFreq();
        Sys::Clock::setAHBFreq(72000000);
    }

    void resetClock(){
        Sys::Clock::setAHBFreq(pre_clock);
    }

    Page pageWarp(const int & index){
        return index > 0 ? index : page_count + index;
    }

    AddressRange getAddressRange(){
        return (page_range * page_size).shift(base_address);
    }
public:
    Flash(int _page_begin):Flash(_page_begin, Sys::Chip::getFlashSize() / page_size){;}
    Flash(int _page_begin, int _page_end):
            page_count(Sys::Chip::getFlashSize() / page_size),
            page_range(PageRange(Page(0),Sys::Chip::getFlashSize() / page_size)
            .intersection(PageRange(pageWarp(_page_begin), pageWarp(_page_end)))){;}

    ~Flash(){exit();}

    void init(){
        settleClock();
        Systick_Init();
        delay(10);
    }

    template<typename T>
    volatile bool store(const T & data){

        auto NbrOfPage = page_range.get_length(); //计算要擦除多少页
        Address PAGE_WRITE_START_ADDR = getAddressRange().start;
        Address PAGE_WRITE_END_ADDR = getAddressRange().end;

        logger.println(PAGE_WRITE_START_ADDR);
        FLASH_Status FLASHStatus =  FLASH_COMPLETE;

        __disable_irq();
        FLASH_Unlock_Fast();

        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_WRPRTERR);

        // logger.println("begin erase", PAGE_WRITE_START_ADDR);

        // uint32_t len = sizeof(data);
        // for(Page EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++){
            // Address op_address = PAGE_WRITE_START_ADDR + (page_size * EraseCounter)
        uint32_t buf[page_size / sizeof(uint32_t)] = {0};
        // memcpy(&buf, &data, sizeof(T));
        for(size_t i = 0; i < sizeof(T); i++){
            ((uint8_t *)buf)[i] = ((uint8_t *)&data)[i];
            // logger.println(i,((uint8_t *)&data)[i] );
        }

        FLASH_ErasePage_Fast(PAGE_WRITE_START_ADDR);
            // if()
            // FLASH_ProgramPage_Fast(PAGE_WRITE_START_ADDR, (uint32_t *)((uint8_t *)&data + page_size * EraseCounter));
        FLASH_ProgramPage_Fast(PAGE_WRITE_START_ADDR, (uint32_t *)&buf);
        // }

        FLASH_Lock_Fast();
        __enable_irq();

        Address address = PAGE_WRITE_START_ADDR;
        bool MemoryProgramStatus = true;

        uint32_t i = 0;
        while((address < PAGE_WRITE_END_ADDR) && (MemoryProgramStatus != false)){
            auto read_data = (*(__IO uint32_t*) address);
            auto examine_data = buf[i];
            if(read_data != examine_data){
                MemoryProgramStatus = false;
            }
            // logger.println("vait", address, read_data, examine_data);
            address += 4;
            i++;
        }
        // logger.println("vait", MemoryProgramStatus);
        return MemoryProgramStatus;
    };

    template<typename T>
    volatile void load(T & data){
        Address PAGE_WRITE_START_ADDR = getAddressRange().start;
        Address PAGE_WRITE_END_ADDR = getAddressRange().end;
        Address address = PAGE_WRITE_START_ADDR;
        // uint32_t i = 0;
        for(size_t i = 0; i<sizeof(T);i++){
            auto read_data = (*(__IO uint8_t*)(PAGE_WRITE_START_ADDR + i));
            *((volatile uint8_t*)&data + i) = read_data;
            // logger.println("read", address, read_data);
            // address += 1;
            // i++;
        }
    };

    void exit(){
        resetClock();
    }
};

// void testFlash(){


//     Flash flash(-1);
//     flash.init();
//     Sys::Misc::prework();

//     uart2.init(115200);

//     logger.setSpace(" ");
//     logger.setRadix(16);

//     // logger.println("Flash Size:", Sys::Chip::getFlashSize());
//     // logger.println("Sys Clock:", Sys::Clock::getSystemFreq());
//     // logger.println("AHB Clock:", Sys::Clock::getAHBFreq());
//     // logger.println("APB1 Clock:", Sys::Clock::getAPB1Freq());
//     // logger.println("APB2 Clock:", Sys::Clock::getAPB2Freq());
//     struct Temp{
//         uint8_t data[4] = {0,1,2,3};
//         char name[8] = "Rstr1aN";
//         real_t value = real_t(0.1);
//         uint8_t crc = 0x08;
//     };
//     Temp temp;

//     logger.println("单纯的妇女而会计法v你rfwevdnoln");
//     // bkp.init();
//     // logger.println("bkp_data", bkp.readData(1));

//     // if(bkp.readData(1) % 2){
//     //     // flash.load(temp);
//     //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     //     logger.println(temp.name);
//     //     logger.println(temp.value);
//     //     logger.println(temp.crc);

//     // }
//     flash.load(temp);
//     if(temp.data[0] == 0x39){
//         logger.println("need to store new");
//         logger.println("new data is");
//         Temp new_temp = Temp();
//         logger.println(new_temp.data[0], new_temp.data[1], new_temp.data[2], new_temp.data[3]);
//         flash.store(new_temp);
//         flash.load(temp);
//         logger.println("new store done");
//     }

//     logger.println("data loaded");
//     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     logger.println(temp.name);
//     logger.println(temp.value);
//     logger.println(temp.crc);
//     bkp.init();
//     bkp.writeData(1, bkp.readData(1) + 1);
//     temp.crc = bkp.readData(1);
//     flash.store(temp);
//     // flash.load(temp);
//     // logger.println();
//     // if(temp.data[0] == 0xE3 || temp.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
//     //     logger.println(temp.data[0]);

//     //     temp.data[3] = bkp.readData(1);
//     //     temp.name[2] = 'k';
//     //     // temp.value = real_t(0.2);
//     //     temp.crc = 10;

//     //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     //     logger.println(temp.name);
//     //     logger.println(temp.value);
//     //     logger.println(temp.crc);
//     //     flash.store(temp);
//     //     flash.load(temp);
//     // }else{
//     //     logger.println("suss");
//     //         logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     // logger.println(temp.name);
//     // logger.println(temp.value);
//     // logger.println(temp.crc);
//     // }


//     // }
//     // logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
//     // logger.println(temp.name);
//     // logger.println(temp.value);
//     // logger.println(temp.crc);

//     // 





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
class Estimmator{
// protected:
public:

    using PositionObserver =  LinearObersver_t<real_t, real_t>;
    Odometer & instance;
    // real_t lastPosition;
    PositionObserver positionObserver;
    LowpassFilter_t<real_t, real_t>speed_lpf = LowpassFilter_t<real_t, real_t>(30.0);
    real_t speed;
    uint32_t dur;
    uint32_t cnt = 0;
public:
    Estimmator(Odometer & _instance, const int & _dur = 20):
        instance(_instance),
        dur(_dur){;}

    virtual void init(){
        instance.init();
    }
    void update(){
        instance.update();

        cnt++;
        if(cnt == dur){
            cnt = 0;
            positionObserver.update(instance.getPosition(), Sys::Clock::getCurrentSeconds());
        }
        speed = speed_lpf.update(positionObserver.getDerivative(), t);
    }

    real_t getPosition(){
        // return instance.getPosition();
        return positionObserver.predict(Sys::Clock::getCurrentSeconds());
    }

    real_t getSpeed(){
        return speed;
    }

    real_t getDirection(){
        return sign(getSpeed());
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
        case 'P':
            if(argv.size() == 0) goto syntax_error;
            pos2curr_pid.kp = real_t(argv[0]);
            break;
        case 'I':
            if(argv.size() == 0) goto syntax_error;
            pos2curr_pid.ki = real_t(argv[0]);
            break;
        case 'D':
            if(argv.size() == 0) goto syntax_error;
            pos2curr_pid.kd = real_t(argv[0]);
            break;
        case 'O':
            if(argv.size() == 0) goto syntax_error;
            omega = real_t(argv[0]);
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


void pmdc_test(){
    uart2.init(115200 * 8, Uart::Mode::TxRx);
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
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T28_5}
        },
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T71_5}
        });
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    // timer3[4] = 0;
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    TIM3->CH4CVR = TIM3->ATRLR >> 1;
    adc1.enableCont();
    adc1.enableScan();
    adc1.enableAutoInject();

    // adc1.enableRightAlign(false);
    real_t motor_curr;
    // adc1.start();
    // adc1.swStartRegular();
    // adc1.swStartInjected();

    est.init();
    // trigGpioA.InPullUP();
    // trigGpioB.InPullUP();
    // auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(0, 0), ExtiChannel::Trigger::RisingFalling);
    // auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 4), ExtiChannel::Trigger::RisingFalling);

    // int16_t cnt = 0;
    // trigExtiCHA.bindCb([&cnt, &trigGpioA, &trigGpioB](){
    //     if(bool(trigGpioA)){
    //         if(bool(trigGpioB)) cnt--;
    //         else cnt++;
    //     }else{
    //         if(bool(trigGpioB)) cnt++;
    //         else cnt--;
    //     }
    // });

    // trigExtiCHB.bindCb([&cnt,&trigGpioA,  &trigGpioB](){
    //     if(bool(trigGpioB)){
    //         if(bool(trigGpioA)) cnt++;
    //         else cnt--;
    //     }else{
    //         if(bool(trigGpioA)) cnt--;
    //         else cnt++;
    //     }
    // });


    // trigExtiCHA.init();
    // trigExtiCHB.init();

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
    timer4.bindCb(Timer::IT::Update, [&](){
        t_watch.set();
        est.update();
        // duty = CLAMP(duty +), -1, 1);
        // motor.setDuty( pos_pid.update(target, odo.getPosition(), est.getSpeed()));
        target_curr = pos2curr_pid.update(target_pos, odo.getPosition(), est.getSpeed());
        real_t sense_uni;
        u16_to_uni(ADC1->IDATAR1 << 4, sense_uni);
        constexpr float sense_scale = (1000.0 / 680.0) * 3.3;
        motor_curr_temp = sign(duty) * sense_uni * sense_scale;
        // motor_curr = lpf.update(motor_curr_temp, t);4
        motor_curr = lpf.forward(motor_curr_temp, real_t(1.0 / closeloop_freq));

        duty = curr_pid.update(target_curr, motor_curr);
        motor = duty;
        t_watch.clr();
    });

    timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
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
        logger.println(target_pos, odo.getPosition(), est.getSpeed(), TIM3->CH2CVR);
        // motor.setDuty(sin(t));
        // logger.println(ADC1->IDATAR1);
        // delay(2);

        
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
}


class Opa{
protected:
    uint8_t opa_num;
    uint8_t psel;
    uint8_t nsel;
    uint8_t osel;

    Gpio getPosPin(){
        switch(opa_num){
            case 1:
                switch(psel){
                    case 0:
                        return Gpio(OPA1_P0_Port, (Pin)OPA1_P0_Pin);
                    case 1:
                        return Gpio(OPA1_P1_Port, (Pin)OPA1_P1_Pin);
                    default:
                        break;
                }
                break;
            case 2:
                switch(psel){
                    case 0:
                        return Gpio(OPA2_P0_Port, (Pin)OPA2_P0_Pin);
                    case 1:
                        return Gpio(OPA2_P1_Port, (Pin)OPA2_P1_Pin);
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        return Gpio(GPIOA, Pin::None);
    };

    Gpio getNegPin(){
        switch(opa_num){
            case 1:
                switch(psel){
                    case 0:
                        return Gpio(OPA1_N0_Port, (Pin)OPA1_N0_Pin);
                    case 1:
                        return Gpio(OPA1_N1_Port, (Pin)OPA1_N1_Pin);
                    default:
                        break;
                }
                break;
            case 2:
                switch(psel){
                    case 0:
                        return Gpio(OPA2_N0_Port, (Pin)OPA2_N0_Pin);
                    case 1:
                        return Gpio(OPA2_N1_Port, (Pin)OPA2_N1_Pin);
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        return Gpio(GPIOA, Pin::None);
    };

    // Gpio getOutPin(){
    //     switch(opa_num){
    //         case 1:
    //             switch(psel){
    //                 case 0:
    //                     return Gpio(OPA1_O0_Port, (Pin)OPA1_O0_Pin);
    //                 case 1:
    //                     return Gpio(OPA1_O1_Port, (Pin)OPA1_O1_Pin);
    //                 default:
    //                     break;
    //             }
    //             break;
    //         case 2:
    //             switch(psel){
    //                 case 0:
    //                     return Gpio(OPA2_O0_Port, (Pin)OPA2_O0_Pin);
    //                 case 1:
    //                     return Gpio(OPA2_O1_Port, (Pin)OPA2_O1_Pin);
    //                 default:
    //                     break;
    //             }
    //             break;
    //         default:
    //             break;
    //     }
    //     return Gpio(GPIOA, Pin::None);
    // };
public:
    Opa(const uint8_t & _opa_num):opa_num( _opa_num ){;}

    void init( const uint8_t & sel){
        getNegPin().InAnalog();
        getPosPin().InAnalog();
        OPA_InitTypeDef OPA_InitStructure;
        OPA_InitStructure.OPA_NUM = CLAMP((OPA_Num_TypeDef)(OPA1 + (opa_num - 1)), OPA1, OPA4);
        OPA_InitStructure.PSEL = (OPA_PSEL_TypeDef)sel;
        OPA_InitStructure.NSEL = (OPA_NSEL_TypeDef)sel;
        OPA_InitStructure.Mode = (OPA_Mode_TypeDef)sel;
        OPA_Init(&OPA_InitStructure);
        OPA_Cmd(OPA_InitStructure.OPA_NUM, ENABLE);
    }
};

Opa opa1(1);
Opa opa2(2);

template<typename T>
struct targAndMeasurePair_t{
    T target;
    T measure;
};

using targAndMeasurePair = targAndMeasurePair_t<real_t>;


struct buckRuntimeValues{
    targAndMeasurePair curr;
    targAndMeasurePair volt;
};


// class Test:pub
void buck_test(){
    uart2.init(115200 * 8, Uart::Mode::TxRx);
    logger.setSpace(",");
    logger.setEps(4);

    timer1.initBdtr();
    timer1.init(32000, Timer::TimerMode::Up, false);
    auto & ch = timer1.ch(1);
    auto & chn = timer1.chn(1);

    ch.setIdleState(true);
    chn.setIdleState(true);
    chn.init();

    auto buck_pwm = PwmChannel(ch);
    buck_pwm.setClamp(real_t(0.1), real_t(0.9));
    buck_pwm.init();

    adc1.init(
        {
            AdcChannelConfig{.channel = AdcChannels::CH0, .sample_cycles = AdcSampleCycles::T28_5}
        },
        {
            AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T239_5}
            // AdcChannelConfig{.channel = AdcChannels::CH1, .sample_cycles = AdcSampleCycles::T239_5},
        });

    static constexpr int buck_freq = 1000;
    timer3.init(buck_freq);

    real_t adc_fl1, adc_fl2, adc_out, duty;
    buckRuntimeValues buck_rv;
    LowpassFilter_t<real_t, real_t> lpf(50);
    LowpassFilter_t<real_t, real_t> lpf2(30);

    timer3.bindCb(Timer::IT::CC4, [&](){
        duty = real_t(0.3) + 0.14 * sin(4 * TAU * t);
        // duty = real_t(0.5);
        u16_to_uni(ADC1->IDATAR1<<4, adc_out);
        adc_fl1 = lpf.forward(adc_out, real_t(1.0 / buck_freq));
        adc_fl2 = lpf2.forward(adc_fl1, real_t(1.0 / buck_freq));

        buck_rv.curr.measure = real_t(adc_fl2);
        buck_pwm = duty;
    });
    timer3.enableIt(Timer::IT::CC4, NvicPriority(0, 0));

    // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
    // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
    adc1.setPga(AdcOnChip::Pga::X64);
    // adc1.enableCont();
    // adc1.setPga(AdcOnChip::Pga::X4);
    // adc1.enableScan();
    adc1.enableAutoInject(); // must be enabled for ext.inj
    // adc1.swStartRegular();
    adc1.swStartInjected();
    opa2.init(1);

    timer1.enable();

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

        logger.println(duty, adc_out, adc_fl1, adc_fl2);
        Sys::Clock::reCalculateTime();
    }
}
int main(){
    Sys::Misc::prework();
    // stepper_app();
    // stepper_app_new();
    // chassis_app();
    // modem_app();
    // test_app();
    // pmdc_test();
    buck_test();

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
    // extern_dac.setDistort(5);
    // extern_dac.setRail(real_t(1), real_t(4));

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
