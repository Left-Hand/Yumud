#ifndef __SPI2_HPP__

#define __SPI2_HPP__

#include "../bus.hpp"
#include "../../defines/comm_inc.h"
#include "../../clock/clock.h"


#define SPI2_CS_Periph RCC_APB2Periph_GPIOB
#define SPI2_CS_Port GPIOB
#define SPI2_CS_Pin GPIO_Pin_12

#define SPI2_GPIO_Periph RCC_APB2Periph_GPIOB
#define SPI2_Port GPIOB
#define SPI2_SCK_Pin GPIO_Pin_13
#define SPI2_MISO_Pin GPIO_Pin_14
#define SPI2_MOSI_Pin GPIO_Pin_15

#define CS_LOW \
__nopn(6); \
SPI2_CS_Port->BCR = SPI2_CS_Pin; \
__nopn(6); \

#define CS_HIGH \
__nopn(6); \
SPI2_CS_Port->BSHR = SPI2_CS_Pin; \
__nopn(6); \

class Spi2:public Bus{
public:
    Spi2():Bus(){;}
    void init(const uint32_t & baudRate) override;

    void configDataSize(const uint8_t & data_size) override;
    void configBaudRate(const uint32_t & baudRate) override;
    void configBitOrder(const bool & msb) override;
    __fast_inline void begin() override {CS_LOW;}
    __fast_inline void end() override {CS_HIGH;}
    __fast_inline bool busy() override {return (SPI2->STATR & SPI_I2S_FLAG_BSY);}

    __fast_inline Error write(const uint32_t & data) override;
    Error write(void * _data_ptr, const size_t & len) override;

    __fast_inline Error read(uint32_t & data, bool toAck = true) override;
    Error read(void * _data_ptr, const size_t & len) override;

    __fast_inline Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override;
    Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) override;
};

__fast_inline Bus::Error Spi2::write(const uint32_t & data){
    uint32_t _;
    return transfer(_, data);
}

__fast_inline Bus::Error Spi2::read(uint32_t & data, bool toAck){
    return transfer(data, 0);
}

__fast_inline Bus::Error Spi2::transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
    while ((SPI2->STATR & SPI_I2S_FLAG_TXE) == RESET);
    SPI2->DATAR = data_tx; 
    
    while ((SPI2->STATR & SPI_I2S_FLAG_RXNE) == RESET);
    data_rx = SPI2->DATAR;
    
    return Bus::ErrorType::OK;
}

uint16_t SPI2_Prescale_Caculate(uint32_t baudRate);
void SPI2_GPIO_Init(void);
void SPI2_Init(uint32_t baudRate);

#undef CS_LOW
#undef CS_HIGH

extern Spi2 spi2;

#endif