#include "tb.h"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"
#include "bus/spi/spihw.hpp"
#include "bus/spi/spisw.hpp"


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
    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);


    // SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;
    // auto & spi = spisw;


    spi.init(2_MHz);
    spi.bindCsPin(portA[0], 0);

    LT8920 lt{spi, 0};
    bindSystickCb([&](){
        lt.tick();
    });

    if(lt.verify()){
        DEBUG_PRINTLN("LT8920 founded");
    }else{
        DEBUG_PRINTLN("LT8920 not founded, please check your physical connection");
        PANIC();
    }

    lt.bindNrstGpio(portB[0]);
    lt.init();
    lt.setDataRate(1_MHz);

    // LT8920::Role role = LT8920::Role::BROADCASTER;
    // lt.setRole(role);
    bool is_rx = (Sys::Chip::getChipIdCrc() != 0x5E0799D2);
    
    if(is_rx) lt.startListen();

    if(is_rx){

    }else{
        auto src = String("Hello World!!!");
        while(true){
            // lt.verify();
            // lt.setDataRate(LT8920::DataRate::Kbps125);
            // DEBUG_PRINTLN(lt.isRfSynthLocked());

            lt.writeBlock((const uint8_t *)src.c_str(), src.length());
            delay(200);
            // DEBUG_PRINTLN(src, lt.receivedAck());
        }
    }
}