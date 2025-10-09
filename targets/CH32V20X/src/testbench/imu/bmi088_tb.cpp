#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/BMI088/BMI088.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::drivers;

using ymd::drivers::BMI088_Acc;
using ymd::drivers::BMI088_Gyr;

#define UART hal::uart2
#define ACC_CS_GPIO hal::PB<0>()
#define GYR_CS_GPIO hal::PB<1>()

static constexpr size_t CALC_FREQ_HZ = 200;



static void bmi088_tb(hal::Spi & spi){
    auto acc_cs_gpio_ = ACC_CS_GPIO;
    auto gyr_cs_gpio_ = GYR_CS_GPIO;

    const auto acc_cs_idx = spi.allocate_cs_gpio(&acc_cs_gpio_).unwrap();
    const auto gyr_cs_idx = spi.allocate_cs_gpio(&gyr_cs_gpio_).unwrap();
    

    auto acc_sensor = BMI088_Acc{&spi, acc_cs_idx};
    auto gyr_sensor = BMI088_Gyr{&spi, gyr_cs_idx};

    acc_sensor.init().examine();
    gyr_sensor.init().examine();


    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    hal::timer1.init({CALC_FREQ_HZ}, EN);
    hal::timer1.attach<hal::TimerIT::Update>(
        {0,0}, [&](){

            const auto begin_m = clock::micros();

            
            mahony.update(
                gyr_sensor.read_gyr().examine(), 
                acc_sensor.read_acc().examine()
            );
                
            const auto end_m = clock::micros();

            DEBUG_PRINTLN(
                mahony.rotation(), 
                end_m - begin_m
            );
        }, EN
    );

    while(true);
}


void bmi088_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    clock::delay(200ms);

    hal::spi1.init({9_MHz});
    bmi088_tb(hal::spi1);
}