#pragma once

namespace ymd{

class BusIntf{
public:
    virtual void setBaudRate(const uint32_t baudrate) = 0;
    virtual void setBitOrder(const Endian endian){};
    virtual void set_data_width(const uint8_t len){};
};

class NonProtocolBusIntf:public BusIntf{
public:
    virtual void setBitOrder(const Endian endian){};
    virtual void set_data_width(const uint8_t len){};
};

}