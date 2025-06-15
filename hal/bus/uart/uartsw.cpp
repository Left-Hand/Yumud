#include "uartsw.hpp"
#include "core/stream/CharOpTraits.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd::hal;

void UartSw::init(const Config & cfg){
    set_rx_strategy(cfg.rx_strategy);
    set_tx_strategy(cfg.tx_strategy);
}

void UartSw::tick(){
    // tx_gpio_.toggle();
    switch(prog_){
        case ByteProg::START:
            tx_gpio_.clr();
            prog_ = ByteProg::D0;
            break;

        case ByteProg::D0 ... ByteProg::D7:
            tx_gpio_.write(BoolLevel::from(current_char & (1 << (uint8_t)prog_)));
            prog_ = (prog_ == ByteProg::D7) ? ByteProg::STOP : ByteProg((int8_t)prog_ + 1);
            break;
            
        case ByteProg::STOP:
            tx_gpio_.set();
            prog_ = ByteProg::IDLE;
            break;
        case ByteProg::IDLE:
            tx_gpio_.set();
            if(tx_fifo_.available()){
                current_char = fetch_next();
                // current_char = 0x55;
                prog_ = ByteProg::START;
            }
            break;
    }

}

void UartSw::set_tx_strategy(const CommStrategy tx_strategy){
    tx_gpio_.outpp(HIGH);
}

void UartSw::set_rx_strategy(const CommStrategy rx_strategy){
    rx_gpio_.inpu();
}