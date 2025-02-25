#pragma once

#include "uart.hpp"

namespace ymd::hal{
class UartSw : public Uart{
protected:
    Gpio & m_tx_gpio;
    Gpio & m_rx_gpio;

    enum class ByteProg: int8_t{
        D0 = 0,D1,D2,D3,D4,D5,D6,D7,
        START,
        STOP
    };

    char current_char;
    ByteProg byteProg = ByteProg::STOP;
    
    void write(const char data) override{
        tx_fifo.push(data);
    }

    char fetch_next(){return tx_fifo.pop();}
    Error lead(const uint8_t _address) override;
    void trail() override{;}
public:

    UartSw(Gpio & _m_tx_gpio, Gpio & _m_rx_gpio): m_tx_gpio(_m_tx_gpio), m_rx_gpio(_m_rx_gpio){;}

    void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) override;
    void tick();

    void setTxMethod(const CommMethod _txMethod) override;

    void setRxMethod(const CommMethod _rxMethod) override;
};
}