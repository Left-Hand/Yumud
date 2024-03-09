// #ifndef __SPI2_HPP__

// #define __SPI2_HPP__

// #include "spi.hpp"
// #include "src/platform.h"

// class Spi2:public Spi{
// protected:
//     static int8_t occupied;

//     Error begin_use(const uint8_t & index = 0) override {
//         occupied = index;
//         __nopn(6);
//         SPI2_CS_Port->BCR = SPI2_CS_Pin;
//         __nopn(6);
//         return ErrorType::OK;
//     }

//     void end_use() override {
//         __nopn(6);
//         SPI2_CS_Port->BSHR = SPI2_CS_Pin;
//         __nopn(6);
//         occupied = -1;
//     }

//     bool is_idle() override {
//         return (occupied >= 0 ? false : true);
//     }

//     bool owned_by(const uint8_t & index = 0) override{
//         return (occupied == index);
//     }

// public:
//     void init(const uint32_t & baudRate) override;

//     void configDataSize(const uint8_t & data_size) override;
//     void configBaudRate(const uint32_t & baudRate) override;
//     void configBitOrder(const bool & msb) override;

//     __fast_inline Error write(const uint32_t & data) override;
//     __fast_inline Error read(uint32_t & data, bool toAck = true) override;
//     __fast_inline Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override;

//     // Error write(void * _data_ptr, const size_t & len) override;
//     // Error read(void * _data_ptr, const size_t & len) override;
//     // Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) override;
// };

// __fast_inline Bus::Error Spi2::write(const uint32_t & data){
//     uint32_t _;
//     return transfer(_, data);
// }

// __fast_inline Bus::Error Spi2::read(uint32_t & data, bool toAck){
//     return transfer(data, 0);
// }

// __fast_inline Bus::Error Spi2::transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){

//     while ((SPI2->STATR & SPI_I2S_FLAG_TXE) == RESET);
//     SPI2->DATAR = data_tx;

//     while ((SPI2->STATR & SPI_I2S_FLAG_RXNE) == RESET);
//     data_rx = SPI2->DATAR;

//     return Bus::ErrorType::OK;
// }

// uint16_t SPI2_Prescaler_Calculate(uint32_t baudRate);
// void SPI2_GPIO_Init(void);
// void SPI2_Init(uint32_t baudRate);



// extern Spi2 spi2;

// #endif