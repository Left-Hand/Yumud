#include "src/testbench/tb.h"
#include "hal/bus/i2c/i2csw.hpp"
#include "core/debug/debug.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/port.hpp"

#include "drivers/Proximeter/PAJ7620/paj7620.hpp"

using namespace ymd::drivers;

#define UART_DEST_FPGA 0
#define UART_DEST_PC 1

#define UART_DEST UART_DEST_PC
// #define UART_DEST UART_DEST_FPGA

void paj7620_main(){
    auto & logger = DEBUGGER_INST;
    logger.init(DEBUG_UART_BAUD);
    
    I2cSw i2c = {portD[2], portC[12]};
    i2c.init(400_KHz);


    PAJ7620 paj{i2c};

    paj.init();

    while(true){
        delay(100);
        paj.update();

        #if UART_DEST == UART_DEST_FPGA
        logger.write(paj.detect());
        #elif UART_DEST == UART_DEST_PC
        DEBUG_PRINTLN(std::bitset<8>(paj.detect()));
        #endif
        // DEBUG_PRINTLN(std::bitset<8>(6));
    }
    while(true);

    // while(true){
    //     servo_left.setRadian(0);
    //     servo_right.setRadian(0);
    // }
}