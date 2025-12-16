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

#define SCL_PIN hal::PD<2>()
#define SDA_PIN hal::PC<12>()
void paj7620_main(){
    hal::usart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    
    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;  

    hal::I2cSw i2c = hal::I2cSw{&scl_pin_, &sda_pin_};
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