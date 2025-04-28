#pragma once

#include "core/platform.hpp"

#include "bus_enums.hpp"
#include "bus_error.hpp"

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

class BusBase{
private:

    class Locker final{
    private:
        uint32_t req_id_:29 = 0;
        uint32_t is_read:1 = false;
        uint32_t oninterrupt_:1 = false;
        uint32_t locked_:1 = false;
    public:
        Locker(const Locker & other) = delete;
        Locker(Locker && other) = delete;
        __fast_inline Locker(){;}

        __fast_inline ~Locker(){
            unlock();
        }

        void lock(const LockRequest req);

        __fast_inline void unlock(){
            locked_ = false;
        }

        bool is_owned_by(const LockRequest req) const;

        __fast_inline bool is_locked() const {
            return locked_;
        }
    };

    Locker __own_locker__ = {};
    Locker & locker;

    virtual HalResult lead(const LockRequest req) = 0;
    virtual void trail() = 0;

    struct _Guard{
        BusBase & bus_;
        
        __fast_inline _Guard(BusBase & bus):
            bus_(bus){;}
        __fast_inline ~_Guard(){
            bus_.end();
        }
    };
public:
    BusBase():locker(__own_locker__){;}

    virtual ~BusBase(){;}
    
    BusBase(const BusBase &) = delete;
    BusBase(BusBase &&) = default;

    HalResult begin(const LockRequest req);

    void end();

    _Guard create_guard(){return _Guard{*this};}

    bool occupied(){return locker.is_locked();}
};

template<typename TBus>
concept is_bus = std::is_base_of_v<BusBase, TBus>;

template<typename TBus>
concept is_writable_bus = requires(TBus bus, const uint32_t data) {
    bus.write(data);
};

template<typename TBus>
concept is_readable_bus = requires(TBus bus, uint32_t & data, Ack need_ack) {
    bus.read(data);
};

template<typename TBus>
concept is_fulldup_bus = is_writable_bus<TBus> && is_readable_bus<TBus>;

template <typename TBus>
struct driver_of_bus {
    using driver_type = void;
};

};
