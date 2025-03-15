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
    BusError writeSame_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t length, 
        const Endian endian);


    BusError writeMulti_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pdata, 
        const Endian endian);

    BusError readMulti_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pdata,
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

    BusError writeBody(const valid_i2c_data auto data, const Endian endian);
    BusError readBody(const valid_i2c_data auto data, const Endian endian);
public:
    I2cDrv(hal::I2c & _bus, const uint8_t _index):
        ProtocolBusDrv<I2c>(_bus, _index){};

    BusError writeRegAddress(
        const valid_i2c_regaddr auto addr, 
        const Endian endian);
        
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeMulti(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata,
        const Endian endian
    ){
        return writeMulti_impl(addr, pdata, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeMulti(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata
    ){
        return writeMulti_impl(addr, pdata, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeBurst(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata
    ){
        return writeMulti_impl(addr, pdata, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeSame(
        const valid_i2c_regaddr auto addr,
        const T data, 
        const size_t len, 
        const Endian endian
    ){
        return writeSame_impl(addr, data, len, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError writeSame(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t len
    ){
        return writeSame_impl(addr, data, len, LSB);
    }



    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readMulti(
        const valid_i2c_regaddr auto addr, 
        std::span<T> pdata,
        const Endian endian
    ){
        return this->readMulti_impl(addr, pdata, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readMulti(
        const valid_i2c_regaddr auto addr,
        std::span<T> pdata
    ){
        return this->readMulti_impl(addr, pdata, LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError writeReg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->writeMulti_impl(addr, std::span(&data, 1), endian);
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
        return this->writeMulti_impl(addr, std::span(&data, 1), LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->readMulti_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data
    ){
        return this->readMulti_impl(addr, std::span(&data, 1), LSB);
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