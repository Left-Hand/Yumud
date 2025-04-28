#include "bus_base.hpp"

#include "core/stream/ostream.hpp"
#include "core/system.hpp"

using namespace ymd;
using namespace ymd::hal;



void BusBase::Locker::lock(const LockRequest req){
    sys::exception::disable_interrupt();
    oninterrupt_ = sys::exception::is_intrrupt_acting();
    req_id_ = req.id();
    locked_ = true;
    sys::exception::enable_interrupt();
}

bool BusBase::Locker::is_owned_by(const LockRequest req) const {
    return 
        ((req_id_ == req.id()) 
        and (sys::exception::is_intrrupt_acting() == oninterrupt_));
}

hal::BusError BusBase::begin(const LockRequest req){
    if(false == locker.is_locked()){
        locker.lock(req);
        return lead(req);
    }else if(locker.is_owned_by(req)){
        locker.lock(req);
        return lead(req);
    }else{
        return hal::BusError::OccuipedByOther;
    }
}

void BusBase::end(){
    this->trail();
    locker.unlock();
}

namespace ymd{

OutputStream & print_buserr_kind(OutputStream & os, const hal::BusError::Kind err){
    using Kind = hal::BusError::Kind;
    #define PRINT_CASE(x) case Kind::x: return os << #x;
    switch(err){
        PRINT_CASE(AlreadyUnderUse);
        PRINT_CASE(OccuipedByOther);
        PRINT_CASE(AckTimeout);
        PRINT_CASE(BusOverload);
        PRINT_CASE(SelecterOutOfRange);
        PRINT_CASE(NoSelecter);
        PRINT_CASE(PayloadNoLength);
        PRINT_CASE(VerifyFailed);
        PRINT_CASE(LengthOverflow);
        PRINT_CASE(Unspecified);
        default: return os << "Unknown";
    }
}

OutputStream & operator << (OutputStream & os, const hal::BusError & err){
    if(err.is_ok()) return os << "Ok";
    else return print_buserr_kind(os, err.unwrap_err());
}

OutputStream & operator << (OutputStream & os, const hal::BusError::Kind & err){
    return print_buserr_kind(os, err);
}
}