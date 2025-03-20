#include "uartsw.hpp"
#include "sys/stream/CharOpTraits.hpp"

using namespace ymd::hal;

void UartSw::init(
    uint32_t baudrate, 
    CommStrategy rx_strategy,
    CommStrategy tx_strategy
){
    set_rx_strategy(rx_strategy);
    set_tx_strategy(tx_strategy);
}

void UartSw::tick(){
    switch(byteProg){
        case ByteProg::START:
            tx_gpio_.clr();
            byteProg = ByteProg::D0;
            break;

        case ByteProg::D0 ... ByteProg::D7:
            tx_gpio_.write(current_char & (1 << (uint8_t)byteProg));
            byteProg = ByteProg((int8_t)byteProg + 1);
            break;

        case ByteProg::STOP:
            tx_gpio_.set();
            if(tx_fifo_.available()){
                current_char = fetch_next();
                byteProg = ByteProg::START;
            }
            break;
    }

}

void UartSw::set_tx_strategy(const CommStrategy tx_strategy){
    if(tx_strategy != CommStrategy::None){
        tx_gpio_.outpp(HIGH);
        // tx_strategy
    }
}

void UartSw::set_rx_strategy(const CommStrategy rx_strategy){
    if(rx_strategy != CommStrategy::None){
        rx_gpio_.inpu();
    }
}