#pragma once

#include <type_traits>
#include <concepts>
#include <initializer_list>


#include "spi.hpp"

namespace ymd::hal{



class SpiDrv final{
protected:
    Spi & spi_;
    SpiSlaveIndex idx_;
    Endian endian_ = LSB;  
    uint32_t baudrate_ = 1000000;
public:
    SpiDrv(hal::Spi & spi, const SpiSlaveIndex idx):
        spi_(spi), idx_(idx){;}

    template<typename T>
    void force_write(const T data) {
        constexpr size_t size = sizeof(T);
        if (size != 1) this->set_data_width(size * 8);

        if constexpr (size == 1) {
            spi_.write(uint8_t(data));
        } else if constexpr (size == 2) {
            spi_.write(uint16_t(data));
        } else {
            spi_.write(uint32_t(data));
        }

        if (size != 1) this->set_data_width(8);
    }
    
    void set_data_width(const size_t width){}
    void set_endian(const Endian endian){endian_ = endian;}
    void set_baudrate(const uint32_t baud){baudrate_ = baud;}

    uint8_t index_;
public:
    [[nodiscard]]
    hal::BusError release();
    void end(){spi_.end();}

    template<typename T>
    requires std::is_standard_layout_v<T>
    [[nodiscard]]
    hal::BusError write_single(const T data, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U>
    [[nodiscard]]
    hal::BusError write_burst(const is_stdlayout auto & data, const size_t len, Continuous cont = DISC);

    template<typename U>
    requires std::is_standard_layout_v<U>
    [[nodiscard]]
    hal::BusError write_burst(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T>
    [[nodiscard]]
    hal::BusError read_burst(T * data_ptr, const size_t len, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T>
    [[nodiscard]]
    hal::BusError read_single(T & data, const Continuous cont = DISC);

    template<typename T>
    requires std::is_standard_layout_v<T>
    [[nodiscard]]
    hal::BusError transfer_single(T & data_rx, T data_tx, Continuous cont = DISC);
};

template<typename T>
requires std::is_standard_layout_v<T>
hal::BusError SpiDrv::write_single(const T data, Continuous cont) {
    constexpr size_t size = sizeof(T);
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (size != 1) this->set_data_width(size * 8);

        if constexpr (size == 1) {
            spi_.write(uint8_t(data));
        } else if constexpr (size == 2) {
            spi_.write(uint16_t(data));
        } else {
            spi_.write(uint32_t(data));
        }

        if (cont == DISC) spi_.end();
        if (size != 1) this->set_data_width(8);
    }

    return hal::BusError::Ok();
}

template <typename U>
requires std::is_standard_layout_v<U>
hal::BusError SpiDrv::write_burst(const is_stdlayout auto & data, const size_t len, Continuous cont) {
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (sizeof(U) != 1) this->set_data_width(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) spi_.write(static_cast<U>(data));
        if (cont == DISC) spi_.end();
        if (sizeof(U) != 1) this->set_data_width(8);
    }
    return hal::BusError::Ok();
}

template <typename U>
requires std::is_standard_layout_v<U>
hal::BusError SpiDrv::write_burst(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont) {
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (sizeof(U) != 1) this->set_data_width(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) spi_.write(static_cast<U>(data_ptr[i]));
        if (cont == DISC) spi_.end();
        if (sizeof(U) != 1) this->set_data_width(8);
    }
    return hal::BusError::Ok();
}

template<typename T>
requires std::is_standard_layout_v<T>
hal::BusError SpiDrv::read_burst(T * data_ptr, const size_t len, const Continuous cont) {
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        for (size_t i = 0; i < len; i++) {
            uint32_t temp = 0;
            spi_.read(temp);
            data_ptr[i] = temp;
        }
        if (cont == DISC) spi_.end();
        if (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::BusError::Ok();
}

template<typename T>
requires std::is_standard_layout_v<T>
hal::BusError SpiDrv::read_single(T & data, const Continuous cont) {
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t temp = 0;
        spi_.read(temp);
        data = temp;
        if (cont == DISC) spi_.end();
        if (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::BusError::Ok();
}

template<typename T>
requires std::is_standard_layout_v<T>
hal::BusError SpiDrv::transfer_single(T & datarx, T datatx, Continuous cont) {
    if (hal::BusError::Ok() == spi_.begin(idx_.to_req())) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t ret = 0;
        spi_.transfer(ret, datatx);
        datarx = ret;
        if (sizeof(T) != 1) this->set_data_width(8);
        if (cont == DISC) spi_.end();
    }
    return hal::BusError::Ok();
}



template<>
struct driver_of_bus<hal::Spi>{
    using driver_type = hal::SpiDrv;
};
    
}