#pragma once

#include "bus.hpp"
#include "sys/clock/clock.h"
#include <type_traits>
#include <initializer_list>


namespace ymd::hal{

template<typename BusType>
concept is_bus = std::is_base_of_v<BusBase, BusType>;

template<typename BusType>
concept is_writable_bus = requires(BusType bus, const uint32_t data) {
    bus.write(data);
};

template<typename BusType>
concept is_readable_bus = requires(BusType bus, uint32_t & data, Ack need_ack) {
    bus.read(data);
};

template<typename BusType>
concept is_fulldup_bus = std::is_base_of_v<FullDuplexBus, BusType>;


template <typename BusType>
struct driver_of_bus {
    using driver_type = void;
};


template<typename BusType>
class BusDrv{
protected:
    BusType & bus_;
    uint8_t index_;
    uint8_t data_bits = 8;
    uint16_t wait_time = 10;

    
    BusDrv(BusType & bus, const uint8_t index):bus_(bus), index_(index){;}
    
    struct _Guard{
        BusType & bus_;
        
        _Guard(BusType & bus):
        bus_(bus){;}
        ~_Guard(){
            bus_.end();
        }
    };
    
    [[nodiscard]] _Guard createGuard(){return _Guard{bus_};}
    public:
    BusType & bus() const {return bus_;}
    auto index() const {return index_;}
    void set_data_width(const size_t _data_bits){
        bus_.set_data_width(_data_bits);
    }
    
    void setBaudRate(const uint baud){
        bus_.setBaudRate(baud);
    }
};

template <typename BusType>
class NonProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index_;
    using BusDrv<BusType>::bus_;

public:
    NonProtocolBusDrv(BusType & _bus, const uint8_t _index) : BusDrv<BusType>(_bus, _index) {}
    [[nodiscard]]
    BusError release(){
        if (auto err = bus_.begin(index_); err.ok()) {
            delay(1);
            bus_.end();
            return BusError::OK;
        }else{
            return err;
        }
    }
    void end(){bus_.end();}

    template<typename T>
    requires std::is_standard_layout_v<T> and is_writable_bus<BusType>
    [[nodiscard]]
    BusError writeSingle(const T data, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    [[nodiscard]]
    BusError write_burst(const is_stdlayout auto & data, const size_t len, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    [[nodiscard]]
    BusError write_burst(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    [[nodiscard]]
    BusError read_burst(T * data_ptr, const size_t len, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    [[nodiscard]]
    BusError readSingle(T & data, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_fulldup_bus<BusType>
    [[nodiscard]]
    BusError transferSingle(T & data_rx, T data_tx, Continuous cont = DISC);
};

template <typename BusType>
class ProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index_;
    using BusDrv<BusType>::bus_;
    ProtocolBusDrv(BusType & _bus, const uint8_t _index):
        BusDrv<BusType>(_bus, _index){};

public:
};



auto operator|(BusDrv<auto> & drv, auto&& fn) -> decltype(auto) {
    return fn(drv);
}

}

#include "NonProtocolBusDrv.tpp"

