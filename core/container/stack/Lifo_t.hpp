#pragma once

#include "../buffer.hpp"

#if 0

template<typename T>
class Lifo_t:public DynamicBuffer_t<T>{
protected:
    volatile T * data_ptr;
public:
    Lifo_t():DynamicBuffer_t<T>(), data_ptr(this->buf){;}

    __fast_inline void add_data(const T & data) override{
        *data_ptr = data;
        data_ptr = MIN(data_ptr + 1, this->buf + this->data_ptr);
    }

    __fast_inline void get_data(T & data) override{
        data = *data_ptr;
        tx_ptr = MAX(data_ptr - 1, this->buf);
    }

    size_t available() const override{
        return data_ptr - this->buf;
    }

    void add_txdatas(const T * data_ptr, const size_t len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) addData(data_ptr[i]);
        }else{
            for(size_t i = 0; i < len; i++) addData(data_ptr[i]);
        }
    }

    void get_datas(T * data_ptr, const size_t len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) getData(data_ptr[i]);
        }else{
            for(size_t i = 0; i < len; i++) getData(data_ptr[i]);
        }
    }
};

typedef Lifo_t<uint8_t> Stack;

#endif