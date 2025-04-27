#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <span>
#include <bitset>


#include "i2c.hpp"
#include "core/utils/BytesIterator.hpp"

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

class I2cDrv final{
private:
    I2c & i2c_;
    I2cSlaveAddr<7> slave_addr_;

    // uint8_t data_width_ = 8;
    uint16_t timeout_ = 10;
public:

    I2cDrv(I2c & i2c, I2cSlaveAddr<7> addr):
        i2c_(i2c),
        slave_addr_(addr){;}

    I2c & bus(){return i2c_;}
    void set_baudrate(const uint baud){
        i2c_.set_baudrate(baud);
    }
private:

    hal::BusError write_repeat_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t len, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> hal::BusError{return this->write_homo_payload(data, len, endian);});
        }

    hal::BusError write_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pdata, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> hal::BusError{return this->write_payload(pdata, endian);});}

    hal::BusError read_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pdata,
        const Endian endian){
            return read_template(addr, endian, [&]() -> hal::BusError{return this->read_payload(pdata, endian);});}

    template<typename T, typename... Ts>    //TODO 改写为Y组合子
    __fast_inline
    hal::BusError write_payloads(Endian endian, const std::span<std::add_const_t<T>> first, std::span<std::add_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return hal::BusError::Ok();
        else{hal::BusError err = write_payload(first, endian);
            return err.is_err() ? err : write_payloads<Ts...>(endian, rest...);}
    }

    template<typename T, typename... Ts>   //TODO 改写为Y组合子
    __fast_inline
    hal::BusError read_payloads(Endian endian, const std::span<std::remove_const_t<T>> first, std::span<std::remove_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return hal::BusError::Ok();
        else{hal::BusError err = read_payload(first, endian);
            return err.is_err() ? err : read_payloads<Ts...>(endian, rest...);}
    }

    template<valid_i2c_data T>
    hal::BusError write_payload(const std::span<const T> pdata,const Endian endian){
        // if constexpr (sizeof(T) == 1){
        //     for(size_t i = 0; i < pdata.size(); i++){
        //         const auto err = i2c_.write(uint32_t(pdata[i]));
        //         if(err.is_err()) return err;
        //     }
        //     return hal::BusError::Ok();
        // }
        return iterate_bytes(
            pdata, endian, 
            [&](const std::byte byte, const bool is_end) -> hal::BusError{ return i2c_.write(uint32_t(byte)); },
            [](const hal::BusError err) -> bool {return err.is_err();},
            []() -> hal::BusError {return hal::BusError::Ok();}
        );
    }

    template<valid_i2c_data Tfirst, valid_i2c_data ... Trest>
    hal::BusError operate_payloads(const std::span<Tfirst> pfirst, const std::span<Trest>... prest, const Endian endian){
        if constexpr(std::is_const_v<Tfirst>) {
            if(const auto err = this->write_payload(pfirst, endian); err.is_err()) return err;}
        else {if(const auto err = this->read_payload(pfirst, endian); err.is_err()) return err;}

        if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(prest..., endian);
        else return hal::BusError::Ok();

    }
    
    hal::BusError write_homo_payload(const valid_i2c_data auto data, const size_t len, const Endian endian){
        return iterate_bytes(
            data, len, endian, 
            [&](const std::byte byte, const bool is_end) -> hal::BusError{ return i2c_.write(uint32_t(byte)); },
            [](const hal::BusError err) -> bool {return err.is_err();},
            []() -> hal::BusError {return hal::BusError::Ok();}
        );
    }
    
    hal::BusError read_payload(
        const std::span<valid_i2c_data auto> pdata,
        const Endian endian
    ){
        return iterate_bytes(
            pdata, endian, 
            [&](std::byte & byte, const bool is_end) -> hal::BusError{
                uint32_t dummy = 0; auto err = i2c_.read(dummy, is_end ? NACK : ACK); byte = std::byte(dummy); return err;},
            [](const hal::BusError err) -> bool {return err.is_err();},
            []() -> hal::BusError {return hal::BusError::Ok();}
        );
        
    }

    template<typename Fn>
    hal::BusError write_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        const auto guard = i2c_.create_guard();
        if(const auto err = i2c_.begin(slave_addr_.to_write_req()); err.is_err()) return err;
        if(const auto err = this->write_payload(std::span(&addr, 1), endian); err.is_err()) return err;
        return std::forward<Fn>(fn)();
    }

    template<typename Fn>
    hal::BusError read_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        return write_template(addr, endian, [&]() -> hal::BusError{
            if(const auto reset_err = i2c_.begin(slave_addr_.to_read_req()); reset_err.is_ok()){
                return std::forward<Fn>(fn)();
            }else{
                return reset_err;
            }
        });
    }
public:
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_burst(
        const valid_i2c_regaddr auto addr, 
        const std::span<const T> pdata
    ){
        return write_burst_impl(addr, pdata, LSB);
    }
        
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_burst(
        const valid_i2c_regaddr auto addr, 
        const std::span<const T> pdata,
        const Endian endian
    ){
        return write_burst_impl(addr, pdata, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    hal::BusError read_burst(
        const valid_i2c_regaddr auto addr,
        const std::span<T> pdata
    ){
        return this->read_burst_impl(addr, pdata, LSB);

    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    hal::BusError read_burst(
        const valid_i2c_regaddr auto addr,
        const std::span<T> pdata,
        const Endian endian
    ){
        return this->read_burst_impl(addr, pdata, endian);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    hal::BusError write_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<std::add_const_t<Ts>> ... args,
        const Endian endian
    ){
        return write_template(addr, endian, [&]() -> hal::BusError{
            if constexpr(sizeof...(args)) return this->write_payloads<Ts...>(endian, args...);
            else return hal::BusError::Ok();
        });
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    hal::BusError read_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<std::remove_const_t<Ts>> ... args,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> hal::BusError{
            if constexpr(sizeof...(args)) return this->read_payloads<Ts...>(endian, args...);
            else return hal::BusError::Ok();
        });
    }

    template<typename ... Trest>
    [[nodiscard]] __fast_inline
    hal::BusError operate_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<Trest> ... rest,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> hal::BusError{
            if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(rest..., endian);
            else return hal::BusError::Ok();
        });
    }
    
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_repeat(
        const valid_i2c_regaddr auto addr,
        const T data, 
        const size_t len, 
        const Endian endian
    ){
        return write_repeat_impl(addr, data, len, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_repeat(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t len
    ){
        return write_repeat_impl(addr, data, len, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_reg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    hal::BusError write_reg(
        const valid_i2c_regaddr auto addr, 
        const T & data
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    hal::BusError read_reg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    hal::BusError read_reg(
        const valid_i2c_regaddr auto addr,
        T & data
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), LSB);
    }

    [[nodiscard]]
    hal::BusError verify();

    [[nodiscard]]
    hal::BusError release();
};

template<>
struct driver_of_bus<hal::I2c>{
    using driver_type = hal::I2cDrv;
};


}

