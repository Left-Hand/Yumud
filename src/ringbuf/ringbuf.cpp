#include "ringbuf.hpp"


RingBuf::RingBuf(const size_t & _size) : size(_size) {
    buf = new uint8_t[size];
    tx_ptr = buf;
    rx_ptr = buf;
}

RingBuf::~RingBuf() {
    delete[] buf;
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

