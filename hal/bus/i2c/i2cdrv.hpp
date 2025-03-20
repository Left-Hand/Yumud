#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <span>


#include "i2c.hpp"
#include "hal/bus/busdrv.hpp"
#include "core/utils/BytesIterator.hpp"



namespace ymd::hal{

template <typename T>
concept valid_i2c_regaddr = std::integral<T> and (sizeof(T) <= 2) and std::is_unsigned_v<T>;

template <typename T>
concept valid_i2c_data = std::is_standard_layout_v<T> and (sizeof(T) <= 4);


class I2cDrv:public ProtocolBusDrv<I2c> {
protected:
    uint8_t index_r_ = 0;
    BusError write_repeat_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t len, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> BusError{return this->write_homo_payload(data, len, endian);});
        }

    BusError write_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pdata, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> BusError{return this->write_payload(pdata, endian);});}

    BusError read_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pdata,
        const Endian endian){
            return read_template(addr, endian, [&]() -> BusError{return this->read_payload(pdata, endian);});}

    template<typename T, typename... Ts>    //TODO 改写为Y组合子
    __fast_inline
    BusError write_payloads(Endian endian, std::span<std::add_const_t<T>> first, std::span<std::add_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return BusError::OK;
        else{BusError err = write_payload(first, endian);
            return err.wrong() ? err : write_payloads<Ts...>(endian, rest...);}
    }

    template<typename T, typename... Ts>   //TODO 改写为Y组合子
    __fast_inline
    BusError read_payloads(Endian endian, std::span<std::remove_const_t<T>> first, std::span<std::remove_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return BusError::OK;
        else{BusError err = read_payload(first, endian);
            return err.wrong() ? err : read_payloads<Ts...>(endian, rest...);}
    }

    BusError write_payload(std::span<const valid_i2c_data auto> pdata,const Endian endian){
        return iterate_bytes(
            pdata, endian, 
            [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },
            [](const BusError err) -> bool {return err.wrong();},
            []() -> BusError {return BusError::OK;}
        );
    }

    template<valid_i2c_data Tfirst, valid_i2c_data ... Trest>
    BusError operate_payloads(std::span<Tfirst> pfirst, std::span<Trest>... prest, const Endian endian){
        if constexpr(std::is_const_v<Tfirst>) {
            if(const auto err = this->write_payload(pfirst, endian); err.wrong()) return err;}
        else {if(const auto err = this->read_payload(pfirst, endian); err.wrong()) return err;}

        if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(prest..., endian);
        else return BusError::OK;

    }
    
    BusError write_homo_payload(const valid_i2c_data auto data, const size_t len, const Endian endian){
        return iterate_bytes(
            data, len, endian, 
            [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },
            [](const BusError err) -> bool {return err.wrong();},
            []() -> BusError {return BusError::OK;}
        );
    }
    
    BusError read_payload(
        std::span<valid_i2c_data auto> pdata,
        const Endian endian
    ){
        return iterate_bytes(
            pdata, endian, 
            [&](std::byte & byte, const bool is_end) -> BusError{
                uint32_t dummy = 0; auto err = bus_.read(dummy, is_end ? NACK : ACK); byte = std::byte(dummy); return err;},
            [](const BusError err) -> bool {return err.wrong();},
            []() -> BusError {return BusError::OK;}
        );
        
    }

    template<typename Fn>
    BusError write_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        const auto guard = createGuard();
        if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
            if(const auto err = write_payload(std::span(&addr, 1), endian); err.wrong()){
                return err;
            }
            return std::forward<Fn>(fn)();
        }else{
            return begin_err;
        }
    }

    template<typename Fn>
    BusError read_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        return write_template(addr, endian, [&]() -> BusError{
            if(const auto reset_err = bus_.begin(index_r_); reset_err.ok()){
                return std::forward<Fn>(fn)();
            }else{
                return reset_err;
            }
        });
    }

public:
    I2cDrv(hal::I2c & i2c, const uint8_t index):
        I2cDrv(i2c, index, index | 0x01){;};

    I2cDrv(hal::I2c & i2c, const uint8_t index, const uint8_t index_r):
        ProtocolBusDrv<I2c>(i2c, index), index_r_(index_r){;};

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError write_burst(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata
    ){
        return write_burst_impl(addr, pdata, LSB);
    }
        
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError write_burst(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata,
        const Endian endian
    ){
        return write_burst_impl(addr, pdata, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError read_burst(
        const valid_i2c_regaddr auto addr,
        std::span<T> pdata
    ){
        return this->read_burst_impl(addr, pdata, LSB);

    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError read_burst(
        const valid_i2c_regaddr auto addr,
        std::span<T> pdata,
        const Endian endian
    ){
        return this->read_burst_impl(addr, pdata, endian);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    BusError write_blocks(
        const valid_i2c_regaddr auto addr, 
        std::span<std::add_const_t<Ts>> ... args,
        const Endian endian
    ){
        return write_template(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(args)) return this->write_payloads<Ts...>(endian, args...);
            else return BusError::OK;
        });
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    BusError read_blocks(
        const valid_i2c_regaddr auto addr, 
        std::span<std::remove_const_t<Ts>> ... args,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(args)) return this->read_payloads<Ts...>(endian, args...);
            else return BusError::OK;
        });
    }

    template<typename ... Trest>
    [[nodiscard]] __fast_inline
    BusError operate_blocks(
        const valid_i2c_regaddr auto addr, 
        std::span<Trest> ... rest,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(rest..., endian);
            else return BusError::OK;
        });
    }
    
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError write_repeat(
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
    BusError write_repeat(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t len
    ){
        return write_repeat_impl(addr, data, len, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError write_reg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError write_reg(
        const valid_i2c_regaddr auto addr, 
        const T & data
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError read_reg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError read_reg(
        const valid_i2c_regaddr auto addr,
        T & data
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), LSB);
    }

    [[nodiscard]]
    BusError verify();

    [[nodiscard]]
    BusError release();
};

template<>
struct driver_of_bus<hal::I2c>{
    using driver_type = hal::I2cDrv;
};


}

