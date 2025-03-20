#pragma once

#include "bus.hpp"
#include "core/clock/clock.hpp"
#include <type_traits>
#include <initializer_list>


namespace ymd::hal{

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
concept is_fulldup_bus = std::is_base_of_v<FullDuplexBus, TBus>;


template <typename TBus>
struct driver_of_bus {
    using driver_type = void;
};


template<typename TBus>
class BusDrv{
protected:
    TBus & bus_;
    uint8_t index_;
    uint8_t data_width_ = 8;
    uint16_t timeout_ = 10;

    
    BusDrv(TBus & bus, const uint8_t index):bus_(bus), index_(index){;}
    
    struct _Guard{
        TBus & bus_;
        
        _Guard(TBus & bus):
        bus_(bus){;}
        ~_Guard(){
            bus_.end();
        }
    };
    
    [[nodiscard]] _Guard create_guard(){return _Guard{bus_};}
    public:
    TBus & bus() const {return bus_;}
    auto index() const {return index_;}
    void set_data_width(const size_t data_bits){
        bus_.set_data_width(data_bits);
    }
    
    void set_baudrate(const uint baud){
        bus_.set_baudrate(baud);
    }
};

template <typename TBus>
class NonProtocolBusDrv : public BusDrv<TBus> {
protected:
    using BusDrv<TBus>::index_;
    using BusDrv<TBus>::bus_;

public:
    NonProtocolBusDrv(TBus & _bus, const uint8_t _index) : BusDrv<TBus>(_bus, _index) {}
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
    requires std::is_standard_layout_v<T> and is_writable_bus<TBus>
    [[nodiscard]]
    BusError write_single(const T data, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<TBus>
    [[nodiscard]]
    BusError write_burst(const is_stdlayout auto & data, const size_t len, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<TBus>
    [[nodiscard]]
    BusError write_burst(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<TBus>
    [[nodiscard]]
    BusError read_burst(T * data_ptr, const size_t len, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<TBus>
    [[nodiscard]]
    BusError read_single(T & data, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_fulldup_bus<TBus>
    [[nodiscard]]
    BusError transfer_single(T & data_rx, T data_tx, Continuous cont = DISC);
};

template <typename TBus>
class ProtocolBusDrv : public BusDrv<TBus> {
protected:
    using BusDrv<TBus>::index_;
    using BusDrv<TBus>::bus_;
    ProtocolBusDrv(TBus & _bus, const uint8_t _index):
        BusDrv<TBus>(_bus, _index){};

public:
};



auto operator|(BusDrv<auto> & drv, auto&& fn) -> decltype(auto) {
    return fn(drv);
}

}

#include "NonProtocolBusDrv.tpp"

