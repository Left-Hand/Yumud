#ifndef __BUSDRV_HPP__
#define __BUSDRV_HPP__

#include "spi/spi.hpp"
#include "i2c/i2c.hpp"
#include "i2s/i2s.hpp"
#include <type_traits>
#include <initializer_list>

#include "i2c/i2csw.hpp"

template<typename BusType>
class BusDrv{
public:
    BusType & bus;
protected:
    uint8_t index;
    uint8_t data_bits = 8;
    uint32_t wait_time;

    static constexpr auto is_writable_bus = std::is_base_of_v<WritableBus, BusType>;
    static constexpr auto is_readable_bus = std::is_base_of_v<ReadableBus, BusType>;
    static constexpr auto is_fulldup_bus = std::is_base_of_v<FullDuplexBus, BusType>;

    void setDataBits(const size_t _data_bits){
        // if(_data_bits == data_bits) return;
        // else{
        //     data_bits = _data_bits;
            bus.setDataBits(_data_bits);
        // }  
    }

    virtual void speclize(){;}                                                                                                                             
    BusDrv(BusType & _bus, const uint8_t _index, const uint32_t _wait_time = 320):bus(_bus), index(_index), wait_time(_wait_time){;}
public:

    template<typename T>
    requires (std::is_integral_v<T> || std::is_enum_v<T>) && is_writable_bus
    void write(const T data, bool ends = true){
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

            if(ends) bus.end();
            if (size != 1) this->setDataBits(8);
        }

    }
    template<typename T>
    requires is_writable_bus
    void write(std::initializer_list<T> datas, bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            for(auto data_item : datas) bus.write(data_item);
            if(ends) bus.end();
            if (sizeof(T) != 1) this->setDataBits(8);
        }
    }

    template<typename T, typename U = T>
    requires std::is_integral<T>::value && is_writable_bus
    void write(const T data, const size_t len, bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(U) != 1) this->setDataBits(sizeof(U) * 8);
            for(size_t i = 0; i < len; i++) bus.write(U(data));
            if (ends) bus.end();
            if (sizeof(U) != 1) this->setDataBits(8);
        }
    }

    template<typename T, typename A = T, typename B = A>
    requires std::is_integral<T>::value && is_writable_bus
    void write(const T * data_ptr, const size_t len, bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(B) != 1) this->setDataBits(sizeof(B) * 8);
            for(size_t i = 0; i < len; i++) bus.write(B(A(data_ptr[i])));
            if (ends) bus.end();
            if (sizeof(B) != 1) this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_readable_bus
    void read(T * data_ptr, const size_t len, const bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            for(size_t i = 0; i < len; i++){
                uint32_t temp = 0;
                bus.read(temp, (i != len - 1));
                data_ptr[i] = temp;
            }
            if(ends) bus.end();
            if (sizeof(T) != 1)this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_readable_bus
    void read(T & data, const bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            uint32_t temp;
            bus.read(temp);
            data = temp;
            if(ends) bus.end();
            if (sizeof(T) != 1)this->setDataBits(8);
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_fulldup_bus
    void transfer(T & datarx, T datatx, bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if (sizeof(T) != 1)this->setDataBits(8);
            if(ends) bus.end();
        }
    }

    template<typename T>
    requires std::is_integral<T>::value && is_fulldup_bus
    T transfer(T datatx, bool ends = true){
        if(!bus.begin(index)){
            if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
            T datarx;
            uint32_t ret = 0;
            bus.transfer(ret, datatx);
            datarx = ret;
            if (sizeof(T) != 1)this->setDataBits(8);
            if(ends) bus.end();
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
    ProtocolBusDrv(BusType & _bus, const uint8_t _index, const uint32_t _wait_time = 320):
        BusDrv<BusType>(_bus, _index, _wait_time){};

public:
    void writePool(const uint8_t reg_address, const uint8_t * data_ptr, const size_t size, const size_t length, const Endian msb = MSB){
        if(length == 0) return;
        if(!bus.begin(index)){
            bus.write(reg_address);

            for(size_t i = 0; i < length; i += size){
                if(bool(msb)){
                    for(size_t j = size; j > 0; j--){
                        bus.write(data_ptr[j-1 + i]);
                    }
                }else{
                    for(size_t j = 0; j < size; j++){
                        bus.write(data_ptr[j + i]);
                    }
                }
            }

            bus.end();
        }
    }

    void readPool(const uint8_t reg_address, uint8_t * data_ptr, const size_t size, const size_t length, const Endian msb = MSB){
        if(length == 0) return;
        if(!bus.begin(index)){
            bus.write(reg_address);
            if(!bus.begin(index | 0x01)){
                for(size_t i = 0; i < length; i += size){
                    if(bool(msb)){
                        for(size_t j = size; j > 0; j--){
                            uint32_t temp = 0;
                            bus.read(temp, !((j == 1) && (i == length - size)));
                            data_ptr[j-1 + i] = temp;
                        }
                    }else{
                        for(size_t j = 0; j < size; j++){
                            uint32_t temp = 0;
                            bus.read(temp, (i + j != length - 1));
                            data_ptr[j + i] = temp;
                        }
                    }
                }
            }
            bus.end();
        }
    }

    void readReg(const uint8_t reg_address,uint16_t & reg, Endian msb = MSB){
        uint8_t buf[2] = {0};
        readPool(reg_address, buf, 2, 2, msb);
        reg = buf[1] << 8 | buf[0];
    }

    void readReg(const uint8_t reg_address, uint8_t & reg_data){
        if(!bus.begin(index)){
            bus.write(reg_address);
            bus.begin(index | 0x01);
            uint32_t temp;
            bus.read(temp, false);
            reg_data = temp;
            bus.end();
        }
    }

    void writeReg(const uint8_t reg_address,  const uint16_t reg_data, Endian msb = MSB){
        writePool(reg_address, (uint8_t *)&reg_data, 2, 2, msb);
    }


    void writeReg(const uint8_t reg_address,  const uint8_t reg_data){
        if(!bus.begin(index)){
            bus.write(reg_address);
            bus.write(reg_data);
            bus.end();
        }
    }
};

template <typename BusType, typename = std::enable_if_t<std::is_base_of_v<PackedBus, BusType>>>
class PackedBusDrv : public BusDrv<BusType> {
protected:
    using Packet = BusType::Packet;

};

#endif