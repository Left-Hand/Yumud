#include "src/testbench/tb.h"
#include "hal/bus/i2c/i2csw.hpp"
#include "core/debug/debug.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Proximeter/PAJ7620/paj7620.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define UART_DEST_FPGA 0
#define UART_DEST_PC 1

#define UART_DEST UART_DEST_PC
// #define UART_DEST UART_DEST_FPGA

#define SCL_GPIO hal::PD<2>()
#define SDA_GPIO hal::PC<12>()
void paj7620_main(){
    auto & logger = DEBUGGER_INST;
    logger.init({576000});
    
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;  

    hal::I2cSw i2c = hal::I2cSw{&scl_gpio_, &sda_gpio_};
    i2c.init({400_KHz});


    PAJ7620 paj{&i2c};

    paj.init().examine();

    while(true){
        clock::delay(100ms);
        paj.update().examine();

        #if UART_DEST == UART_DEST_FPGA
        logger.write(paj.detect());
        #elif UART_DEST == UART_DEST_PC
        DEBUG_PRINTLN(
            paj.detect()
                .unwrap()
                .to_bitset()
        );
        #endif
        // DEBUG_PRINTLN(std::bitset<8>(6));
    }
    while(true);

    // while(true){
    //     servo_left.setRadian(0);
    //     servo_right.setRadian(0);
    // }
}