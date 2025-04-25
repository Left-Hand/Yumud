//TODO fix duplicate code 

namespace ymd::hal{

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_writable_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::write_single(const T data, Continuous cont) {
    constexpr size_t size = sizeof(T);
    if (hal::BusError::OK == bus_.begin(index_)) {
        if (size != 1) this->set_data_width(size * 8);

        if constexpr (size == 1) {
            bus_.write(uint8_t(data));
        } else if constexpr (size == 2) {
            bus_.write(uint16_t(data));
        } else {
            bus_.write(uint32_t(data));
        }

        if (cont == DISC) bus_.end();
        if (size != 1) this->set_data_width(8);
    }

    return hal::BusError::OK;
}

template <typename BusType>
template <typename U>
requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::write_burst(const is_stdlayout auto & data, const size_t len, Continuous cont) {
    if (hal::BusError::OK == bus_.begin(index_) ) {
        if (sizeof(U) != 1) this->set_data_width(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) bus_.write(static_cast<U>(data));
        if (cont == DISC) bus_.end();
        if (sizeof(U) != 1) this->set_data_width(8);
    }
    return hal::BusError::OK;
}

template <typename BusType>
template <typename U>
requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::write_burst(const is_stdlayout auto * data_ptr, const size_t len, Continuous cont) {
    if (hal::BusError::OK == bus_.begin(index_) ) {
        if (sizeof(U) != 1) this->set_data_width(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) bus_.write(static_cast<U>(data_ptr[i]));
        if (cont == DISC) bus_.end();
        if (sizeof(U) != 1) this->set_data_width(8);
    }
    return hal::BusError::OK;
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::read_burst(T * data_ptr, const size_t len, const Continuous cont) {
    if (hal::BusError::OK == bus_.begin(index_) ) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        for (size_t i = 0; i < len; i++) {
            uint32_t temp = 0;
            bus_.read(temp);
            data_ptr[i] = temp;
        }
        if (cont == DISC) bus_.end();
        if (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::BusError::OK;
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::read_single(T & data, const Continuous cont) {
    if (hal::BusError::OK == bus_.begin(index_) ) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t temp = 0;
        bus_.read(temp);
        data = temp;
        if (cont == DISC) bus_.end();
        if (sizeof(T) != 1) this->set_data_width(8);
    }
    return hal::BusError::OK;
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_fulldup_bus<BusType>
hal::BusError NonProtocolBusDrv<BusType>::transfer_single(T & datarx, T datatx, Continuous cont) {
    if (hal::BusError::OK == bus_.begin(index_) ) {
        if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
        uint32_t ret = 0;
        bus_.transfer(ret, datatx);
        datarx = ret;
        if (sizeof(T) != 1) this->set_data_width(8);
        if (cont == DISC) bus_.end();
    }
    return hal::BusError::OK;
}

// template <typename BusType>
// template<typename T>
// requires std::is_standard_layout_v<T> && is_fulldup_bus<BusType>
// T NonProtocolBusDrv<BusType>::transfer_single(T datatx, Continuous cont) {
//     if (hal::BusError::OK == bus_.begin(index_) ) {
//         if (sizeof(T) != 1) this->set_data_width(sizeof(T) * 8);
//         T datarx;
//         uint32_t ret = 0;
//         bus_.transfer(ret, datatx);
//         datarx = ret;
//         if (sizeof(T) != 1) this->set_data_width(8);
//         if (cont == DISC) bus_.end();
//         return datarx;
//     }
//     return T(0);
// }

}