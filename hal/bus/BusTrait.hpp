#pragma once

namespace yumud{

class BusTrait{
public:
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t data_size){};
    virtual void setBaudRate(const uint32_t baudRate) = 0;
};

}