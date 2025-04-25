#pragma once

#include "core/platform.hpp"

#include "bus_enums.hpp"

#include <optional>

namespace ymd::hal{


struct BusError{
public:
    enum Kind:uint8_t{
        AlreadyUnderUse = 0,
        OccuipedByOther,
        AckTimeout,
        BusOverload,
        SelecterOutOfRange,
        NoSelecter,
        PayloadNoLength,
        VerifyFailed,
        LengthOverflow,
        Unspecified = 0x7f
    };
    
    static consteval BusError Ok(){
        return {std::nullopt};
    }

    constexpr BusError(std::nullopt_t):kind_(std::nullopt){;}
    constexpr BusError(const Kind & kind):kind_(kind){;}
    constexpr BusError(Kind && kind):kind_(kind){;}
    constexpr BusError(const BusError & other):kind_(other.kind_){;}
    constexpr BusError(BusError && other):kind_(other.kind_){;}
    constexpr BusError & operator = (const BusError & other) = default;
    constexpr BusError & operator = (BusError && other) = default;
    __fast_inline constexpr BusError & emplace(const BusError & other){kind_ = other.kind_; return *this;}
    __fast_inline constexpr BusError & emplace(BusError && other){kind_ = other.kind_; return *this;}

    constexpr bool operator ==(const Kind kind){return kind_.has_value() and (kind_.value() == kind);}
    constexpr bool operator !=(const Kind kind){return kind_.has_value() or (kind_.value() != kind);}


    constexpr bool operator ==(const BusError & other){return kind_ == other.kind_;}
    constexpr bool operator !=(const BusError & other){return kind_ != other.kind_;}


    __fast_inline constexpr bool is_err() const {return unlikely(kind_.has_value());}
    __fast_inline constexpr bool is_ok() const {return likely(!kind_.has_value());}

    // 链式处理
    template<typename Fn>
    BusError then(Fn && fn){
        if (is_ok()) return std::forward<Fn>(fn)();
        return *this;
    }

    BusError operator | (const BusError rhs) const{
        if(is_err()) return *this;
        else return rhs;
    }

    // constexpr explicit operator Kind() const {return kind_.value();}
    // constexpr Kind kind() const {return kind_.value();}

    constexpr Kind unwrap_err() const {return kind_.value();}

private:
    std::optional<Kind> kind_;
};


class LockRequest{
public:
    constexpr explicit LockRequest(const uint32_t payload, const uint32_t custom_len):
        payload_(payload), 
        custom_len_(custom_len){}

    uint32_t custom() const {
        return ((1 << custom_len_) - 1) & payload_; 
    }

    uint32_t id() const {
        return (payload_ >> custom_len_);
    }

    size_t custom_len() const {
        return custom_len_;
    }

    uint32_t as_u32() const {
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
        Locker(){;}

        ~Locker(){
            unlock();
        }

        void lock(const LockRequest req);

        void unlock(){
            locked_ = false;
        }

        bool is_owned_by(const LockRequest req) const;

        bool is_locked() const {
            return locked_;
        }
    };

    Locker __own_locker__ = {};
    Locker & locker;

    virtual BusError lead(const LockRequest req) = 0;
    virtual void trail() = 0;

    struct _Guard{
        BusBase & bus_;
        
        _Guard(BusBase & bus):
        bus_(bus){;}
        ~_Guard(){
            bus_.end();
        }
    };
public:
    BusBase():locker(__own_locker__){;}

    virtual ~BusBase(){;}
    
    BusBase(const BusBase &) = delete;
    BusBase(BusBase &&) = default;

    BusError begin(const LockRequest req);

    BusError end();

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

namespace ymd{
    OutputStream & operator << (OutputStream & os, const hal::BusError & err);

    OutputStream & operator << (OutputStream & os, const hal::BusError::Kind & err);
}