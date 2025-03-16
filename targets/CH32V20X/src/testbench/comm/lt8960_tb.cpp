#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"

using namespace ymd::drivers;


#define UART uart2

#define MAG_ACTIVATED

#define let const auto
void lt8960_tb(){

    auto & led = portC[13];
    led.outpp();


    LT8960L rx_ltr{&portB[6], &portB[7]};
    
        rx_ltr.verify().unwrap();
    rx_ltr.init(LT8960L::Power::_8_Db, 0x12345678).unwrap();
    
    DEBUG_PRINTLN("RX LT8960L init ok!");
    
    LT8960L tx_ltr{&portB[0], &portB[1]};

    tx_ltr.verify().unwrap();
    tx_ltr.init(LT8960L::Power::_8_Db, 0x12345678).unwrap();

    DEBUG_PRINTLN("TX LT8960L init ok!");
    
    scexpr auto ch = LT8960L::Channel(76);
    auto tx_task = [&]{
        constexpr std::array data = {std::byte(0x12), std::byte(0x34), std::byte(0x56), std::byte(0x78)};

        tx_ltr.transmit_rf(ch, std::span(data)).unwrap();
        
        DEBUG_PRINTS("tx:", tx_ltr.is_pkt_ready().unwrap());
        led.toggle();
    };

    [[maybe_unused]] auto rx_task = [&]{
        static std::array<std::byte, 64> data;
        auto len = rx_ltr.receive_rf(ch, data).unwrap();
        DEBUG_PRINTS("rx:", len);
    };
    

    hal::timer1.init(10);
    hal::timer1.attach(hal::TimerIT::Update, {0,0}, tx_task);

    delay(50);

    // hal::timer2.init(10);
    // hal::timer2.attach(hal::TimerIT::Update, {0,1}, rx_task);

    while(true);
}

void lt8960_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();

    delay(200);

    lt8960_tb();
}