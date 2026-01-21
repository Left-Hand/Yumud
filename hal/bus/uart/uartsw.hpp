#pragma once

#include "uart.hpp"

namespace ymd::hal{
class UartSw final: public UartBase{
public:

    UartSw(Gpio & tx_gpio, Gpio & rx_gpio): tx_pin_(tx_gpio), rx_pin_(rx_gpio){;}

    void init(const Config & cfg);

    void tick();

    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);

    size_t try_write_bytes(std::span<const uint8_t> bytes){
        return tx_fifo_.try_push(bytes);
    }

    size_t try_write_byte(const uint8_t byte){
        return tx_fifo_.try_push(byte);
    }

    Gpio & txio(){return tx_pin_;}
    Gpio & rxio(){return rx_pin_;}
    void set_parity(const Parity parity){;}

private:
    Gpio & tx_pin_;
    Gpio & rx_pin_;

    enum class ByteProg: int8_t{
        D0 = 0,D1,D2,D3,D4,D5,D6,D7,
        START,
        STOP,
        IDLE,
    };

    ByteProg prog_ = ByteProg::IDLE;

    uint16_t current_char = '\0';
    uint8_t fetch_next(){
        uint8_t ret;
        if(const auto len = tx_fifo_.try_pop(ret);
            len == 0) __builtin_trap();
        return  ret;
    }
};
}
