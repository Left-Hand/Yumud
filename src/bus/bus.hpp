
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

protected:

    virtual void init() = 0;
    virtual void configDataSize() = 0;
    
    virtual Error _write(const uint8_t & data) = 0;
    virtual Error _write(const uint8_t * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual Error _read(uint8_t & data, bool toAck = true) = 0;
    virtual Error _read(uint8_t * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual Error _trans(uint8_t & data_rx, uint8_t & data_tx, bool toAck = true) = 0;
    virtual Error _trans(uint8_t * data_rx_ptr, uint8_t * data_tx_ptr, const size_t & len, bool msb = false) = 0;

    // virtual Error _write2B(const uint16_t & data) = 0;
    // virtual Error _write2B(const uint16_t * data_ptr, const size_t & len, bool msb = false) = 0;
    // virtual Error _read2B(uint16_t & data, bool & toAck) = 0;
    // virtual Error _read2B(uint16_t * data_ptr, const size_t & len, bool msb = false) = 0;
    // virtual Error _trans2B(uint16_t & data_rx, uint16_t & data_tx, bool & toAck) = 0;
    // virtual Error _trans2B(uint16_t * data_rx_ptr, uint16_t * data_tx_ptr, const size_t & len, bool msb = false) = 0;

    // virtual Error _write2B(const uint32_t & data) = 0;
    // virtual Error _write2B(const uint32_t * data_ptr, const size_t & len, bool msb = false) = 0;
    // virtual Error _read2B(uint32_t & data, bool & toAck) = 0;
    // virtual Error _read2B(uint32_t * data_ptr, const size32_t & len, bool msb = false) = 0;
    // virtual Error _trans2B(uint32_t & data_rx, uint32_t & data_tx, bool & toAck) = 0;
    // virtual Error _trans2B(uint32_t * data_rx_ptr, uint32_t * data_tx_ptr, const size_t & len, bool msb = false) = 0;
};
    
#endif // !__PRINTABLE_HPP__