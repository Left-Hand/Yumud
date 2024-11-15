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
private:

protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;
private:

    template<typename T>
    requires valid_i2c_regaddr<T>
    Bus::Error writeRegAddress(const T reg_address, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    void writeMulti_impl(const valid_i2c_regaddr auto reg_address, const T data, const size_t length, const Endian endian);

    template<typename T>
    requires valid_i2c_data<T>
    void writeMulti_impl(const valid_i2c_regaddr auto reg_address, const T * data_ptr, const size_t length, const Endian endian);


    template<typename T>
    requires valid_i2c_data<T>
    void readMulti_impl(const valid_i2c_regaddr auto reg_address, T * data_ptr, const size_t length, const Endian endian);

public:
    I2cDrv(I2c & _bus, const uint8_t _index):
        BusDrv<I2c>(_bus, _index){};


    template<typename T>
    requires valid_i2c_data<T>
    void writeMulti(const valid_i2c_regaddr auto reg_address, const T * data_ptr, const size_t length, const Endian endian){
        writeMulti_impl<T>(reg_address, data_ptr, length, endian);
    }


    template<typename T>
    requires valid_i2c_data<T>
    void writeMulti(const valid_i2c_regaddr auto reg_address, const T data, const size_t length, const Endian endian){
        writeMulti_impl<T>(reg_address, data, length, endian);
    }


    template<typename T>
    requires valid_i2c_data<T>
    void writeReg(const valid_i2c_regaddr auto reg_address, const T data, const Endian endian){
        writeMulti<T>(reg_address, &data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T>
    void readMulti(const valid_i2c_regaddr auto reg_address, T * data_ptr, const size_t length, const Endian endian){
        readMulti_impl<T>(reg_address, data_ptr, length, endian);
    }

    template<typename T>
    requires valid_i2c_data<T>
    void readReg(const valid_i2c_regaddr auto reg_address, T & reg_data, Endian endian){
        readMulti<T>(reg_address, &reg_data, 1, endian);
    }

    template<typename T>
    requires valid_i2c_data<T>
    T readReg(const valid_i2c_regaddr auto reg_address, Endian endian){
        T reg_data;
        readMulti<T>(reg_address, &reg_data, 1, endian);
        return reg_data;
    }
};


namespace internal{
template <>
struct DrvOfBus<I2c>{
    using DrvType = I2cDrv;  // 默认错误类型
};

}

}


#include "i2cdrv.tpp"