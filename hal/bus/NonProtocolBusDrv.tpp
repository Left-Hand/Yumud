namespace yumud{

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_writable_bus<BusType>
void NonProtocolBusDrv<BusType>::writeSingle(const T data, Continuous cont) {
    constexpr size_t size = sizeof(T);
    if (!bus.begin(index)) {
        if (size != 1) this->setDataBits(size * 8);

        if constexpr (size == 1) {
            bus.write((uint8_t)data);
        } else if constexpr (size == 2) {
            bus.write((uint16_t)data);
        } else {
            bus.write((uint32_t)data);
        }

        if (cont == DISC) bus.end();
        if (size != 1) this->setDataBits(8);
    }
}

template <typename BusType>
template <typename U>
requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
void NonProtocolBusDrv<BusType>::writeMulti(const U & data, const size_t len, Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(U) != 1) this->setDataBits(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) bus.write(static_cast<U>(data));
        if (cont == DISC) bus.end();
        if (sizeof(U) != 1) this->setDataBits(8);
    }
}

template <typename BusType>
template <typename U>
requires std::is_standard_layout_v<U> and is_writable_bus<BusType>
void NonProtocolBusDrv<BusType>::writeMulti(const U * data_ptr, const size_t len, Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(U) != 1) this->setDataBits(sizeof(U) * 8);
        for (size_t i = 0; i < len; i++) bus.write(static_cast<U>(data_ptr[i]));
        if (cont == DISC) bus.end();
        if (sizeof(U) != 1) this->setDataBits(8);
    }
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
void NonProtocolBusDrv<BusType>::readMulti(T * data_ptr, const size_t len, const Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
        for (size_t i = 0; i < len; i++) {
            uint32_t temp = 0;
            bus.read(temp, (i != len - 1));
            data_ptr[i] = temp;
        }
        if (cont == DISC) bus.end();
        if (sizeof(T) != 1) this->setDataBits(8);
    }
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_readable_bus<BusType>
void NonProtocolBusDrv<BusType>::readSingle(T & data, const Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
        uint32_t temp;
        bus.read(temp);
        data = temp;
        if (cont == DISC) bus.end();
        if (sizeof(T) != 1) this->setDataBits(8);
    }
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> and is_fulldup_bus<BusType>
void NonProtocolBusDrv<BusType>::transferSingle(T & datarx, T datatx, Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
        uint32_t ret = 0;
        bus.transfer(ret, datatx);
        datarx = ret;
        if (sizeof(T) != 1) this->setDataBits(8);
        if (cont == DISC) bus.end();
    }
}

template <typename BusType>
template<typename T>
requires std::is_standard_layout_v<T> && is_fulldup_bus<BusType>
T NonProtocolBusDrv<BusType>::transferSingle(T datatx, Continuous cont) {
    if (!bus.begin(index)) {
        if (sizeof(T) != 1) this->setDataBits(sizeof(T) * 8);
        T datarx;
        uint32_t ret = 0;
        bus.transfer(ret, datatx);
        datarx = ret;
        if (sizeof(T) != 1) this->setDataBits(8);
        if (cont == DISC) bus.end();
        return datarx;
    }
    return T(0);
}

}