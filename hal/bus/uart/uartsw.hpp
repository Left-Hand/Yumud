#pragma once

#include "uart.hpp"

namespace ymd::hal{
class UartSw : public Uart{
protected:
    Gpio & tx_gpio_;
    Gpio & rx_gpio_;

    enum class ByteProg: int8_t{
        D0 = 0,D1,D2,D3,D4,D5,D6,D7,
        START,
        STOP,
        IDLE,
    };

    ByteProg prog_ = ByteProg::IDLE;

    uint16_t current_char = '\0';
    uint16_t fetch_next(){return tx_fifo_.pop();}
    BusError lead(const uint8_t address){return BusError::OK;}
    void trail(){;}
public:

    UartSw(Gpio & tx_gpio, Gpio & rx_gpio): tx_gpio_(tx_gpio), rx_gpio_(rx_gpio){;}

    void init(
        uint32_t baudrate, 
        CommStrategy tx_strategy = CommStrategy::Interrupt,
        CommStrategy rx_strategy = CommStrategy::Interrupt);

    void tick();

    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);

    void writeN(const char * pdata, const size_t len){tx_fifo_.push(std::span(pdata, len));}

    void write1(const char data){tx_fifo_.push(data);}

    Gpio & txio(){return tx_gpio_;}
    Gpio & rxio(){return rx_gpio_;}
    void set_parity(const Parity parity){;}
};
}