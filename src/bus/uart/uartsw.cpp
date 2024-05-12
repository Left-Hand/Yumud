#include "uartsw.hpp"

void UartSw::init(const uint32_t & baudRate, const Mode & _mode,const CommMethod & _rxMethod,const CommMethod & _txMethod){
    mode = _mode;

    if(((uint8_t)mode & (uint8_t)Mode::TxOnly)){
        m_tx_gpio.OutPP(true);
    }

    if(((uint8_t)mode & (uint8_t)Mode::RxOnly)){
        m_rx_gpio.InPullUP();
    }
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
            if(txBuf.available()){
                current_char = fetch_next();
                byteProg = ByteProg::START;
            }
            break;
    }



}

void UartSw::setTxMethod(const CommMethod & _txMethod){}

void UartSw::setRxMethod(const CommMethod & _rxMethod){}