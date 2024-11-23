#pragma once

namespace ymd{

class BusTrait{
public:
    virtual void setBaudRate(const uint32_t baudRate) = 0;
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t len){};
};

class NonProtocolBusTrait:public BusTrait{
public:
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t len){};
};

}