#include "bus_base.hpp"

#include "sys/stream/ostream.hpp"
#include "sys/core/system.hpp"

using namespace ymd;


OutputStream & ymd::operator << (OutputStream & os, const BusError & err){
    return os << err.type;
}

OutputStream & ymd::operator << (OutputStream & os, const BusError::ErrorType & err){
    using ErrorType = BusError::ErrorType;

    switch(err){
        case ErrorType::OK:
            return os << "OK";
        case ErrorType::OCCUPIED:
            return os << "OCCUPIED";
        case ErrorType::ALREADY:
            return os << "ALREADY";
        case ErrorType::TIMEOUT:
            return os << "TIMEOUT";
        case ErrorType::NO_CS_PIN:
            return os << "NO_CS_PIN";
        case ErrorType::NO_ACK:
            return os << "NO_ACK";
        default:
            return os << "UNKNOWN";
    }
}

void BusBase::Locker::lock(const uint8_t index){
    sys::Exception::disableInterrupt();
    oninterrupt_ = sys::Exception::isIntrruptActing();
    req = index >> 1;
    locked_ = true;
    sys::Exception::enableInterrupt();
}

bool BusBase::Locker::owned_by(const uint8_t index) const {
    return (req == index >> 1) and (sys::Exception::isIntrruptActing() == oninterrupt_);
}



BusError BusBase::begin(const uint8_t index){
    if(false == locker.locked()){
        locker.lock(index);
        return lead(index);
    }else if(locker.owned_by(index)){
        locker.lock(index);
        return lead(index);
    }else{
        return BusError::OCCUPIED;
    }
}

BusError BusBase::end(){
    this->trail();
    locker.unlock();

    return BusError::OK;
}

// void Bus::lock(const uint8_t index){
//     if(locker == nullptr) HALT;

// }

// void Bus::unlock(){
//     if(locker == nullptr) HALT;
    
// }

// bool Bus::locked(){
//     if(locker == nullptr) HALT;
//     return 
// }

// bool Bus::owned_by(const uint8_t index){
//     if(locker == nullptr) HALT;
//     return 
// }
