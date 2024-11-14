#pragma once

#include "bus_base.hpp"


namespace ymd{


class ReadableBus:virtual public Bus{
protected:
public:
    CommMethod rxMethod = CommMethod::None;
    virtual Error read(uint32_t & data, bool toAck = true) = 0;
};

class WritableBus:virtual public Bus{
protected:
public:
    CommMethod txMethod = CommMethod::None;
    virtual Error write(const uint32_t data) = 0;
};

class DuplexBus:public ReadableBus, WritableBus{
public:
// public:
    using WritableBus::txMethod;
    using ReadableBus::rxMethod;
    using WritableBus::write;
    using ReadableBus::read;
    DuplexBus():ReadableBus(), WritableBus(){;}
};

class FullDuplexBus:public DuplexBus{
public:
    virtual Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true) = 0;
};


class ProtocolBus:public DuplexBus{
protected:

};

template<typename Packet>
class PackedBus:public Bus{
private:
    using Bus::setDataBits;//disable this;
public:
    virtual bool write(const Packet & msg) = 0;
    virtual Packet read() = 0;
};

}