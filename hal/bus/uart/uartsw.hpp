#ifndef __UARTSW_HPP__
#define __UARTSW_HPP__

#include "uart.hpp"

class UartSw : public Uart{
protected:
    Gpio & m_tx_gpio;
    Gpio & m_rx_gpio;

    enum class ByteProg:int8_t{
        START = -1,
        D0,D1,D2,D3,D4,D5,D6,D7,
        STOP
    };

    char current_char;
    ByteProg byteProg = ByteProg::STOP;
    
    void write(const char data) override{
        txBuf.addData(data);
    }

    char fetch_next(){return txBuf.getData();}
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
#endif
