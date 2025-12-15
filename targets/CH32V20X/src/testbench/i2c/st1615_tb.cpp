#include <atomic>
#include <barrier>

#include "src/testbench/tb.h"

#include "core/utils/cpp_stl/mdspan/mdspan.hpp"
#include "core/debug/debug.hpp"
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/hid_input/keycode.hpp"
#include "primitive/hid_input/axis_input.hpp"
#include "primitive/hid_input/segcode.hpp"
#include "primitive/hid_input/button_input.hpp"

#include "drivers/HID/ST1615/ST1615.hpp"



using namespace ymd;
using drivers::ST1615;

#define UART hal::uart2
// #define UART hal::uart6
#define SCL_PIN hal::PB<3>()
#define SDA_PIN hal::PB<5>()

void st1615_main(){
    // UART.init({576_KHz});
    UART.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000,
        .tx_strategy = CommStrategy::Dma
    });
    DEBUGGER.retarget(&UART);

    
    auto led = hal::PA<15>();
    led.outpp();

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;

    hal::I2cSw i2c{&scl_pin_, &sda_pin_};
    i2c.init({ST1615::MAX_I2C_BAUDRATE});

    auto nrst_gpio = hal::PB<0>();
    nrst_gpio.set_mode(hal::GpioMode::OutPP);
    nrst_gpio.set_low();
    clock::delay(1ms);
    nrst_gpio.set_high();
    clock::delay(100ms);

    ST1615 st1615{&i2c};
    st1615.init().examine();


    std::array<Vec2i, ST1615::MAX_POINTS_COUNT> points = {
        Vec2i::ZERO, Vec2i::ZERO, Vec2i::ZERO,
        Vec2i::ZERO, Vec2i::ZERO, Vec2i::ZERO,
        Vec2i::ZERO, Vec2i::ZERO, Vec2i::ZERO
    };

    while(true){
        led.toggle();
        for(size_t i = 0; i < 1; ++i){
            st1615.get_point(i).examine().inspect([&](Vec2i point) {
                points[i] = point;
            });
        }

        // pos.inspect([&](Vec2i point) {
        //     // st1615.draw_point(point, 0xffff).examine();
        //     DEBUG_PRINTLN(
        //         // clock::millis(),
        //         // st1615.get_sensor_count().examine()

        //         // .unwrap_or<Vec2<int>>(Vec2{0, 0})
        //         point.x, point.y
        //         // st1615.get_gesture_info().examine()
        //     );
        // });

        DEBUG_PRINTLN(
            // pos, 
            std::span(points.data(), 2),
            // st1615.get_gesture_info().examine()
            // st1615.get_sensor_count().examine(),
            st1615.get_capabilities().examine()
        );

        clock::delay(30ms);
    }
}