#pragma once

#include "i2c.hpp"
#include "hal/bus/busdrv.hpp"
#include "utils/BytesIterator.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <span>


namespace ymd::hal{

template <typename T>
concept valid_i2c_regaddr = std::integral<T> and (sizeof(T) <= 2) and std::is_unsigned_v<T>;

template <typename T>
concept valid_i2c_data = std::is_standard_layout_v<T> and (sizeof(T) <= 4);


class I2cDrv:public ProtocolBusDrv<I2c> {
protected:

    BusError writeRegAddress(
        const valid_i2c_regaddr auto addr, 
        const Endian endian);

    BusError writeSame_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t length, 
        const Endian endian);


    BusError writeMulti_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto * data_ptr, 
        const size_t length, 
        const Endian endian);

    BusError readMulti_impl(
        const valid_i2c_regaddr auto addr, 
        valid_i2c_data auto * data_ptr, 
        const size_t length, 
        const Endian endian);

    BusError writeBurst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pdata,
        const Endian endian);

    BusError readBurst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pdata,
        const Endian endian);

    BusError writeCommand_impl(
        const valid_i2c_regaddr auto cmd, 
        const Endian endian);
public:
    I2cDrv(hal::I2c & _bus, const uint8_t _index):
        ProtocolBusDrv<I2c>(_bus, _index){};

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeMulti(
        const valid_i2c_regaddr auto addr, 
        const T * data_ptr, 
        const size_t length, 
        const Endian endian
    ){
        return writeMulti_impl(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeMulti(
        const valid_i2c_regaddr auto addr, 
        const T * data_ptr, 
        const size_t length
    ){
        return writeMulti_impl(addr, data_ptr, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeBurst(
        const valid_i2c_regaddr auto addr, 
        const T * data_ptr, 
        const size_t length
    ){
        return writeMulti_impl(addr, data_ptr, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeSame(
        const valid_i2c_regaddr auto addr,
        const T data, 
        const size_t length, 
        const Endian endian
    ){
        return writeSame_impl(addr, data, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeSame(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t length
    ){
        return writeSame_impl(addr, data, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T>
    BusError writeBody(const T data, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readMulti(
        const valid_i2c_regaddr auto addr, 
        T * data_ptr, 
        const size_t length, 
        const Endian endian
    ){
        return this->readMulti_impl(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readMulti(
        const valid_i2c_regaddr auto addr,
        T * data_ptr, 
        const size_t length
    ){
        return this->readMulti_impl(addr, data_ptr, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeReg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->writeMulti_impl(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_regaddr<T> and (sizeof(T) == 1)
    BusError writeCommand(const T cmd){
        return writeCommand_impl(cmd, LSB);
    }

    template<typename T>
    requires valid_i2c_regaddr<T> and (sizeof(T) != 1)
    BusError writeCommand(
        const T cmd, 
        const Endian endian
    ){
        return writeCommand_impl(cmd, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeReg(
        const valid_i2c_regaddr auto addr, 
        const T & data
    ){
        return this->writeMulti_impl(addr, &data, 1, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->readMulti_impl(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data
    ){
        return this->readMulti_impl(addr, &data, 1, LSB);
    }

    BusError verify();
    BusError release();
};

template<>
struct driver_of_bus<hal::I2c>{
    using driver_type = hal::I2cDrv;
};


}


#include "i2cdrv.tpp"