#pragma once

namespace ymd{

class BusTrait{
public:
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t len){};
    virtual void setBaudRate(const uint32_t baudRate) = 0;
};

}