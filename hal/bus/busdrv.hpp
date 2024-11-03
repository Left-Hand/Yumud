#pragma once

#include "bus.hpp"
#include <type_traits>
#include <initializer_list>


namespace yumud{
template<typename BusType>
class BusDrv{
public:
    BusType & bus;
protected:
    uint8_t index;
    uint8_t data_bits = 8;
    uint32_t wait_time = 10;

    static constexpr auto is_writable_bus = std::is_base_of_v<WritableBus, BusType>;
    static constexpr auto is_readable_bus = std::is_base_of_v<ReadableBus, BusType>;
    static constexpr auto is_fulldup_bus = std::is_base_of_v<FullDuplexBus, BusType>;

    void setDataBits(const size_t _data_bits){
        bus.setDataBits(_data_bits);
    }
                                                                                                                           
    BusDrv(BusType & _bus, const uint8_t _index):bus(_bus), index(_index){;}
public:

};


template <typename BusType>
class NonProtocolBusDrv : public BusDrv<BusType> {
protected:
    using BusDrv<BusType>::index;
    using BusDrv<BusType>::bus;
    NonProtocolBusDrv(BusType & _bus, const uint8_t _index):
        BusDrv<BusType>(_bus, _index){};

public:

    template<typename T>
    requires (std::is_integral_v<T> || std::is_enum_v<T>) && is_writable_bus
    void writeSingle(const T data, Continuous cont = DISC){
        constexpr size_t size = sizeof(T);
        if(!bus.begin(index)){
            if (size != 1) this->setDataBits(size * 8);
            
            if constexpr(size == 1){
                bus.write((uint8_t)data);
            }else if constexpr(size == 2){
                bus.write((uint16_t)data);
            }else{
                bus.write((uint32_t)data);
            }

            if (cont == DISC) bus.end();
            if (size != 1) this->setDataBits(8);
        }

    }
    template<typename T>
    requires is_writable_bus
    void writeMulti(std::initializer_list<T> datas, Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            for(auto data_item : datas) bus.write(data_item);
            if (cont == DISC) bus.end();
            if (sizeof(T) != 1) this->setDataBits(8);
        }
    }

    template<typename T, typename U = T>
    requires std::is_integral<T>::value && is_writable_bus
    void writeMulti(const T data, const size_t len, Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(U) != 1) this->setDataBits(sizeof(U) * 8);
            for(size_t i = 0; i < len; i++) bus.write(U(data));
            if (cont == DISC) bus.end();
            if (sizeof(U) != 1) this->setDataBits(8);
        }
    }

    template<typename T, typename A = T, typename B = A>
    requires std::is_integral<T>::value && is_writable_bus
    void writeMulti(const T * data_ptr, const size_t len, Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(B) != 1) this->setDataBits(sizeof(B) * 8);
            for(size_t i = 0; i < len; i++) bus.write(B(A(data_ptr[i])));
            if (cont == DISC) bus.end();
            if (sizeof(B) != 1) this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_readable_bus
    void readMulti(T * data_ptr, const size_t len, const Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            if (cont == DISC) bus.end();
            if (sizeof(T) != 1)this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_readable_bus
    void readSingle(T & data, const Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            uint32_t temp;
            bus.read(temp);
            data = temp;
            if (cont == DISC) bus.end();
            if (sizeof(T) != 1)this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_fulldup_bus
    void transferSingle(T & datarx, T datatx, Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if (sizeof(T) != 1)this->setDataBits(8);
            if (cont == DISC) bus.end();
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_fulldup_bus
    T transferSingle(T datatx, Continuous cont = DISC){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            T datarx;
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if (sizeof(T) != 1)this->setDataBits(8);
            if (cont == DISC) bus.end();
            return datarx;
        }
        return T(0);
    }
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