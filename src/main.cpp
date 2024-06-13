
#include "system.hpp"
#include "hal/flash/flash.hpp"
using Sys::t;
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




// static IOStream & logger = uart2;
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

#include "drivers/Encoder/ABEncoder.hpp"
#include "drivers/Encoder/OdometerLines.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "dsp/oscillator.hpp"
#include "drivers/Dac/TM8211/tm8211.hpp"

#include <bits/stl_numeric.h>
#include <optional>

#include "hal/bus/bus_inc.h"
#include "robots/buck.hpp"
#include "robots/stepper/stepper.hpp"
#include "defines/defs.h"
#include "bus/bus_inc.h"
#include "bkp/bkp.hpp"
#include "dsp/controller/PID.hpp"
#include "dsp/filter/KalmanFilter.hpp"
#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "dsp/filter/BurrFilter.hpp"
#include "hal/exti/exti.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "dsp/filter/LowpassFilter.hpp"
#include "dsp/filter/HighpassFilter.hpp"
#include "dsp/observer/LinearObserver.hpp"
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Modem/Nec/Encoder/nec_encoder.hpp"
#include "drivers/VirtualIO/AW9523/aw9523.hpp"
// #include "MLX90640/MLX90640_API.h"
// #include "HX711/HX711.h"
// #include "TTP229/TTP229.h"
// #include "AS5600/as5600.hpp"
// #include "MT6701/mt6701.hpp"
// #include "HMC5883L/hmc5883l.hpp"
// #include "QMC5883L/qmc5883l.hpp"
// #include "BMP280/bmp280.hpp"
// #include "HX711/hx711.hpp"
// #include "timer/timer.hpp"
// #include "ST7789/st7789.hpp"
// #include "SSD13XX/ssd1306.hpp"
// #include "MPU6050/mpu6050.hpp"
// #include "SGM58031/sgm58031.hpp"
// #include "TCS34725/tcs34725.hpp"
// #include "VL53L0X/vl53l0x.hpp"
// #include "PCF8574/pcf8574.hpp"
// #include "AS5600/as5600.hpp"
// #include "TM8211/tm8211.hpp"
// #include "BH1750/bh1750.hpp"
// #include "AT24CXX/at24c32.hpp"
// #include "W25QXX/w25qxx.hpp"
// #include "LT8920/lt8920.hpp"
// #include "MA730/ma730.hpp"
// #include "ADXL345/adxl345.hpp"
// #include "gpio/gpio.hpp"
// #include "memory/flash.hpp"
// #include "LT8920/lt8920.hpp"

#include "../types/image/painter.hpp"
#include "stdlib.h"
#include "timer/timers/timer_hw.hpp"
#include "timer/pwm_channel.hpp"
#include "dsp/controller/PID.hpp"
#include "hal/timer/capture_channel.hpp"
#include "dsp/filter/BurrFilter.hpp"
// #include "adc/adcs/adc1.hpp"
#include "drivers/Actuator/Servo/PwmServo/pwm_servo.hpp"
#include "drivers/Actuator/Coil/coil.hpp"
#include "types/basis/Basis_t.hpp"
#include "types/quat/Quat_t.hpp"
#include "drivers/Wireless/Radio/HC12/HC12.hpp"
#include "gpio/port_virtual.hpp"
#include "drivers/VirtualIO/HC595/hc595.hpp"
#include "drivers/IMU/Axis6/BMI270/bmi270.hpp"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "hal/nvic/nvic.hpp"
#include "dsp/lut.hpp"
#include "algo/any/any.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "hal/crc/crc.hpp"

#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;

#include "dsp/pll.hpp"
#include "hal/opa/opa.hpp"
#include "drivers/Adc/INA226/ina226.hpp"

#define I2C_BaudRate 400000


#include "drivers/Encoder/Estimmator.hpp"
#include "drivers/Memory/EEPROM/AT24CXX/at24c02.hpp"

void testFlash(){
    IOStream & logger = uart2;
    

    Flash sto(-1);
    sto.init();
    Memory flash = sto;
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

    // logger.println("单纯的妇女而会计法v你rfwevdnoln");
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
}

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

    uart2.init(115200);
    IOStream & logger = uart2;
    logger.setEps(4);
    logger.setRadix(10);
    logger.setSpace(",");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    AT24C02 at24{I2cDrv(i2csw, AT24C02::default_id)};

    delay(200);
    at24.init();

    if(false){
        at24.store(at24.load(0) + 1, 0);
        delay(20);
        uart2.println(at24.load(0));
    }

    if(true){
        AT24C02_DEBUG("muti store begin");
        constexpr auto begin_addr = 7;
        // constexpr auto end_addr = 15;
        uint8_t data[] = {0, 1, 2, 4};
        uint8_t data2[sizeof(data)];

        Memory mem = {at24};
        mem.store(data, begin_addr);

        mem.load(data2, begin_addr);
        for(const auto & item : data2){
            logger.println("data_read", int(item));
        }
    }

    // if(false){
    //     constexpr int page_size = 8;
    //     Rangei plat = {11, 17};
    //     Rangei window = {0, 0};

    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     // AT24C02_DEBUG(plat.gird_part(17, page_size, false));
    // }

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