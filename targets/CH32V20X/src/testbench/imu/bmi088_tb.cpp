#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/BMI088/BMI088.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd::drivers;

using ymd::drivers::BMI088_Acc;
using ymd::drivers::BMI088_Gyr;

#define UART uart2
#define ACC_CS_PIN portB[0]
#define GYR_CS_PIN portB[1]

static constexpr size_t CALC_FREQ_HZ = 200;



static void bmi088_tb(hal::Spi & spi){
    const auto acc_cs_idx = spi.attach_next_cs(ACC_CS_PIN).value();
    const auto gyr_cs_idx = spi.attach_next_cs(GYR_CS_PIN).value();
    

    auto acc_sensor = BMI088_Acc{spi, acc_cs_idx};
    auto gyr_sensor = BMI088_Gyr{spi, gyr_cs_idx};

    acc_sensor.init().unwrap();
    gyr_sensor.init().unwrap();


    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    hal::timer1.init(CALC_FREQ_HZ);
    hal::timer1.attach(TimerIT::Update, {0,0}, [&](){

        const uint32_t begin_m = micros();

        
        mahony.update(
            gyr_sensor.read_gyr().unwrap(), 
            acc_sensor.read_acc().unwrap()
        );
            
        const uint32_t end_m = micros();

        DEBUG_PRINTLN(
            mahony.result(), 
            end_m - begin_m
        );
    });

    while(true);
}


void bmi088_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    delay(200);

    spi1.init(9_MHz);
    bmi088_tb(hal::spi1);
}