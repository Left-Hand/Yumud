#pragma once

#include <cstdint>

namespace ymd{

enum class CommStrategy:unsigned char{
    None = 0,
    Blocking,
    Sync = Blocking,
    Interrupt,
    Dma
};

enum class CommDirection:unsigned char{
    RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
};

class PeriphUserId{
public:
    using id_type = uint16_t;

    PeriphUserId(const id_type id):id_(id){}
    
    PeriphUserId clone() const{return *this;}

    void emplace(const PeriphUserId & other){id_ = other.id_;}
    void emplace(PeriphUserId && other){id_ = other.id_;}
private:
    PeriphUserId(const PeriphUserId &) = default;
    PeriphUserId(PeriphUserId &&) = default;
    id_type id_;
};

using BusUserId = PeriphUserId;

//用途说明符 为仲裁器提供当前外设的用法信息
template<typename TEnum>
class PeriphPurpose{
private:
    //用法说明符 为仲裁器提供当前外设的用法信息
    //例如 当前I2C外设可以被[PMBUS]作为[写用法]征用
    uint8_t usage_:4;//保存用途

    // 为仲裁器提供当前外设的转换信息
    // 例如 当前的Usart设备当作RS232使用
    uint8_t transform_:4;//保存转换
};


enum class I2cUsage:uint8_t{
    READ,
    WRITE,
};

enum class PmbusUsage:uint8_t{
    READ,
    WRITE,
};


}

