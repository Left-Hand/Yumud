#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <span>
#include <bitset>

#include "core/utils/BytesIterator.hpp"
#include "core/utils/Result.hpp"

#include "i2c.hpp"

namespace ymd::hal{

class I2cDrv final{

public:

    explicit I2cDrv(Some<I2c *> i2c, I2cSlaveAddr<7> addr):
        i2c_(i2c.deref()),
        slave_addr_(addr){;}

    I2c & bus(){return i2c_;}
    HalResult set_baudrate(const uint32_t baud){
        return i2c_.set_baudrate(baud);
    }
private:

    I2c & i2c_;
    I2cSlaveAddr<7> slave_addr_;

    uint16_t timeout_ = 10;
    HalResult write_repeat_impl(
        const valid_i2c_regaddr auto addr, 
        const valid_i2c_data auto data, 
        const size_t len, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> HalResult{return this->write_homo_payload(data, len, endian);});
        }

    HalResult write_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<const valid_i2c_data auto> pbuf, 
        const Endian endian){
            return write_template(addr, endian, [&]() -> HalResult{return this->write_payload(pbuf, endian);});}

    HalResult read_burst_impl(
        const valid_i2c_regaddr auto addr, 
        std::span<valid_i2c_data auto> pbuf,
        const Endian endian){
            return read_template(addr, endian, [&]() -> HalResult{return this->read_payload(pbuf, endian);});}

    template<typename T, typename... Ts>    //TODO 改写为Y组合子
    __fast_inline
    HalResult write_payloads(Endian endian, const std::span<std::add_const_t<T>> first, std::span<std::add_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return HalResult::Ok();
        else{HalResult res = write_payload(first, endian);
            return res.is_err() ? res : write_payloads<Ts...>(endian, rest...);}
    }

    template<typename T, typename... Ts>   //TODO 改写为Y组合子
    __fast_inline
    HalResult read_payloads(Endian endian, const std::span<std::remove_const_t<T>> first, std::span<std::remove_const_t<Ts>>... rest) {
        if constexpr (sizeof...(Ts) == 0) return HalResult::Ok();
        else{HalResult res = read_payload(first, endian);
            return res.is_err() ? res : read_payloads<Ts...>(endian, rest...);}
    }

    template<valid_i2c_data T>
    HalResult write_payload(const std::span<const T> pbuf,const Endian endian){
        return iterate_bytes(
            pbuf, endian, 
            [&](const uint8_t byte, const bool is_end) -> HalResult{ return i2c_.write(uint32_t(byte)); },
            [](const HalResult res) -> bool {return res.is_err();},
            []() -> HalResult {return HalResult::Ok();}
        );
    }

    template<valid_i2c_data Tfirst, valid_i2c_data ... Trest>
    HalResult operate_payloads(const std::span<Tfirst> pfirst, const std::span<Trest>... prest, const Endian endian){
        if constexpr(std::is_const_v<Tfirst>) {
            if(const auto res = this->write_payload(pfirst, endian); 
                res.is_err()) return res;
        }else {
            if(const auto res = this->read_payload(pfirst, endian); 
                res.is_err()) return res;
        }

        if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(prest..., endian);
        else return HalResult::Ok();

    }
    
    HalResult write_homo_payload(const valid_i2c_data auto data, const size_t len, const Endian endian){
        return iterate_bytes(
            data, len, endian, 
            [&](const uint8_t byte, const bool is_end) -> HalResult{ return i2c_.write(uint32_t(byte)); },
            [](const HalResult res) -> bool {return res.is_err();},
            []() -> HalResult {return HalResult::Ok();}
        );
    }
    
    HalResult read_payload(
        const std::span<valid_i2c_data auto> pbuf,
        const Endian endian
    ){
        return iterate_bytes(
            pbuf, endian, 
            [&](uint8_t & byte, const bool is_end) -> HalResult{
                auto res = i2c_.read(byte, is_end ? NACK : ACK); return res;},
            [](const HalResult res) -> bool {return res.is_err();},
            []() -> HalResult {return HalResult::Ok();}
        );
        
    }

    template<typename Fn>
    HalResult write_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        const auto guard = i2c_.create_guard();
        if(const auto res = i2c_.borrow(slave_addr_.with_write()); res.is_err()) return res;
        if(const auto res = this->write_payload(std::span(&addr, 1), endian); res.is_err()) return res;
        return std::forward<Fn>(fn)();
    }

    template<typename Fn>
    HalResult read_template(
        const valid_i2c_regaddr auto addr,
        const Endian endian,
        Fn && fn
    ){
        return write_template(addr, endian, [&]() -> HalResult{
            if(const auto reset_err = i2c_.borrow(slave_addr_.with_read()); reset_err.is_ok()){
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
    HalResult write_burst(
        const valid_i2c_regaddr auto addr, 
        const std::span<const T> pbuf,
        const Endian endian = LSB
    ){
        return write_burst_impl(addr, pbuf, endian);
    }
        
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    HalResult write_burst(
        const valid_i2c_regaddr auto addr, 
        const std::span<const T> pbuf,
        const Endian endian
    ){
        return write_burst_impl(addr, pbuf, endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    HalResult read_burst(
        const valid_i2c_regaddr auto addr,
        const std::span<T> pbuf,
        const Endian endian = LSB
    ){
        return this->read_burst_impl(addr, pbuf, endian);

    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    HalResult read_burst(
        const valid_i2c_regaddr auto addr,
        const std::span<T> pbuf,
        const Endian endian
    ){
        return this->read_burst_impl(addr, pbuf, endian);
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    HalResult write_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<std::add_const_t<Ts>> ... args,
        const Endian endian
    ){
        return write_template(addr, endian, [&]() -> HalResult{
            if constexpr(sizeof...(args)) return this->write_payloads<Ts...>(endian, args...);
            else return HalResult::Ok();
        });
    }

    template<typename ... Ts>
    [[nodiscard]] __fast_inline
    HalResult read_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<std::remove_const_t<Ts>> ... args,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> HalResult{
            if constexpr(sizeof...(args)) return this->read_payloads<Ts...>(endian, args...);
            else return HalResult::Ok();
        });
    }

    template<typename ... Trest>
    [[nodiscard]] __fast_inline
    HalResult operate_blocks(
        const valid_i2c_regaddr auto addr, 
        const std::span<Trest> ... rest,
        const Endian endian
    ){
        return read_template(addr, endian, [&]() -> HalResult{
            if constexpr(sizeof...(Trest)) return this->operate_payloads<Trest...>(rest..., endian);
            else return HalResult::Ok();
        });
    }
    
    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    HalResult write_repeat(
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
    HalResult write_repeat(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const size_t len,
        const Endian endian = LSB
    ){
        return write_repeat_impl(addr, data, len, endian);
    }


    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    HalResult write_reg(
        const valid_i2c_regaddr auto addr, 
        const T data, 
        const Endian endian
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    HalResult write_reg(
        const valid_i2c_regaddr auto addr, 
        const T & data,
        const Endian endian = LSB
    ){
        return this->write_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) != 1)
    [[nodiscard]] __fast_inline
    HalResult read_reg(
        const valid_i2c_regaddr auto addr,
        T & data, 
        Endian endian
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), endian);
    }

    template<typename T>
    requires valid_i2c_data<T> and (sizeof(T) == 1)
    [[nodiscard]] __fast_inline
    HalResult read_reg(
        const valid_i2c_regaddr auto addr,
        T & data,
        const Endian endian = LSB
    ){
        return this->read_burst_impl(addr, std::span(&data, 1), endian);
    }

    [[nodiscard]]
    HalResult validate();

    [[nodiscard]]
    HalResult release();
};

}

