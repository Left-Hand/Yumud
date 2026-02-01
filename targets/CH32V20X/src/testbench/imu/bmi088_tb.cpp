#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/spi/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/IMU/Axis6/BMI088/BMI088.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::drivers;

using ymd::drivers::BMI088_Acc;
using ymd::drivers::BMI088_Gyr;

#define UART hal::usart2
#define ACC_CS_PIN hal::PB<0>()
#define GYR_CS_PIN hal::PB<1>()

static constexpr size_t CALC_FREQ_HZ = 200;



static void bmi088_tb(hal::Spi & spi){
    auto acc_cs_pin_ = ACC_CS_PIN;
    auto gyr_cs_pin_ = GYR_CS_PIN;

    const auto acc_cs_rank = spi.allocate_cs_pin(&acc_cs_pin_).unwrap();
    const auto gyr_cs_rank = spi.allocate_cs_pin(&gyr_cs_pin_).unwrap();
    

    auto acc_sensor = BMI088_Acc{&spi, acc_cs_rank};
    auto gyr_sensor = BMI088_Gyr{&spi, gyr_cs_rank};

    acc_sensor.init().examine();
    gyr_sensor.init().examine();


    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(CALC_FREQ_HZ),
        .count_mode = hal::TimerCountMode::Up
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            const auto begin_ms = clock::micros();

            
            mahony.update(
                gyr_sensor.read_gyr().examine(), 
                acc_sensor.read_acc().examine()
            );
                
            const auto end_ms = clock::micros();

            DEBUG_PRINTLN(
                mahony.rotation(), 
                end_ms - begin_ms
            );
            break;
        }
        default: break;
        }
    });

    timer.start();
    while(true);
}


void bmi088_main(){
    DEBUGGER_INST.init({
        hal::USART2_REMAP_PA2_PA3,
        hal::NearestFreq(576000),
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    clock::delay(200ms);

    hal::spi1.init({
        .remap = hal::SPI1_REMAP_PB3_PB4_PB5_PA15,
        .baudrate = hal::NearestFreq(9_MHz)
    });
    bmi088_tb(hal::spi1);
}