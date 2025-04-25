#include "bus_base.hpp"

#include "core/stream/ostream.hpp"
#include "core/system.hpp"

using namespace ymd;
using namespace ymd::hal;



void BusBase::Locker::lock(const uint8_t index){
    sys::exception::disable_interrupt();
    oninterrupt_ = sys::exception::is_intrrupt_acting();
    req = index >> 1;
    locked_ = true;
    sys::exception::enable_interrupt();
}

bool BusBase::Locker::owned_by(const uint8_t index) const {
    return (req == index >> 1) and (sys::exception::is_intrrupt_acting() == oninterrupt_);
}



hal::BusError BusBase::begin(const uint8_t index){
    if(false == locker.locked()){
        locker.lock(index);
        return lead(index);
    }else if(locker.owned_by(index)){
        locker.lock(index);
        return lead(index);
    }else{
        return hal::BusError::OCCUPIED;
    }
}

hal::BusError BusBase::end(){
    this->trail();
    locker.unlock();

    return hal::BusError::OK;
}

namespace ymd{
OutputStream & operator << (OutputStream & os, const hal::BusError & err){
    return os << err.type;
}

OutputStream & operator << (OutputStream & os, const hal::BusError::Kind & err){
    using Kind = hal::BusError::Kind;

    switch(err){
        case Kind::OK:
            return os << "OK";
        case Kind::OCCUPIED:
            return os << "OCCUPIED";
        case Kind::ALREADY:
            return os << "ALREADY";
        case Kind::TIMEOUT:
            return os << "TIMEOUT";
        case Kind::NO_CS_PIN:
            return os << "NO_CS_PIN";
        case Kind::NO_ACK:
            return os << "NO_ACK";
        default:
            return os << "UNKNOWN" << uint8_t(err);
    }
}
}