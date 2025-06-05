#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/string/string.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/spi/spisw.hpp"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

bool isInInterruptContext() {
    // uint64_t mstatus;
    // asm volatile("csrr %0, mstatus" : "=r"(mstatus));  // 读取 mstatus 寄存器
    // uint64_t mpp = mstatus & 0b11;             // 获取 mpp 的值
    // bool inInterrupt = (mpp == 1 || mpp == 3);        // 如果 mpp 为 1 或 3，则在中断上下文中
    
    // return inInterrupt;
    // scexpr uint32_t address = 0xE000E04C;
    // return * (uint32_t *)(address) & (0b01'0000'0000);
    return QingKeV4::isIntrruptActing();
}

void lt8920_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);


    // SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;
    // auto & spi = spisw;


    spi.init(2_MHz);
    

    LT8920 lt{spi, spi.attach_next_cs(portA[0]).value()};
    bindSystickCb([&](){
        lt.tick().examine();
    });

    if(lt.validate().is_ok()){
        DEBUG_PRINTLN("LT8920 founded");
    }else{
        DEBUG_PRINTLN("LT8920 not founded, please check your physical connection");
        PANIC();
    }

    lt.bind_nrst_gpio(portB[0]).examine();
    lt.init().examine();
    lt.set_data_rate(1_MHz).examine();

    // LT8920::Role role = LT8920::Role::BROADCASTER;
    // lt.setRole(role);
    bool is_rx = (sys::chip::get_chip_id_crc() != 0x5E0799D2);
    
    if(is_rx) lt.start_listen().examine();

    if(is_rx){

    }else{
        auto src = String("Hello World!!!");
        while(true){
            // lt.validate();
            // lt.setDataRate(LT8920::DataRate::Kbps125);
            // DEBUG_PRINTLN(lt.isRfSynthLocked());

            lt.write_block(std::span(
                reinterpret_cast<const uint8_t *>(src.c_str()), 
                src.length())).examine();
            clock::delay(200ms);
            // DEBUG_PRINTLN(src, lt.receivedAck());
        }
    }
}