#ifndef __RING_BUF_HPP

#define __RING_BUF_HPP

#include "../defines/comm_inc.h"
#include <cstdint>
#include <string>

class RingBuf{
private:
    uint8_t * buf;
    size_t size;

    uint8_t * tx_ptr;
    uint8_t * rx_ptr;

    uint8_t* advancePointer(uint8_t* ptr, size_t step = 1) {
        return (ptr + step >= buf + size) ? ptr + step - size : ptr + step;
    }

public:
    RingBuf(const size_t & _size = 128);
    ~RingBuf();

    __fast_inline void addTxData(const uint8_t & data){
        *tx_ptr = data;
        tx_ptr = advancePointer(tx_ptr);
    }

    __fast_inline void getTxData(uint8_t & data){
        data = *rx_ptr;
        rx_ptr = advancePointer(rx_ptr);
    }
        
    __fast_inline void addRxData(const uint8_t & data){
        *rx_ptr = data;
        rx_ptr = advancePointer(rx_ptr);
    }

    __fast_inline void getRxData(uint8_t & data){
        data = *tx_ptr;
        tx_ptr = advancePointer(tx_ptr);
    }
    
    __fast_inline void waste(const size_t & len){
        tx_ptr = advancePointer(tx_ptr, len);
    }
    
    __fast_inline uint8_t * rxPtr(){return rx_ptr;}
    __fast_inline uint8_t * txPtr(){return tx_ptr;}

    size_t available() const {
        if (rx_ptr >= tx_ptr) {
            return rx_ptr - tx_ptr;
        } else {
            return size - (tx_ptr - rx_ptr);
        }
    }

    void addTxdatas(const uint8_t * data_ptr, const size_t & len, bool msb = false);
    void getRxDatas(uint8_t * data_ptr, const size_t & len, bool msb = false);
};

#endif // !__RING_BUF_HPP