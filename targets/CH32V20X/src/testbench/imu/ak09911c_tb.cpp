#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/IMU/Magnetometer/AK09911C/AK09911C.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::drivers;

// #define UART usart2
#define UART hal::usart2
#define SCL_PIN hal::PB<3>()
#define SDA_PIN hal::PB<5>()

static constexpr uint ISR_FREQ = 100;
static constexpr auto INV_ISR_FREQ = (1.0_uq24 / ISR_FREQ);


static void ak09911c_test(drivers::AK09911C & aku){

    aku.init().examine();
    aku.set_mode(AK09911C::Mode::Cont4).examine();
    DEBUG_PRINTLN("app started");

    auto rotation = Quat<iq24>::from_uninitialized();
    auto measure = [&](){
        aku.update().examine();
        const auto dir = aku.read_mag().examine();
        rotation = rotation.slerp(Quat<iq24>::from_direction(dir), 0.05_r);
    };

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(ISR_FREQ),
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
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            measure();
            break;
        }
        default: break;
        }
    });
    
    timer.start();
    while(true){
        // DEBUG_PRINTLN(aku.update());
        DEBUG_PRINTLN(clock::millis(), rotation, aku.read_mag());
    }
}

void ak09911c_main(){
    DEBUGGER_INST.init({
        hal::UART2_REMAP_PA2_PA3,
        576000
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;

    hal::I2cSw i2c{&scl_pin_, &sda_pin_};
    i2c.init({200_KHz});

    clock::delay(200ms);


    AK09911C aku{&i2c};

    // auto & spi = spi1;
    // spi.init(18_MHz);
    // AK09911C aku = {SpiDrv(spi, spi.allocate_cs_pin(hal::PA<15>()).value())};

    ak09911c_test(aku);
}