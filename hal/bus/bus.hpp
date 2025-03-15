#pragma once

#include "bus_base.hpp"
#include "sys/utils/BytesIterator.hpp"


namespace ymd{

class ReadableBus:public BusBase{
public:
    virtual BusError read(uint32_t & data, const Ack ack) = 0;
};

class WritableBus:public BusBase{
protected:
public:
    virtual BusError write(const uint32_t data) = 0;
};

class HalfDuplexBus:public BusBase{
public:
    virtual BusError read(uint32_t & data, const Ack ack) = 0;
    virtual BusError write(const uint32_t data) = 0;
};

class FullDuplexBus:public BusBase{
public:
    virtual BusError read(uint32_t & data) = 0;
    virtual BusError write(const uint32_t data) = 0;
    virtual BusError transfer(uint32_t & data_rx, const uint32_t data_tx) = 0;
};

template<typename Packet>
class PackedBus:public BusBase{
public:
    virtual bool write(const Packet & msg) = 0;
    // virtual bool write(const Packet && msg){return write(static_cast<const Packet &>(msg));}
    virtual const Packet && read() = 0;
};

}