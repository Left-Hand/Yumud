
#ifndef __BUS_HPP__
#define __BUS_HPP__

#include <cstring>
#include <cstdint>
#include <cmath>
#include <sstream>
#include <cstdlib>
#include "../types/string/string.hpp"
#include "../defines/comm_inc.h"
#include "../clock/clock.h"

class Bus{
protected:
    uint16_t write_ovt = 114;
    uint16_t read_ovt = 114;

    uint8_t data_size = 8;

public:
    enum class ErrorType{
        OK,
        IS_USING,
        OVER_TIME,
        NO_ACK
    };

    struct Error{
        ErrorType errorType = ErrorType::OK;

        Error(const ErrorType & _errorType):errorType(_errorType){;}

        explicit operator bool() {return errorType != ErrorType::OK;}
        explicit operator ErrorType() {return errorType;}
    };

    virtual void init(const uint32_t & baudRate) = 0;

    virtual void configDataSize(const uint8_t & data_size) = 0;
    virtual void configBaudRate(const uint32_t & baudRate) = 0;
    virtual void configBitOrder(const bool & msb) = 0;

    virtual Bus::Error begin(const uint8_t & index = 0) = 0;
    virtual void end() = 0;

    virtual Error write(const uint32_t & data) = 0;
    virtual Error write(void * _data_ptr, const size_t & len) = 0;

    virtual Error read(uint32_t & data, bool toAck = true) = 0;
    virtual Error read(void * _data_ptr, const size_t & len) = 0;

    virtual Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) = 0;
    virtual Error transfer(void * _data_rx_ptr, void * _data_tx_ptr, const size_t & len) = 0;
};
    
#endif // !__PRINTABLE_HPP__