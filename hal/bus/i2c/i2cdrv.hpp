#pragma once

#include "i2c.hpp"
#include "sys/debug/debug_inc.h"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>


namespace ymd::hal{

template <typename T>
concept valid_i2c_regaddr = std::integral<T> and (sizeof(T) <= 2) and std::is_unsigned_v<T>;

template <typename T>
concept valid_i2c_data = std::is_standard_layout_v<T> and (sizeof(T) <= 4);


class I2cDrv : public BusDrv<I2c> {
protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;

    template<typename T>
    requires valid_i2c_regaddr<T>
    BusError writeRegAddress(const T addr, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    BusError writeSame_impl(const valid_i2c_regaddr auto addr, const T data, const size_t length, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    BusError writeMulti_impl(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length, const Endian endian);


    template<typename T>
    requires valid_i2c_data<T>
    BusError readMulti_impl(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length, const Endian endian);

public:
    I2cDrv(I2c & _bus, const uint8_t _index):
        BusDrv<I2c>(_bus, _index){};


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeMulti(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length, const Endian endian){
        return writeMulti_impl<T>(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeMulti(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length){
        return writeMulti_impl<T>(addr, data_ptr, length, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeSame(const valid_i2c_regaddr auto addr, const T data, const size_t length, const Endian endian){
        return writeSame_impl<T>(addr, data, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeSame(const valid_i2c_regaddr auto addr, const T data, const size_t length){
        return writeSame_impl<T>(addr, data, length, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readMulti(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length, const Endian endian){
        return this->readMulti_impl<T>(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readMulti(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length){
        return this->readMulti_impl<T>(addr, data_ptr, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeReg(const valid_i2c_regaddr auto addr, const T data, const Endian endian){
        // this->writeMulti_impl<T>
        return this->writeMulti_impl<T>(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeReg(const valid_i2c_regaddr auto addr, const T & data){
        return this->writeMulti_impl<T>(addr, &data, 1, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readReg(const valid_i2c_regaddr auto addr, T & data, Endian endian){
        return this->readMulti_impl<T>(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readReg(const valid_i2c_regaddr auto addr, T & data){
        return this->readMulti_impl<T>(addr, &data, 1, LSB);
    }

    __inline bool verify();
    __inline void release();
};




}

namespace ymd::internal{
template <>
struct DrvOfBus<ymd::hal::I2c>{
    using DrvType = ymd::hal::I2cDrv;  // 默认错误类型
};

}


#include "i2cdrv.tpp"