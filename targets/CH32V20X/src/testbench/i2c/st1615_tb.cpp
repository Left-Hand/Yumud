#include "core/utils/cpp_stl/mdspan/mdspan.hpp"
#include <atomic>
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include <barrier>

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/prelude/keycode.hpp"
#include "drivers/HID/prelude/axis_input.hpp"
#include "drivers/HID/prelude/segcode.hpp"
#include "drivers/HID/prelude/button_input.hpp"
#include "drivers/HID/ST1615/ST1615.hpp"

#include "core/debug/debug.hpp"

using namespace ymd;
using drivers::ST1615;

#define UART hal::uart2
// #define UART hal::uart6
#define SCL_GPIO hal::PB<3>()
#define SDA_GPIO hal::PB<5>()

void st1615_main(){
    // UART.init({576_KHz});
    UART.init({
        .baudrate = 576000,
        .tx_strategy = CommStrategy::Dma
    });
    DEBUGGER.retarget(&UART);

    
    auto led = hal::PA<15>();
    led.outpp();

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;

    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    i2c.init(ST1615::MAX_I2C_BAUDRATE);

    auto nrst_gpio = hal::PB<0>();
    nrst_gpio.set_mode(hal::GpioMode::OutPP);
    nrst_gpio.clr();
    clock::delay(1ms);
    nrst_gpio.set();
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