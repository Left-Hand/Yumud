#include "ringbuf.hpp"


RingBuf::RingBuf(const size_t & _size) : size(_size) {
    buf = new uint8_t[size];
    tx_ptr = buf;
    rx_ptr = buf;
}

RingBuf::~RingBuf() {
    delete[] buf;
}

__fast_inline void RingBuf::addTxData(const uint8_t & data) {
    *tx_ptr = data;
    tx_ptr = advancePointer(tx_ptr);
}


__fast_inline void RingBuf::getTxData(uint8_t & data) {
    data = *rx_ptr;
    rx_ptr = advancePointer(rx_ptr);
}


__fast_inline void RingBuf::addRxData(const uint8_t & data) {
    *rx_ptr = data;
    rx_ptr = advancePointer(rx_ptr);
}


// __fast_inline void RingBuf::getRxData(uint8_t & data)


size_t RingBuf::available() const {
    if (tx_ptr >= rx_ptr) {
        return tx_ptr - rx_ptr;
    } else {
        return size - (rx_ptr - tx_ptr);
    }
}

void RingBuf::addTxdatas(const uint8_t * data_ptr, const size_t & len, bool msb){
    if(msb){
        for(size_t i = len - 1; i > 0; i--) addTxData(data_ptr[i]);
    }else{
        for(size_t i = 0; i < len; i++) addTxData(data_ptr[i]);
    }
}

void RingBuf::getRxDatas(uint8_t * data_ptr, const size_t & len, bool msb){
    if(msb){
        for(size_t i = len - 1; i > 0; i--) getRxData(data_ptr[i]);
    }else{
        for(size_t i = 0; i < len; i++) getRxData(data_ptr[i]);
    }
}

