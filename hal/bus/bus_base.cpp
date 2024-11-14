#include "bus_base.hpp"
#include "sys/core/system.hpp"
#include "sys/debug/debug_inc.h"


using namespace ymd;


OutputStream & ymd::operator << (OutputStream & os, const Bus::Error & err){
    return os << err.type;
}

OutputStream & ymd::operator << (OutputStream & os, const Bus::ErrorType & err){
    using ErrorType = Bus::ErrorType;

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

void Bus::Locker::lock(const uint8_t index){
    Sys::Exception::disableInterrupt();
    oninterrupt_ = Sys::Exception::isIntrruptActing();
    req = index >> 1;
    locked_ = true;
    Sys::Exception::enableInterrupt();
}


bool Bus::Locker::owned_by(const uint8_t index) const {
    return (req == index >> 1) and (Sys::Exception::isIntrruptActing() == oninterrupt_);
}


void Bus::lock(const uint8_t index){
    if(locker == nullptr) HALT;
    locker->lock(index);
}

void Bus::unlock(){
    if(locker == nullptr) HALT;
    locker->unlock();
}

bool Bus::locked(){
    if(locker == nullptr) HALT;
    return locker->locked();
}

bool Bus::owned_by(const uint8_t index){
    if(locker == nullptr) HALT;
    return locker->owned_by(index);
}
