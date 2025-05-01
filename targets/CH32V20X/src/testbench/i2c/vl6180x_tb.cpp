#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/Proximeter/VL6180X/VL6180X.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]

using drivers::VL6180X;

[[maybe_unused]]
static void vl6180x_range_single_shot_tb(VL6180X & vl6180){
    vl6180.init();
    vl6180.configure_default();
    vl6180.set_timeout(500);

    while(true){
        DEBUG_PRINTLN(
            vl6180.read_range_single_millimeters().unwrap()
        );
    }
}

[[maybe_unused]]
static void vl6180x_range_single_shot_scaling_tb(VL6180X & vl6180){
    static constexpr auto SCALING = 2;

    vl6180.init();
    vl6180.configure_default();
    vl6180.set_scaling(SCALING);
    vl6180.set_timeout(500);
    while(true){
        DEBUG_PRINTLN(
            // vl6180.get_scaling().unwrap(),
            vl6180.read_range_single_millimeters().unwrap()
        );
        delay(1);
    }
}


[[maybe_unused]]
static void vl6180x_range_interleaved_continuous_tb(VL6180X & vl6180){ 
    vl6180.init();
    vl6180.configure_default();

    // Reduce range max convergence time and ALS integration
    // time to 30 ms and 50 ms, respectively, to allow 10 Hz
    // operation (as suggested by table "Interleaved mode
    // limits (10 Hz operation)" in the datasheet).
    vl6180.set_max_convergence_time(30);
    vl6180.set_inter_measurement_period(50);

    vl6180.set_timeout(500);

    // stop continuous mode if already active
    vl6180.stop_continuous();
    // in case stopContinuous() triggered a single-shot
    // measurement, wait for it to complete
    delay(1300);
    // start interleaved continuous mode with period of 100 ms
    vl6180.start_interleaved_continuous(100);
    vl6180.start_ambient_continuous(100);
    vl6180.start_range_continuous(100);


    while(true){
        DEBUG_PRINTLN(
            vl6180.read_ambient_continuous().unwrap(),
            vl6180.read_range_continuous_millimeters().unwrap()
        );
    }
}

void vl6180x_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();

    hal::I2cSw i2c = {SCL_GPIO, SDA_GPIO};
    i2c.init(200_KHz);

    // VL6180X vl6180{i2c, I2cSlaveAddr<7>::from_u7(0)};
    VL6180X vl6180{i2c};

    // vl6180x_range_single_shot_tb(vl6180);
    vl6180x_range_single_shot_scaling_tb(vl6180);
    // vl6180x_range_interleaved_continuous_tb(vl6180);
}