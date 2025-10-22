#pragma once

#include "core/platform.hpp"
#include "hal/bus/bus_base.hpp"
#include "hal/hal_result.hpp"
#include <bitset>

namespace ymd::hal{
template <typename T>
concept valid_i2c_regaddr = std::integral<T> and (sizeof(T) <= 2) and std::is_unsigned_v<T>;

template <typename T>
concept valid_i2c_data = std::is_standard_layout_v<T> and (sizeof(T) <= 4);

enum class I2cRole:uint8_t{
    Master,
    Slave
};


template<I2cRole R>
struct I2cAddrWithRw{
public:
    static constexpr uint16_t LSB_READ = 0x01;
    static constexpr uint16_t LSB_WRITE = 0x00;

    [[nodiscard]] static constexpr I2cAddrWithRw<R> from_7bits_read(const uint8_t i2c_addr){
        return I2cAddrWithRw<R>(static_cast<uint16_t>(i2c_addr << 1) | LSB_READ, 7);
    }

    [[nodiscard]] static constexpr I2cAddrWithRw<R> from_7bits_write(const uint8_t i2c_addr){
        return I2cAddrWithRw<R>(static_cast<uint16_t>(i2c_addr << 1) | LSB_WRITE, 7);
    }

    [[nodiscard]] static constexpr I2cAddrWithRw<R> from_8bits(const uint8_t i2c_raw_addr){
        return I2cAddrWithRw<R>(i2c_raw_addr, 7);
    }

    constexpr I2cAddrWithRw(const I2cAddrWithRw<R> & other) = default;
    constexpr I2cAddrWithRw(I2cAddrWithRw<R> && other) = default;

    [[nodiscard]] constexpr bool is_read() const {return is_read_;}
    [[nodiscard]] constexpr bool is_write() const {return !is_read_;}

    [[nodiscard]] constexpr uint16_t addr_without_rw() const {return i2c_addr_;}

    [[nodiscard]] constexpr uint16_t as_unique_id() const {
        switch(width_){
            //正常情况使用rw位，i2c地址和宽度生成唯一码
            case 7:
            case 10:
                return std::bit_cast<uint16_t>(*this) & 0xfffe;
            //如果直接从原始比特构造(伪I2c) 那么忽略rw位
            case 8:
            case 11:
                return std::bit_cast<uint16_t>(*this);
            default: __builtin_unreachable();
        }
    }
    
    [[nodiscard]] constexpr size_t width() const {
        return width_;
    }

private:
    constexpr explicit I2cAddrWithRw(const uint16_t raw_addr, const size_t width):
        is_read_(raw_addr & 0x01),
        i2c_addr_(raw_addr >> 1),
        width_(width){;}
    uint16_t is_read_ : 1;
    uint16_t i2c_addr_ : 9;
    uint16_t width_ : 6;
};

template<size_t N, I2cRole R>
struct I2cAddr;

template<size_t N, I2cRole R>
struct I2cAddr;

template<I2cRole R>
struct I2cAddr<7, R>{
public:
    static constexpr I2cAddr<7, R> from_u7(const uint16_t i2c_addr){
        return I2cAddr<7, R>{uint16_t(i2c_addr)};
    }

    constexpr I2cAddrWithRw<R> with_read() const {
        return I2cAddrWithRw<R>::from_7bits_read(i2c_addr_);
    }

    constexpr I2cAddrWithRw<R> with_write() const {
        return I2cAddrWithRw<R>::from_7bits_write(i2c_addr_);
    }

    [[nodiscard]] constexpr uint16_t as_u7() const {return i2c_addr_;}


    constexpr I2cAddr(const I2cAddr<7, R> & other) = default;
    constexpr I2cAddr(I2cAddr<7, R> && other) = default;
private:
    constexpr explicit I2cAddr(const uint16_t i2c_addr):i2c_addr_(i2c_addr){;}

    uint16_t i2c_addr_;
};


template<I2cRole R>
struct I2cAddr<10, R>{
public:
    static constexpr I2cAddr<10, R> from_u10(const uint16_t i2c_addr){
        return I2cAddr<10, R>{uint16_t(i2c_addr)};
    }

    [[nodiscard]] constexpr uint16_t as_u10() const {return i2c_addr_;}

    constexpr I2cAddr(const I2cAddr<10, R> & other) = default;
    constexpr I2cAddr(I2cAddr<10, R> && other) = default;
private:
    constexpr explicit I2cAddr(const uint16_t i2c_addr):i2c_addr_(i2c_addr){;}

    uint16_t i2c_addr_;
};



template<size_t N>
using I2cSlaveAddr = I2cAddr<N, I2cRole::Slave>;

template<size_t N>
using I2cMasterAddr = I2cAddr<N, I2cRole::Master>;

using I2cSlaveAddrWithRw = I2cAddrWithRw<I2cRole::Slave>;

}