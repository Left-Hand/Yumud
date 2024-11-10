#include "../../testbench/tb.h"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/spi/spisw.hpp"

#include "drivers/Memory/Flash/X25QXX.hpp"

using namespace yumud::drivers;

void w25qxx_main(){
    DEBUGGER_INST.init(576000);

    auto & spi = spi1;
    // auto spi = SpiSw(SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO);
    auto & w25_cs = portD[5];

    spi.bindCsPin(w25_cs, 0);
    spi.init(1000000);
    // spi.init(18000000, CommMethod::Blocking, CommMethod::None);
    // SPI1_MOSI_GPIO.afpp();
    // SPI1_SCLK_GPIO.afpp();
    // SPI1_MISO_GPIO.inflt();

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    // GPIO_PinRemapConfig(SPI1_REMAP, ENABLE);

    // SPI_InitTypeDef SPI_InitStructure;
    // SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    // SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    // SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    // SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    // SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    // SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    // SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    // SPI_InitStructure.SPI_CRCPolynomial = 7;
    // SPI_Init(SPI1, &SPI_InitStructure);

    // SPI_Cmd(SPI1, ENABLE);
    X25QXX w25{SpiDrv{spi, 0}};
    while(true){
        // DEBUG_PRINTLN(w25.getDeviceManufacturer());
        // DEBUG_PRINTLN(std::hex, std::showbase, w25.getDeviceId());
        DEBUG_PRINTLN(std::oct, w25.getDeviceCapacity() >> 10, "kB", w25.getDeviceManufacturer());
        DEBUG_PRINTLN(std::oct, millis());
        // spi.write(0xa5);
        // // w25_cs = !w25_cs;
        // SPI1_SCLK_GPIO = !SPI1_SCLK_GPIO;
        // SPI1_MOSI_GPIO = !SPI1_MOSI_GPIO;
        // SPI1_MISO_GPIO = !SPI1_MISO_GPIO;
        delay(200);
    }
}