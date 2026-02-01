#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/default.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/spi/hw_singleton.hpp"
#include "hal/bus/i2c/soft/soft_i2c.hpp"
#include "hal/bus/uart/hw_singleton.hpp"

#include "algebra/vectors/quat.hpp"

#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define DBG_UART DEBUGGER_INST
#define SCL_PIN hal::PB<3>()
#define SDA_PIN hal::PB<5>()


#define PHY_SEL_I2C 0
#define PHY_SEL_SPI 1

// #define PHY_SEL PHY_SEL_I2C
#define PHY_SEL PHY_SEL_SPI

void bmi160_main(){
    DEBUGGER_INST.init({
        hal::USART2_REMAP_PA2_PA3,
        hal::NearestFreq(576000),
    });
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUGGER.no_fieldname(EN);

    // hal::spi1.init({18'000'000});
    hal::spi1.init({
        .remap = hal::SPI1_REMAP_PB3_PB4_PB5_PA15,
        .baudrate = hal::NearestFreq(2'000'000)
    });

    
    #if PHY_SEL == PHY_SEL_SPI
    // SPI1_MOSI_GPIO pb5
    // SPI1_MISO_GPIO pb4
    // SPI1_CS_GPIO pa15
    // SPI1_SCLK_GPIO pb3

    auto bmi_cs_pin_ = hal::PA<15>();
    bmi160::BMI160 bmi{{
        &hal::spi1, 
        hal::spi1.allocate_cs_pin(&bmi_cs_pin_).unwrap()
    }};
    #elif PHY_SEL == PHY_SEL_I2C

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;
    hal::SoftI2c i2c{&scl_pin_, &sda_pin_};
    // i2c.init(400_KHz);
    i2c.init({200_KHz});

    BMI160 bmi{{
        &i2c
    }};

    #endif
    bmi.init(Default).examine();

    auto red_led_pin_ = hal::PC<13>();
    auto blue_led_pin_ = hal::PC<14>();
    auto green_led_pin_ = hal::PC<15>();

    red_led_pin_.outpp(); 
    blue_led_pin_.outpp(); 
    green_led_pin_.outpp();
    hal::PA<7>().inana();


    while(true){
        red_led_pin_ = BoolLevel::from((clock::millis() % 200).count() > 100);
        blue_led_pin_ = BoolLevel::from((clock::millis() % 400).count() > 200);
        green_led_pin_ = BoolLevel::from((clock::millis() % 800).count() > 400);

        clock::delay(20ms);
        bmi.update().examine();
        const auto acc = bmi.read_acc().unwrap();
        const auto gyr = bmi.read_gyr().unwrap();
        // Quat gest = Quat::from_shortest_arc({0,0,1}, acc);
        // DEBUG_PRINTLN_IDLE(gest);
        DEBUG_PRINTLN_IDLE(acc, gyr);
    }
}