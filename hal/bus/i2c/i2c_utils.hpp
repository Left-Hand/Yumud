#pragma once

#include "core/platform.hpp"
#include "hal/bus/bus_base.hpp"
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

template<size_t N, I2cRole R>
class _I2cAddr{
public:
    static constexpr _I2cAddr<N, R> from_u7(const uint8_t i2c_addr){
        return _I2cAddr<N, R>{uint16_t(i2c_addr)};
    }

    constexpr LockRequest to_read_req() const {
        return LockRequest(
            (i2c_addr_ << 1) | 0x01, 1);
    }

    constexpr LockRequest to_write_req() const {
        return LockRequest(
            (i2c_addr_ << 1), 1);
    }

    constexpr uint8_t to_u7() const {return i2c_addr_;}

    constexpr _I2cAddr(const _I2cAddr<N, R> & other) = default;
    constexpr _I2cAddr(_I2cAddr<N, R> && other) = default;
private:
    constexpr explicit _I2cAddr(const uint16_t i2c_addr):i2c_addr_(i2c_addr){;}

    uint16_t i2c_addr_;
};


template<size_t N, I2cRole R>
class _I2cAddrWithRw{
public:
    static constexpr _I2cAddrWithRw<N, R> from_u8(const uint8_t raw){
        return _I2cAddrWithRw<N, R>(static_cast<uint16_t>(raw));
    }

    static constexpr _I2cAddrWithRw<N, R> from_read(const uint8_t i2c_addr){
        return _I2cAddrWithRw<N, R>(static_cast<uint16_t>(i2c_addr << 1) | 0x01);
    }

    static constexpr _I2cAddrWithRw<N, R> from_write(const uint8_t i2c_addr){
        return _I2cAddrWithRw<N, R>(static_cast<uint16_t>(i2c_addr << 1));
    }

    constexpr uint8_t to_u8() const {
        return static_cast<uint8_t>(std::bit_cast<uint16_t>(*this));}
    
    constexpr bool is_read() const {return is_read_;}
    constexpr bool is_write() const {return !is_read_;}
    constexpr _I2cAddrWithRw(const _I2cAddrWithRw<N, R> & other) = default;
    constexpr _I2cAddrWithRw(_I2cAddrWithRw<N, R> && other) = default;
private:
    constexpr explicit _I2cAddrWithRw(const uint16_t raw):
        is_read_(raw & 0x01),
        i2c_addr_(raw >> 1){;}
    uint16_t is_read_ : 1;
    uint16_t i2c_addr_ : 15;
};


template<size_t N>
using I2cSlaveAddr = _I2cAddr<N, I2cRole::Slave>;

template<size_t N>
using I2cMasterAddr = _I2cAddr<N, I2cRole::Master>;

}