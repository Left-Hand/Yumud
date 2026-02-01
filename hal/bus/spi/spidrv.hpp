#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <functional>

#include "spi.hpp"

namespace ymd::hal{

class Spi;
class SoftSpi;


template <typename T>
concept valid_spi_data = (
    std::is_standard_layout_v<T> 
    and (sizeof(T) <= 2) 
    and std::is_integral_v<T>);

class SpiDrv final{
public:

    SpiDrv(Some<Spi *> spi, const SpiSlaveRank rank):
        spi_(spi.deref()),
        rank_(rank)
        {;}

    template<typename T>
    void force_write(const T data) {
        constexpr size_t size = sizeof(T);
        if constexpr(size != 1) this->set_wordsize(SpiWordSize::TwoBytes);

        if constexpr (size == 1) {
            spi_.blocking_write(uint8_t(data));
        } else if constexpr (size == 2) {
            spi_.blocking_write(uint16_t(data));
        } else {
            spi_.blocking_write(uint32_t(data));
        }

        if constexpr(size != 1) this->set_wordsize(SpiWordSize::OneByte);
    }

    
    __fast_inline hal::HalResult set_wordsize(const SpiWordSize wordsize){
        return spi_.set_wordsize(wordsize);
    }

    void set_endian(const std::endian endian){endian_ = endian;}
    void set_baudrate(const uint32_t baud){baudrate_ = baud;}

public:
    [[nodiscard]]
    hal::HalResult release(){
        if (auto res = spi_.borrow(rank_); 
            res.is_err()) return res;
        __nopn(4);
        spi_.lend();
        return hal::HalResult::Ok();
    }
    void lend(){return spi_.lend();}

    template<valid_spi_data T>
    hal::HalResult write_single(const is_stdlayout auto data, Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult write_repeat(
        const is_stdlayout auto data, const size_t len, 
        Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult write_burst(
        const std::span<const is_stdlayout auto> pbuf, 
        Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult read_burst(
        const std::span<is_stdlayout auto> pbuf, 
        const Continuous cont = DISC);

    template<valid_spi_data T, size_t N, is_stdlayout U>
    hal::HalResult write_burst(
        const std::span<const U, N> pbuf, 
        Continuous cont = DISC
    ){
        return this->write_burst<T>(std::span<const U>(pbuf), cont);
    }

    template<valid_spi_data T, size_t N, is_stdlayout U>
    hal::HalResult read_burst(
        const std::span<U, N> pbuf, 
        const Continuous cont = DISC
    ){
        return this->read_burst<T>(std::span<U>(pbuf), cont);
    }

    template<valid_spi_data T>
    hal::HalResult read_single(is_stdlayout auto & data, const Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult transceive_single(
        T & data_rx, const T data_tx, Continuous cont = DISC);

    template<valid_spi_data T, size_t N>
    hal::HalResult transceive_burst(
        const std::span<T, N> data_rx, 
        const std::span<const T, N> data_tx, 
        Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult transceive_burst(
        const std::span<T> data_rx, 
        const std::span<const T> data_tx, 
        Continuous cont = DISC);

private:
    Spi & spi_;
    SpiSlaveRank rank_;
    std::endian endian_ = std::endian::little;  
    uint32_t baudrate_ = 1000000;

    using WriteFn = std::function<HalResult(uint32_t)>;
};

template<valid_spi_data T>
hal::HalResult SpiDrv::write_single(const is_stdlayout auto data, Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;
    if constexpr (sizeof(T) != 1){
        if(const auto res = this->set_wordsize(SpiWordSize::TwoBytes); 
            res.is_err()) return res;
    }

    if constexpr (sizeof(T) == 1) {
        spi_.blocking_write(std::bit_cast<uint8_t>(data)); 
    } else if constexpr (sizeof(T) == 2) {
        spi_.blocking_write(std::bit_cast<uint16_t>(data)); 
    }

    if (cont == DISC) spi_.lend();
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);

    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::write_repeat(const is_stdlayout auto data, const size_t len, Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
    if (const auto res = spi_.borrow(rank_); 
        res.is_err()) return res; 
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
    for (size_t i = 0; i < len; i++){
        spi_.blocking_write(uint32_t(static_cast<T>(data))); 
    }
    if (cont == DISC) spi_.lend();
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
    return hal::HalResult::Ok();
}


template <valid_spi_data T, is_stdlayout U>
hal::HalResult SpiDrv::write_burst(
        const std::span<const U> pbuf, 
        Continuous cont
) {
    static_assert(sizeof(T) == sizeof(U));
    if (const auto res = spi_.borrow(rank_); 
        res.is_err()) return res; 
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
    for (size_t i = 0; i < pbuf.size(); i++){
        spi_.blocking_write(uint32_t(pbuf[i]));
    } 
    if (cont == DISC) spi_.lend();
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
    return hal::HalResult::Ok();
}


template <valid_spi_data T, is_stdlayout U>
hal::HalResult SpiDrv::read_burst(
        const std::span<U> pbuf, 
        const Continuous cont
) {
    static_assert(sizeof(T) == sizeof(U));
    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;

    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
    for (size_t i = 0; i < pbuf.size(); i++) {
        uint32_t temp = 0;
        temp = spi_.blocking_read();
        pbuf[i] = temp;
    }
    if (cont == DISC) spi_.lend();
    if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::read_single(is_stdlayout auto & data, const Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
        const uint32_t temp = spi_.blocking_read();
        // memcpy(&data, &temp, sizeof(T));
        data = static_cast<T>(temp);
        if (cont == DISC) spi_.lend();
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
    }
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::transceive_single(T & datarx, const T datatx, Continuous cont) {
    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
        uint32_t ret = 0;
        ret = spi_.blocking_transceive(datatx);
        datarx = ret;
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
        if (cont == DISC) spi_.lend();
    }
    return hal::HalResult::Ok();
}

template<valid_spi_data T, size_t N>
hal::HalResult SpiDrv::transceive_burst(
    const std::span<T, N> pbuf_rx, 
    const std::span<const T, N> pbuf_tx, 
    Continuous cont 
){
    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
        for(size_t i = 0; i < N; i++) {
            uint32_t dummy = 0;
            dummy = spi_.blocking_transceive(pbuf_tx[i]);
            pbuf_rx[i] = dummy;
        }
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
        if (cont == DISC) spi_.lend();
    }
    return hal::HalResult::Ok();
}

template<valid_spi_data T>
hal::HalResult SpiDrv::transceive_burst(
    const std::span<T> pbuf_rx, 
    const std::span<const T> pbuf_tx, 
    Continuous cont
){
    if(const auto res = spi_.borrow(rank_); 
        res.is_err()) return res;
    {
        const auto N = pbuf_rx.size();
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::TwoBytes);
        for(size_t i = 0; i < N; i++) {
            uint32_t dummy = 0;
            dummy = spi_.blocking_transceive(pbuf_tx[i]);
            pbuf_rx[i] = dummy;
        }
        if constexpr (sizeof(T) != 1) this->set_wordsize(SpiWordSize::OneByte);
        if (cont == DISC) spi_.lend();
    }
    return hal::HalResult::Ok();
}


}