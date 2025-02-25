#pragma once

#include "bus.hpp"
#include "sys/clock/clock.h"
#include <type_traits>
#include <initializer_list>


namespace ymd{

template<typename BusType>
concept is_bus = std::is_base_of_v<Bus, BusType>;

template<typename BusType>
concept is_writable_bus = std::is_base_of_v<WritableBus, BusType>;

template<typename BusType>
concept is_readable_bus = std::is_base_of_v<ReadableBus, BusType>;

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
    uint32_t wait_time = 10;

    void setDataBits(const size_t _data_bits){
        bus_.setDataBits(_data_bits);
    }
                                                                                                                           
    BusDrv(BusType & bus, const uint8_t index):bus_(bus), index_(index){;}
public:
    auto & bus(){return bus_;}
    auto index(){return index_;}
};

template <typename BusType>
class NonProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index_;
    using BusDrv<BusType>::bus_;

public:
    NonProtocolBusDrv(BusType & _bus, const uint8_t _index) : BusDrv<BusType>(_bus, _index) {}
    void release(){
        if (BusError::OK == bus_.begin(index_) ) {
            delay(1);
            bus_.end();
        }
    }
    void end(){bus_.end();}

    template<typename T>
    requires std::is_standard_layout_v<T> and is_writable_bus<BusType>
    BusError writeSingle(const T data, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    BusError writeMulti(const is_stdlayout auto & data, const size_t len, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    BusError writeMulti(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    BusError readMulti(T * data_ptr, const size_t len, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    BusError readSingle(T & data, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_fulldup_bus<BusType>
    BusError transferSingle(T & datarx, T datatx, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> && is_fulldup_bus<BusType>
    T transferSingle(T datatx, Continuous cont = DISC);
};

template <typename BusType, typename = std::enable_if_t<std::is_base_of_v<ProtocolBus, BusType>>>
class ProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index_;
    using BusDrv<BusType>::bus_;
    ProtocolBusDrv(BusType & _bus, const uint8_t _index):
        BusDrv<BusType>(_bus, _index){};

public:
};

template <typename BusType, typename = std::enable_if_t<std::is_base_of_v<PackedBus, BusType>>>
class PackedBusDrv : public BusDrv<BusType> {
protected:
    using Packet = BusType::Packet;
};
}


#include "NonProtocolBusDrv.tpp"

