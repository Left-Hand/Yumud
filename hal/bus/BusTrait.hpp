#pragma once

namespace ymd{

class BusIntf{
public:
    virtual void setBaudRate(const uint32_t baudRate) = 0;
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t len){};
};

class NonProtocolBusIntf:public BusIntf{
public:
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t len){};
};

}