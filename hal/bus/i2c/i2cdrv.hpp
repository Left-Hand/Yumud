#pragma once

#include "i2c.hpp"
#include "sys/debug/debug_inc.h"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>


namespace ymd{

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
    Bus::Error writeRegAddress(const T addr, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    Bus::Error writeSame_impl(const valid_i2c_regaddr auto addr, const T data, const size_t length, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    Bus::Error writeMulti_impl(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length, const Endian endian);


    template<typename T>
    requires valid_i2c_data<T>
    Bus::Error readMulti_impl(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length, const Endian endian);

public:
    I2cDrv(I2c & _bus, const uint8_t _index):
        BusDrv<I2c>(_bus, _index){};


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    Bus::Error writeMulti(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length, const Endian endian){
        return writeMulti_impl<T>(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    Bus::Error writeMulti(const valid_i2c_regaddr auto addr, const T * data_ptr, const size_t length){
        return writeMulti_impl<T>(addr, data_ptr, length, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    Bus::Error writeSame(const valid_i2c_regaddr auto addr, const T data, const size_t length, const Endian endian){
        return writeSame_impl<T>(addr, data, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    Bus::Error writeSame(const valid_i2c_regaddr auto addr, const T data, const size_t length){
        return writeSame_impl<T>(addr, data, length, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    Bus::Error readMulti(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length, const Endian endian){
        return this->readMulti_impl<T>(addr, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    Bus::Error readMulti(const valid_i2c_regaddr auto addr, T * data_ptr, const size_t length){
        return this->readMulti_impl<T>(addr, data_ptr, length, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    Bus::Error writeReg(const valid_i2c_regaddr auto addr, const T data, const Endian endian){
        // this->writeMulti_impl<T>
        return this->writeMulti_impl<T>(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    Bus::Error writeReg(const valid_i2c_regaddr auto addr, const T & data){
        return this->writeMulti_impl<T>(addr, &data, 1, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    Bus::Error readReg(const valid_i2c_regaddr auto addr, T & data, Endian endian){
        return this->readMulti_impl<T>(addr, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    Bus::Error readReg(const valid_i2c_regaddr auto addr, T & data){
        return this->readMulti_impl<T>(addr, &data, 1, LSB);
    }

    __inline bool verify();
    __inline void release();
};


namespace internal{
template <>
struct DrvOfBus<I2c>{
    using DrvType = I2cDrv;  // 默认错误类型
};

}

}


#include "i2cdrv.tpp"