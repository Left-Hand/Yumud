#pragma once

#include "core/platform.hpp"

#include "bus_enums.hpp"
#include "hal/hal_result.hpp"
#include "core/system.hpp"

namespace ymd::hal{


class LockRequest{
public:
    constexpr explicit LockRequest(const uint32_t payload, const uint32_t custom_len):
        payload_(payload), 
        custom_len_(custom_len){}

    constexpr uint32_t custom() const {
        return ((1 << custom_len_) - 1) & payload_; 
    }

    constexpr uint32_t id() const {
        return (payload_ >> custom_len_);
    }

    constexpr size_t custom_len() const {
        return custom_len_;
    }

    constexpr uint32_t as_u32() const {
        return std::bit_cast<uint32_t>(*this);
    }
private:
    uint32_t payload_:29;
    uint32_t custom_len_:3;
};


class BusLocker final{
private:
    uint32_t req_id_:29 = 0;
    uint32_t is_read:1 = false;
    uint32_t oninterrupt_:1 = false;
    uint32_t locked_:1 = false;
public:
    BusLocker(const BusLocker & other) = delete;
    BusLocker(BusLocker && other) = delete;
    __fast_inline BusLocker(){;}

    __fast_inline ~BusLocker(){
        unlock();
    }

    void lock(const LockRequest req){
        sys::exception::disable_interrupt();
        oninterrupt_ = sys::exception::is_intrrupt_acting();
        req_id_ = req.id();
        locked_ = true;
        sys::exception::enable_interrupt();
    }

    __fast_inline void unlock(){
        locked_ = false;
    }

    bool is_borrowed_by(const LockRequest req) const{
        return ((req_id_ == req.id()) 
            and (sys::exception::is_intrrupt_acting() == oninterrupt_));
    }

    __fast_inline bool is_borrowed() const {
        return locked_;
    }
};

template <typename TBus>
struct driver_of_bus {
    using driver_type = void;
};

};
