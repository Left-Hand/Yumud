#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"
#include "src/testbench/algo/utils.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Actuator/SVPWM/svpwm3.hpp"
#include "hal/gpio/port.hpp"
using namespace ymd::drivers;


#define UART hal::uart2

#define MAG_ACTIVATED

#define let const auto



scexpr auto ch = LT8960L::Channel(76);
static constexpr uint32_t master_id = 65536;

uint32_t get_id(){
    // const auto id = uint32_t(sys::Chip::getChipId());
    const auto size = sys::chip::get_flash_size();
    return size;
}

bool has_tx_authority(){
    return get_id() == master_id;
}

bool has_rx_authority(){
    return get_id() == master_id;
}

void lt8960_tb(){

    auto & led = portC[13];
    led.outpp();


    LT8960L tx_ltr{&portB[6], &portB[7]};
    LT8960L rx_ltr{&portA[9], &portA[10]};
    
    auto common_settings = [](LT8960L & ltr){
        (ltr.set_rf_channel(ch)
        | ltr.enable_use_hw_pkt(true)
        | ltr.set_datarate(LT8960L::DataRate::_62_5K)
        // | ltr.set_datarate(LT8960L::DataRate::_1M)
        | ltr.enable_gain_weaken(true)
        // | ltr.set_syncword_tolerance_bits(1)
        | ltr.set_syncword_tolerance_bits(0)
        // | ltr.set_retrans_time(3)
        // | ltr.enable_autoack(false)

        ).unwrap();
    };

    if(has_rx_authority()) {
        rx_ltr.init(LT8960L::Power::_8_Db, 0x12345678).loc().expect("RX init failed!");
        common_settings(rx_ltr);
    }
    if(has_tx_authority()){
        tx_ltr.init(LT8960L::Power::_8_Db, 0x12345678).loc().expect("TX init failed!");

        common_settings(tx_ltr);
    }

    DEBUG_PRINTLN("LT8960L init done");
    
    auto tx_task = [&]{
        // if(!tx_ltr.is_pkt_ready().unwrap()) return;
        // std::array data = {std::byte(uint8_t(64 + 64 * sin(time() * 20))), std::byte(0x34), std::byte(0x56), std::byte(0x78)};
        const auto t = time();
        const auto [s, c] = sincos(t * 10);
        auto [u, v, w] = SVM(s,c);
        const auto payload = make_bytes_from_args(u, v, t);

        tx_ltr.transmit_rf(std::span(payload)).unwrap();
        led.toggle();
    };
    
    [[maybe_unused]] auto rx_task = [&]{
        static std::array<std::byte, 16> data;
        auto len = rx_ltr.receive_rf(data).unwrap();
        if(len){
            auto [u, v, w] = make_tuple_from_bytes<std::tuple<real_t, real_t, real_t>>(std::span<const std::byte>(data));
            // DEBUG_PRINTLN(u, v, w, time() - tt);
            DEBUG_PRINTLN(u, v, time() - w);
        }
        led.toggle();
    };



    if (has_tx_authority()) {
        hal::timer1.init(180);
        hal::timer1.attach(hal::TimerIT::Update, {0,0}, tx_task);
    }

    delay(5);

    if (has_rx_authority()) {
        hal::timer2.init(180);
        hal::timer2.attach(hal::TimerIT::Update, {0,1}, rx_task);
    }

    while(true);
}

void lt8960_main(){
    // UART.init(576_KHz);
    // UART.init(1152_KHz);
    UART.init(6_MHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();

    lt8960_tb();
}