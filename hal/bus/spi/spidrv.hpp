#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>
#include <functional>


#include "spi.hpp"
#include "spihw.hpp"

namespace ymd::hal{

class SpiHw;
class SpiSw;


template <typename T>
concept valid_spi_data = (
    std::is_standard_layout_v<T> 
    and (sizeof(T) <= 2) 
    and std::is_integral_v<T>);

class SpiDrv final{
protected:
    Spi & spi_;
    SpiSlaveIndex idx_;
    Endian endian_ = LSB;  
    uint32_t baudrate_ = 1000000;
    uint8_t last_width_ = -1;

    using WriteFn = std::function<HalResult(uint32_t)>;

    // WriteFn write_fn_;
public:

    // template<typename T>
    // requires (std::is_base_of_v<Spi, T>)
    SpiDrv(Spi & spi, const SpiSlaveIndex idx):
        spi_(spi), 
        idx_(idx)
        // write_fn_([this](const uint32_t data) -> HalResult{return this->spi_.write(data);})
        {;}

    template<typename T>
    void force_write(const T data) {
        constexpr size_t size = sizeof(T);
        if constexpr(size != 1) this->set_data_width(size * 8);

        if constexpr (size == 1) {
            spi_.write(uint8_t(data));
        } else if constexpr (size == 2) {
            spi_.write(uint16_t(data));
        } else {
            spi_.write(uint32_t(data));
        }

        if constexpr(size != 1) this->set_data_width(8);
    }

    
    __fast_inline hal::HalResult set_data_width(const size_t width){
        if(last_width_ != width) return spi_.set_data_width(width);
        else return hal::HalResult::Ok();
    }

    void set_endian(const Endian endian){endian_ = endian;}
    void set_baudrate(const uint32_t baud){baudrate_ = baud;}

    uint8_t index_;
public:
    [[nodiscard]]
    hal::HalResult release(){
        if (auto err = spi_.begin(idx_.to_req()); err.is_ok()) {
            __nopn(4);
            spi_.end();
            return hal::HalResult::Ok();
        }else{
            return err;
        }
    }
    void end(){return spi_.end();}

    template<valid_spi_data T>
    hal::HalResult write_single(const is_stdlayout auto data, Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult write_repeat(const is_stdlayout auto data, const size_t len, Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult write_burst(const is_stdlayout auto * pdata, const size_t len, Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult read_burst(is_stdlayout auto * pdata, const size_t len, const Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult read_single(is_stdlayout auto & data, const Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult transceive_single(T & data_rx, const T data_tx, Continuous cont = DISC);

    template<valid_spi_data T, size_t N>
    hal::HalResult transceive_burst(const std::span<T, N> data_rx, const std::span<const T, N> data_tx, Continuous cont = DISC);

    template<valid_spi_data T>
    hal::HalResult transceive_burst(const std::span<T> data_rx, const std::span<const T> data_tx, Continuous cont = DISC);
};

template<valid_spi_data T>
hal::HalResult SpiDrv::write_single(const is_stdlayout auto data, Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));

    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    if constexpr (sizeof(T) != 1){
        if(const auto res = this->set_data_width(sizeof(T) * 8); res.is_err())
            return res;
    }

    if constexpr (sizeof(T) == 1) {
        if(const auto res = spi_.write(std::bit_cast<uint8_t>(data)); res.is_err()) return res;
    } else if constexpr (sizeof(T) == 2) {
        if(const auto res = spi_.write(std::bit_cast<uint16_t>(data)); res.is_err()) return res;
    }

    if (cont == DISC) spi_.end();
    if constexpr (sizeof(T) != 1) this->set_data_width(8);

    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::write_repeat(const is_stdlayout auto data, const size_t len, Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
    if (const auto err = spi_.begin(idx_.to_req()); err.is_err()) return err; 
    if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
    for (size_t i = 0; i < len; i++){
        if(const auto res = spi_.write(uint32_t(static_cast<T>(data))); 
            res.is_err()) return res;
    }
    if (cont == DISC) spi_.end();
    if constexpr (sizeof(T) != 1) this->set_data_width(8);
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::write_burst(const is_stdlayout auto * pdata, const size_t len, Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(*pdata)>));
    if (const auto err = spi_.begin(idx_.to_req()); err.is_err()) return err; 
    if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
    for (size_t i = 0; i < len; i++){
        if(const auto res = spi_.write(uint32_t(reinterpret_cast<const T *>(pdata)[i]));
            res.is_err()) return res;
    } 
    if (cont == DISC) spi_.end();
    if constexpr (sizeof(T) != 1) this->set_data_width(8);
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::read_burst(is_stdlayout auto * pdata, const size_t len, const Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(*pdata)>));
    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        for (size_t i = 0; i < len; i++) {
            uint32_t temp = 0;
            spi_.read(temp);
            pdata[i] = temp;
        }
        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::read_single(is_stdlayout auto & data, const Continuous cont) {
    static_assert(sizeof(T) == sizeof(std::decay_t<decltype(data)>));
    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t temp = 0;
        spi_.read(temp);
        // data = *reinterpret_cast<std::decay_t<decltype(data)> *>(&temp);
        memcpy(&data, &temp, sizeof(T));
        if (cont == DISC) spi_.end();
        if constexpr (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::HalResult::Ok();
}


template <valid_spi_data T>
hal::HalResult SpiDrv::transceive_single(T & datarx, const T datatx, Continuous cont) {
    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t ret = 0;
        spi_.transceive(ret, datatx);
        datarx = ret;
        if constexpr (sizeof(T) != 1) this->set_data_width(8);
        if (cont == DISC) spi_.end();
    }
    return hal::HalResult::Ok();
}

template<valid_spi_data T, size_t N>
hal::HalResult SpiDrv::transceive_burst(
    const std::span<T, N> pbuf_rx, 
    const std::span<const T, N> pbuf_tx, 
    Continuous cont 
){
    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    {
        if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        for(size_t i = 0; i < N; i++) {
            uint32_t dummy = 0;
            spi_.transceive(dummy, pbuf_tx[i]);
            pbuf_rx[i] = dummy;
        }
        if constexpr (sizeof(T) != 1) this->set_data_width(8);
        if (cont == DISC) spi_.end();
    }
    return hal::HalResult::Ok();
}

template<valid_spi_data T>
hal::HalResult SpiDrv::transceive_burst(
    const std::span<T> pbuf_rx, 
    const std::span<const T> pbuf_tx, 
    Continuous cont
){
    if(const auto res = spi_.begin(idx_.to_req()); res.is_err()) return res;
    {
        const auto N = pbuf_rx.size();
        if constexpr (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t ret = 0;
        for(size_t i = 0; i < N; i++) {
            uint32_t dummy = 0;
            spi_.transceive(dummy, pbuf_tx[i]);
            pbuf_rx[i] = dummy;
        }
        if constexpr (sizeof(T) != 1) this->set_data_width(8);
        if (cont == DISC) spi_.end();
    }
    return hal::HalResult::Ok();
}



template<>
struct driver_of_bus<hal::SpiHw>{
    using driver_type = hal::SpiDrv;
};

}