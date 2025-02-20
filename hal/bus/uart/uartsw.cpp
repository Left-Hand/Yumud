#include "uartsw.hpp"

using namespace ymd;

void UartSw::init(const uint32_t baudRate, const CommMethod _rxMethod,const CommMethod _txMethod){
    setTxMethod(_txMethod);
    setRxMethod(_rxMethod);
}

void UartSw::tick(){
    switch(byteProg){
        case ByteProg::START:
            m_tx_gpio.clr();
            byteProg = ByteProg::D0;
            break;

        case ByteProg::D0 ... ByteProg::D7:
            m_tx_gpio.write(current_char & (1 << (uint8_t)byteProg));
            byteProg = ByteProg((int8_t)byteProg + 1);
            break;

        case ByteProg::STOP:
            m_tx_gpio.set();
            if(tx_fifo.available()){
                current_char = fetch_next();
                byteProg = ByteProg::START;
            }
            break;
    }



}

void UartSw::setTxMethod(const CommMethod _txMethod){
    if(_txMethod != CommMethod::None){
        m_tx_gpio.outpp(HIGH);
    }
}

void UartSw::setRxMethod(const CommMethod _rxMethod){
    if(_rxMethod != CommMethod::None){
        m_rx_gpio.inpu();
    }
}