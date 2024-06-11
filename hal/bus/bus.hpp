
#ifndef __BUS_HPP__
#define __BUS_HPP__

#include "src/platform.h"
#include "src/clock/clock.h"

enum class CommMethod:uint8_t{
    None = 0,
    Blocking,
    Interrupt,
    Dma,
};

enum class CommMode:uint8_t{
    RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
};


class Bus{
protected:

public:
    enum class ErrorType{
        OK,
        ALREADY,
        OCCUPIED,
        TIMEOUT,
        OVERLOAD,
        NACK,
        NO_CS_PIN
    };

    struct Error{
        ErrorType errorType = ErrorType::OK;

        Error(const ErrorType & _errorType):errorType(_errorType){;}

        bool operator ==(const ErrorType & _errorType){return errorType == _errorType;}
        explicit operator bool() {return errorType != ErrorType::OK;}
        explicit operator ErrorType() {return errorType;}
    };

    // virtual void init(const uint32_t & baudRate) = 0;

    virtual void configDataSize(const uint8_t & data_size) = 0;
    virtual void configBaudRate(const uint32_t & baudRate) = 0;

    virtual Error begin(const uint8_t & index = 0) = 0;
    virtual void end() = 0;

    virtual Error write(const uint32_t & data) = 0;
    virtual Error read(uint32_t & data, bool toAck = true) = 0;
    virtual Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) = 0;
};

#endif // !__PRINTABLE_HPP__