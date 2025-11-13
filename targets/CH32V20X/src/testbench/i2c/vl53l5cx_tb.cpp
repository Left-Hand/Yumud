#include "core/debug/debug.hpp"

#include "core/utils/default.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/Proximeter/VL53L5CX/vl53l5cx.hpp"

// https://blog.csdn.net/qq_24312945/article/details/133848222

using namespace ymd;
using namespace ymd::drivers;

#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

using drivers::VL53L5CX;

void vl53l5cx_main(){
    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);

    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    i2c.init({400_KHz});

    auto red_led_gpio_ = hal::PC<13>();
    auto blue_led_gpio_ = hal::PC<14>();
    red_led_gpio_.outpp();
    blue_led_gpio_.outpp();

    auto blink_service_poller = [&]{

        red_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        blue_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    };


    // VL6180X vl6180{i2c, I2cSlaveAddr<7>::from_u7(0)};
    VL53L5CX vl53{&i2c};
    DEBUG_PRINTLN("start init");

    vl53.init().examine();
    [[maybe_unused]]auto motion_config = VL53L5CX::VL53L5CX_Motion_Config(Default);
    
    #if 0
    vl53.motion_indicator_init(motion_config, VL53L5CX::Resolution::_4x4).examine();
    vl53.motion_indicator_set_distance_motion(motion_config, 1000, 2000).examine();

    vl53.set_resolution(VL53L5CX::Resolution::_4x4).examine();
    vl53.set_ranging_mode(VL53L5CX::RangingMode::Autonomous).examine();
    // vl53.set_resolution(VL53L5CX::Resolution::_8x8).examine();
    vl53.set_integration_time_ms(10).examine();
    vl53.set_ranging_frequency_hz(5).examine();
    #else
    // vl53.set_resolution(VL53L5CX::Resolution::_8x8).examine();
    vl53.set_ranging_mode(VL53L5CX::RangingMode::Continuous).examine();
    // vl53.set_integration_time_ms(10).examine();
    vl53.set_ranging_frequency_hz(60).examine();
    #endif

    DEBUG_PRINTLN("resolution:", vl53.get_resolution().examine());
    DEBUG_PRINTLN("freq:", vl53.get_ranging_frequency_hz().examine());
    DEBUG_PRINTLN("intergration time:", vl53.get_integration_time_ms().examine());
    DEBUG_PRINTLN("ranging mode:", vl53.get_ranging_mode().examine());

    vl53.start_ranging().examine();
    
    drivers::VL53L5CX::VL53L5CX_Frame frame;
    while(true){
        blink_service_poller();
        if(vl53.is_data_ready().examine() == true){
            vl53.reflash_ranging_data(&frame).examine();
            const auto distance_mm = std::span(frame.distance_mm);
            DEBUG_PRINTLN(distance_mm.subspan(0, 16));
        } 
        clock::delay(1ms);
    }

}