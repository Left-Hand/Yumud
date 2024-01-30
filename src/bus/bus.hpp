
#ifndef __BUS_HPP__
#define __BUS_HPP__

#include <cstring>
#include <cstdint>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include "../types/string/string.hpp"

class Bus{
public:
    enum class ErrorType{
        OK = 0,
        OVER_TIME = 1,
        NO_ACK = 2
    };

    struct Error{
        ErrorType errorType = ErrorType::OK;

        Error(const ErrorType & _errorType):errorType(_errorType){;}

        explicit operator bool() {return errorType == ErrorType::OK;}
        explicit operator ErrorType() {return errorType;}
    };

    virtual void init() = 0;
    virtual void configDataSize(uint8_t data_size) = 0;
    virtual void configBaudRate(uint32_t baud_rate) = 0;
    
    virtual Error write(const size_t & data) = 0;
    virtual Error write(const size_t * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual Error read(size_t & data, bool toAck = true) = 0;
    virtual Error read(size_t * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual Error trans(size_t & data_rx, size_t & data_tx, bool toAck = true) = 0;
    virtual Error trans(size_t * data_rx_ptr, size_t * data_tx_ptr, const size_t & len, bool msb = false) = 0;
};
    
#endif // !__PRINTABLE_HPP__