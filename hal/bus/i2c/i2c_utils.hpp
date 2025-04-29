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

    static constexpr _I2cAddr<N, R> from_u8(const uint8_t i2c_addr){
        return {std::bitset<N>(i2c_addr >> 1)};
    }

    static constexpr _I2cAddr<N, R> from_u7(const uint8_t i2c_addr){
        return {std::bitset<N>(i2c_addr)};
    }

    constexpr LockRequest to_read_req() const {
        return LockRequest(
            (i2c_addr_.to_ulong() << 1) | 0x01, 1);
    }

    constexpr LockRequest to_write_req() const {
        return LockRequest(
            (i2c_addr_.to_ulong() << 1), 1);
    }

    constexpr uint8_t as_u8() const {return i2c_addr_.to_ulong() << 1;}

    constexpr _I2cAddr(const _I2cAddr<N, R> & other) = default;
    constexpr _I2cAddr(_I2cAddr<N, R> && other) = default;
private:
    constexpr _I2cAddr(const std::bitset<N> i2c_addr):i2c_addr_(i2c_addr){;}

    std::bitset<N> i2c_addr_;
};

template<size_t N>
using I2cSlaveAddr = _I2cAddr<N, I2cRole::Slave>;

template<size_t N>
using I2cMasterAddr = _I2cAddr<N, I2cRole::Master>;

}