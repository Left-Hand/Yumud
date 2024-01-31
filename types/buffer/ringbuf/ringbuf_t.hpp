#ifndef __RING_BUF_HPP

#define __RING_BUF_HPP

#include "../buffer.hpp"

template<typename T>
class RingBuf_t:public Buffer_t<T>{
protected:
    T * tx_ptr;
    T * rx_ptr;

    T* advancePointer(T* ptr, size_t step = 1) {
        return (ptr + step >=this->buf + this->size) ? ptr + step - this->size : ptr + step;
    }

public:
    RingBuf_t():Buffer_t<T>(), tx_ptr(this->buf), rx_ptr(this->buf){;}

    __fast_inline void addTxData(const T & data) override{
        *tx_ptr = data;
        tx_ptr = advancePointer(tx_ptr);
    }

    __fast_inline void getTxData(T & data) override{
        data = *rx_ptr;
        rx_ptr = advancePointer(rx_ptr);
    }
        
    __fast_inline void addRxData(const T & data) override{
        *rx_ptr = data;
        rx_ptr = advancePointer(rx_ptr);
    }

    __fast_inline void getRxData(T & data) override{
        data = *tx_ptr;
        tx_ptr = advancePointer(tx_ptr);
    }
    
    
    __fast_inline T * rxPtr() override{return rx_ptr;}
    __fast_inline T * txPtr() override{return tx_ptr;}

    size_t available() const override{
        if (rx_ptr >= tx_ptr) {
            return rx_ptr - tx_ptr;
        } else {
            return this->size - (tx_ptr - rx_ptr);
        }
    }

    void addTxdatas(const T * data_ptr, const size_t & len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) addTxData(data_ptr[i]);
        }else{
            for(size_t i = 0; i < len; i++) addTxData(data_ptr[i]);
        }
    }

    void getRxDatas(T * data_ptr, const size_t & len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) getRxData(data_ptr[i]);
        }else{
            for(size_t i = 0; i < len; i++) getRxData(data_ptr[i]);
        }
    }
};

typedef RingBuf_t<uint8_t> RingBuf;

#endif // !__RING_BUF_HPP