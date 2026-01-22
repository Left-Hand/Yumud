#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;


#define UART_TB_ECHO

using namespace ymd;
[[maybe_unused]] static void uart_tb(hal::Uart & uart){
    #ifdef UART_TB_ECHO

    auto tx_led = hal::PC<13>();
    auto rx_led = hal::PC<14>();
    tx_led.outpp();
    rx_led.outpp();

    uart.set_event_callback([&](const hal::UartEvent& ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                rx_led.set_high();
                clock::delay(1ms);
                rx_led.set_low();
                break;
            case hal::UartEvent::TxIdle:
                tx_led.set_high();
                clock::delay(1ms);
                tx_led.set_low();
                break;
            default:
                PANIC{"Unexpected event", ev};
                break;
        }
    });

    while(true){
        // size_t size = uart.available();
        while(uart.available()){
            uint8_t chr;
            const auto len = uart.try_read_byte(chr);
            if(len)
            (void)uart.try_write_byte(chr);
            clock::delay(1ms);
        }
        clock::delay(300ms);
        tx_led.set_low();
        DEBUG_PRINTLN("noth", uart.available());
        tx_led.set_high();
    }
    #endif
}

#if 0
    auto & EXT_UART = hal::uart4;
    EXT_UART.init({
        .remap = hal::UartRemap::_0,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking,
    });

    auto & DBG_UART = hal::usart2;
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        // .baudrate = hal::NearestFreq(6000000),
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    // DEBUGGER.retarget(&EXT_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.force_sync(EN);
    auto led = hal::PC<13>();
    led.outpp();

    while(true){
        led.set_high();
        const char chars[] = {'h', '\r', '\n'};
        // EXT_UART.try_write_bytes(chars, sizeof(chars)-1);
        DEBUG_PRINTLN(EXT_UART.available(), EXT_UART.try_write_bytes(chars, 3));
        // DEBUG_PRINTLN(EXT_UART.available());
        clock::delay(10ms);
        led.set_low();
        // DEBUG_PRINTLN(EXT_UART.available());
        clock::delay(10ms);
    }
#endif

void uart_main(){
    // uart_tb_old();
    //usart1 passed
    //usart2 passed
    //usart3 passed
    //uart5 passed
    //uart6 passed
    //uart8 passed

    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&hal::usart2);
    // DEBUGGER.init(DEBUG_UART_BAUD, CommStrategy::Dma, CommStrategy::None);
    uart_tb(hal::usart2);
}
