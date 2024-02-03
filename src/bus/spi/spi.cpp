#include "spi.hpp"

// Bus::Error Spi::write(void * _data_ptr, const size_t & len){
//     if(data_size == 8){
//         uint8_t * data_ptr = (uint8_t *)(_data_ptr);
//         for(size_t i = 0; i < len; i++) this->write(data_ptr[i]);
//     }else{
//         uint16_t * data_ptr = (uint16_t *)(_data_ptr);
//         for(size_t i = 0; i < len; i++) this->write(data_ptr[i]);
//     }
//     return Bus::ErrorType::OK;
// }

// Bus::Error Spi::transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len){
//     if(data_size == 8){
//         uint8_t * data_rx_ptr = (uint8_t *)(_data_rx_ptr);
//         uint8_t * data_tx_ptr = (uint8_t *)(_data_tx_ptr);
//         uint32_t data_rx = 0;
//         for(size_t i = 0; i < len; i++){
//             this -> transfer(data_rx, data_tx_ptr[i]);
//             data_rx_ptr[i] = data_rx;
//         }
//     }else{
//         uint16_t * data_rx_ptr = (uint16_t *)(_data_rx_ptr);
//         uint16_t * data_tx_ptr = (uint16_t *)(_data_tx_ptr);
//         uint32_t data_rx = 0;
//         for(size_t i = 0; i < len; i++){
//             this -> transfer(data_rx, data_tx_ptr[i]);
//             data_rx_ptr[i] = data_rx;
//         }
//     }
//     return Bus::ErrorType::OK;
// }

// Bus::Error Spi::read(void * _data_ptr, const size_t & len){
//     if(data_size == 8){
//         uint8_t * data_ptr = (uint8_t *)(_data_ptr);
//         uint32_t data_rx = 0;
//         for(size_t i = 0; i < len; i++){
//             this -> transfer(data_rx, 0);
//             data_ptr[i] = data_rx;
//         }
//     }else{
//         uint16_t * data_ptr = (uint16_t *)(_data_ptr);
//         uint32_t data_rx = 0;
//         for(size_t i = 0; i < len; i++){
//             this -> transfer(data_rx, 0);
//             data_ptr[i] = data_rx;
//         }
//     }
//     return Bus::ErrorType::OK;
// }