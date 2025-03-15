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
    BusError writeRepeat_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t len, 
        const Endian endian){
            return writeTemplate(addr, endian, [&]() -> BusError{return this->writeHomoPayload(data, len, endian);});
        }

    BusError writeBurst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pdata, 
        const Endian endian){
            return writeTemplate(addr, endian, [&]() -> BusError{return this->writePayload(pdata, endian);});}

    BusError readBurst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pdata,
        const Endian endian){
            return readTemplate(addr, endian, [&]() -> BusError{return this->readPayload(pdata, endian);});}

    template<typename T, typename... Ts>    //TODO 改写为Y组合子
    __fast_inline
    BusError writePayloads(Endian endian, std::span<std::add_const_t<T>> first, std::span<std::add_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return BusError::OK;
        else{BusError err = writePayload(first, endian);
            return err.wrong() ? err : writePayloads<Ts...>(endian, rest...);}
    }

    template<typename T, typename... Ts>   //TODO 改写为Y组合子
    __fast_inline
    BusError readPayloads(Endian endian, std::span<std::remove_const_t<T>> first, std::span<std::remove_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return BusError::OK;
        else{BusError err = readPayload(first, endian);
            return err.wrong() ? err : readPayloads<Ts...>(endian, rest...);}
    }

    BusError writePayload(std::span<const valid_i2c_data auto> pdata,const Endian endian){
        return iterate_bytes(
            pdata, endian, 
            [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },
            [](const BusError err) -> bool {return err.wrong();},
            []() -> BusError {return BusError::OK;}
        );
    }

    template<valid_i2c_data Tfirst, valid_i2c_data ... Trest>
    BusError operatePayloads(std::span<Tfirst> pfirst, std::span<Trest>... prest, const Endian endian){
        if constexpr(std::is_const_v<Tfirst>) {
            if(const auto err = this->writePayload(pfirst, endian); err.wrong()) return err;}
        else {if(const auto err = this->readPayload(pfirst, endian); err.wrong()) return err;}

        if constexpr(sizeof...(Trest)) return this->operatePayloads<Trest...>(prest..., endian);
        else return BusError::OK;

    }
    
    BusError writeHomoPayload(const valid_i2c_data auto data, const size_t len, const Endian endian){
        return iterate_bytes(
            data, len, endian, 
            [&](const std::byte byte, const bool is_end) -> BusError{ return bus_.write(uint32_t(byte)); },
            [](const BusError err) -> bool {return err.wrong();},
            []() -> BusError {return BusError::OK;}
        );
    }
    
    BusError readPayload(
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
    BusError writeTemplate(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        const auto guard = createGuard();
        if(const auto begin_err = bus_.begin(index_); begin_err.ok()){
            if(const auto err = writePayload(std::span(&addr, 1), endian); err.wrong()){
                return err;
            }
            return std::forward<Fn>(fn)();
        }else{
            return begin_err;
        }
    }

    template<typename Fn>
    BusError readTemplate(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        return writeTemplate(addr, endian, [&]() -> BusError{
            if(const auto reset_err = bus_.begin(index_ | 0x01); reset_err.ok()){
                return std::forward<Fn>(fn)();
            }else{
                return reset_err;
            }
        });
    }

public:
    I2cDrv(hal::I2c & _bus, const uint8_t _index):
        ProtocolBusDrv<I2c>(_bus, _index){};

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError writeBurst(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata
    ){
        return writeBurst_impl(addr, pdata, LSB);
    }
        
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError writeBurst(
        const valid_i2c_regaddr auto addr, 
        std::span<const T> pdata,
        const Endian endian
    ){
        return writeBurst_impl(addr, pdata, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError readBurst(
        const valid_i2c_regaddr auto addr,
        std::span<T> pdata
    ){
        return this->readBurst_impl(addr, pdata, LSB);

    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError readBurst(
        const valid_i2c_regaddr auto addr,
        std::span<T> pdata,
        const Endian endian
    ){
        return this->readBurst_impl(addr, pdata, endian);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    BusError writeBlocks(
        const valid_i2c_regaddr auto addr, 
        std::span<std::add_const_t<Ts>> ... args,
        const Endian endian
    ){
        return writeTemplate(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(args)) return this->writePayloads<Ts...>(endian, args...);
            else return BusError::OK;
        });
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    BusError readBlocks(
        const valid_i2c_regaddr auto addr, 
        std::span<std::remove_const_t<Ts>> ... args,
        const Endian endian
    ){
        return readTemplate(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(args)) return this->readPayloads<Ts...>(endian, args...);
            else return BusError::OK;
        });
    }

    template<typename ... Trest>
    [[nodiscard]] __fast_inline
    BusError operateBlocks(
        const valid_i2c_regaddr auto addr, 
        std::span<Trest> ... rest,
        const Endian endian
    ){
        return readTemplate(addr, endian, [&]() -> BusError{
            if constexpr(sizeof...(Trest)) return this->operatePayloads<Trest...>(rest..., endian);
            else return BusError::OK;
        });
    }
    
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError writeRepeat(
        const valid_i2c_regaddr auto addr,
        const T data, 
        const size_t len, 
        const Endian endian
    ){
        return writeRepeat_impl(addr, data, len, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError writeRepeat(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t len
    ){
        return writeRepeat_impl(addr, data, len, LSB);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError writeReg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->writeBurst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError writeReg(
        const valid_i2c_regaddr auto addr, 
        const T & data
    ){
        return this->writeBurst_impl(addr, std::span(&data, 1), LSB);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->readBurst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    BusError readReg(
        const valid_i2c_regaddr auto addr,
        T & data
    ){
        return this->readBurst_impl(addr, std::span(&data, 1), LSB);
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

