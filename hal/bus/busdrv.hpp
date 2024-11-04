#pragma once

#include "bus.hpp"
#include <type_traits>
#include <initializer_list>


namespace yumud{

template<typename BusType>
concept is_bus = std::is_base_of_v<Bus, BusType>;

template<typename BusType>
concept is_writable_bus = std::is_base_of_v<WritableBus, BusType>;

template<typename BusType>
concept is_readable_bus = std::is_base_of_v<ReadableBus, BusType>;

template<typename BusType>
concept is_fulldup_bus = std::is_base_of_v<FullDuplexBus, BusType>;


namespace internal{
template <typename BusType>
struct DrvOfBus {
    using DrvType = void;
};

}


template<typename BusType>
class BusDrv{
public:
    BusType & bus;
protected:
    uint8_t index;
    uint8_t data_bits = 8;
    uint32_t wait_time = 10;

    void setDataBits(const size_t _data_bits){
        bus.setDataBits(_data_bits);
    }
                                                                                                                           
    BusDrv(BusType & _bus, const uint8_t _index):bus(_bus), index(_index){;}
};

template <typename BusType>
class NonProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index;
    using BusDrv<BusType>::bus;

public:
    NonProtocolBusDrv(BusType & _bus, const uint8_t _index) : BusDrv<BusType>(_bus, _index) {}

    template<typename T>
    requires std::is_standard_layout_v<T> and is_writable_bus<BusType>
    void writeSingle(const T data, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    void writeMulti(const is_stdlayout auto & data, const size_t len, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
    void writeMulti(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    void readMulti(T * data_ptr, const size_t len, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
    void readSingle(T & data, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> and is_fulldup_bus<BusType>
    void transferSingle(T & datarx, T datatx, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T> && is_fulldup_bus<BusType>
    T transferSingle(T datatx, Continuous cont = DISC);
};

template <typename BusType, typename = std::enable_if_t<std::is_base_of_v<ProtocolBus, BusType>>>
class ProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index;
    using BusDrv<BusType>::bus;
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

