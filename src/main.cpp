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


int main(){


    Flash flash(-2, -1);
    flash.init();
    Sys::Misc::prework();

    uart2.init(115200);

    logger.setSpace(" ");
    logger.setRadix(16);

    // logger.println("Flash Size:", Sys::Chip::getFlashSize());
    // logger.println("Sys Clock:", Sys::Clock::getSystemFreq());
    // logger.println("AHB Clock:", Sys::Clock::getAHBFreq());
    // logger.println("APB1 Clock:", Sys::Clock::getAPB1Freq());
    // logger.println("APB2 Clock:", Sys::Clock::getAPB2Freq());
    struct Temp{
        uint8_t data[4] = {0,1,2,3};
        char name[8] = "Rstr1aN";
        real_t value = real_t(0.1);
        uint8_t crc = 0x08;
    };
    Temp temp;

    
    // bkp.init();
    // logger.println("bkp_data", bkp.readData(1));

    // if(bkp.readData(1) % 2){
    //     // flash.load(temp);
    //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    //     logger.println(temp.name);
    //     logger.println(temp.value);
    //     logger.println(temp.crc);

    // }
    flash.load(temp);
    if(temp.data[0] == 0x39){
        logger.println("need to store new");
        logger.println("new data is");
        Temp new_temp = Temp();
        logger.println(new_temp.data[0], new_temp.data[1], new_temp.data[2], new_temp.data[3]);
        flash.store(new_temp);
        flash.load(temp);
        logger.println("new store done");
    }

    logger.println("data loaded");
    logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    logger.println(temp.name);
    logger.println(temp.value);
    logger.println(temp.crc);
    bkp.init();
    bkp.writeData(1, bkp.readData(1) + 1);
    temp.crc = bkp.readData(1);
    flash.store(temp);
    // flash.load(temp);
    // logger.println();
    // if(temp.data[0] == 0xE3 || temp.data[0] == 0x39 || (bkp.readData(1) & 0b11) == 0){
    //     logger.println(temp.data[0]);

    //     temp.data[3] = bkp.readData(1);
    //     temp.name[2] = 'k';
    //     // temp.value = real_t(0.2);
    //     temp.crc = 10;

    //     logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    //     logger.println(temp.name);
    //     logger.println(temp.value);
    //     logger.println(temp.crc);
    //     flash.store(temp);
    //     flash.load(temp);
    // }else{
    //     logger.println("suss");
    //         logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.println(temp.name);
    // logger.println(temp.value);
    // logger.println(temp.crc);
    // }


    // }
    // logger.println(temp.data[0], temp.data[1], temp.data[2], temp.data[3]);
    // logger.println(temp.name);
    // logger.println(temp.value);
    // logger.println(temp.crc);

    // 





    // flash.load(temp);
    while(true) __WFI;
    // while(true){
        // logger.println("Hi");
        // delay(100);
    // }

    // stepper_app();
    // stepper_app_new();
    chassis_app();
    // modem_app();
    test_app();


    timer1.init(25600);

    auto tim1ch1 = timer1[1];
    tim1ch1.init();

    auto tim1ch1n = timer1[-1];
    tim1ch1n.init();

    auto tim1ch2 = timer1[2];
    tim1ch2.init();

    auto tim1ch3 = timer1[3];
    tim1ch3.init();

    auto tim1ch4 = timer1[4];
    tim1ch4.init();


    timer1.initBdtr(AdvancedTimer::LockLevel::Off, 0);
    timer1.enable();

    auto pwmCoilP = PwmChannel(tim1ch3);
    pwmCoilP.init();

    auto pwmCoilN = PwmChannel(tim1ch4);
    pwmCoilN.init();
    uart2.init(UART2_Baudrate);
    uart2.setEps(4);

    Gpio Led = Gpio(GPIOC, Pin::_13);
    Led.OutPP();
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

    Gpio sda_pin = Gpio(SPI1_MOSI_Port, (Pin)SPI1_MOSI_Pin);
    Gpio scl_pin = Gpio(SPI1_SCLK_Port, (Pin)SPI1_SCLK_Pin);
    I2cSw i2csw(scl_pin, sda_pin);
    i2csw.init(40000);
    I2cDrv i2cdrv(i2csw, 0xd0);
    MPU6050 mpu(i2cdrv);
    mpu.init();
    // Axis6 & imu = mpu;
    Gpio TrigA = Gpio(GPIOC, Pin::_14);
    Gpio TrigB = Gpio(GPIOC, Pin::_15);
    TrigA.InPullUP();
    TrigB.InPullUP();

    // Gpio a0 = Gpio(GPIOA, Pin::_0);
    // a0.InAnalog();
    // AdcChannelOnChip ac0(ADC1);

    // adc1.clearRegularQueue();
    // adc1.AddChannelToQueue(ac0);
    // adc1.init();
    // adc1.setRegularTrigger(AdcHw::RegularTrigger::SW);
    // adc1.start();

    ExtiChannel channel(TrigA, NvicPriority(2,1));

    // adc1.init();
    real_t cnt;
    auto cb = [&Led, &TrigB, &cnt](){cnt += real_t((bool(TrigB) ? 1 : -1)) / real_t(16384);};
    channel.init();
    channel.bindCb(cb);

    uart1.init(115200);
    // VtfRequest(15EXTI15_10_IRQn, 0, cb);
    // GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* GPIOA ----> EXTI_Line0 */
    // EXTI_Init(&EXTI_InitStructure);



    // auto filter = BurrFilter_t<real_t>();
    Vector3 accel;
    real_t f0 = real_t(6);
    real_t f_test = real_t(0.1);
    LowpassFilter_t<real_t, real_t> lpf(f0);
    HighpassFilter_t<real_t, real_t> hpf(f0);
    auto lob = LinearObersver_t<real_t, real_t>();
    auto lob2 = LinearObersver_t<real_t, real_t>();
    real_t x;
    real_t y;
    real_t f;
    while(true){
        real_t x = 6*sin(t * f_test * TAU);


        if(millis()%10 == 0) lob2.update(lob.update(real_t(x), t),t);

        uart2.println(x, lob2.predict(lob.predict(t),t) , lob.predict(t));
        // t = real_t(float(millis()) / float(10000));
        // Led = (millis() / 100) & 0b1;
        // if((millis() % 100) == 0){cb(); delay(1);}
    }
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
    while(true){
        // Led = !Led;
        static bool i = false;
        i = !i;
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
    }
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

    uart2.println("initialized");
    while(1){

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
    }
}
