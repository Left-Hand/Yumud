#include "soft_uart.hpp"
#include "hal/gpio/gpio.hpp"

using namespace ymd::hal;

void UartSw::init(const Config & cfg){
    set_rx_strategy(cfg.rx_strategy);
    set_tx_strategy(cfg.tx_strategy);
}

void UartSw::tick(){
    // tx_pin_.toggle();
    switch(prog_){
        case ByteProg::START:
            tx_pin_.set_low();
            prog_ = ByteProg::D0;
            break;

        case ByteProg::D0 ... ByteProg::D7:
            tx_pin_.write(BoolLevel::from(now_char_ & (1 << (uint8_t)prog_)));
            prog_ = (prog_ == ByteProg::D7) ? ByteProg::STOP : ByteProg((int8_t)prog_ + 1);
            break;
            
        case ByteProg::STOP:
            tx_pin_.set_high();
            prog_ = ByteProg::IDLE;
            break;
        case ByteProg::IDLE:
            tx_pin_.set_high();
            if(tx_queue_.length()){
                now_char_ = fetch_next();
                // now_char_ = 0x55;
                prog_ = ByteProg::START;
            }
            break;
    }

}

void UartSw::set_tx_strategy(const CommStrategy tx_strategy){
    tx_pin_.outpp(HIGH);
}

void UartSw::set_rx_strategy(const CommStrategy rx_strategy){
    rx_pin_.inpu();
}