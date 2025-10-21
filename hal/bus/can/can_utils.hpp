#pragma once

#include "core/sdk.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/sumtype.hpp"
#include <memory>
#include <functional>

namespace ymd{
    class OutputStream;
}
namespace ymd::hal{

enum class CanFifoNth:uint8_t{
    _0 = 0,
    _1 = 1
};

enum class CanMailboxNth:uint8_t{
    _0 = 0,
    _1 = 1,
    _2 = 2
};

enum class CanIT:uint16_t{
    TME = (1u << 0),

    FMP0 = (1u << 1),
    FF0 = (1u << 2),
    FOV0 = (1u << 3),
    FMP1 = (1u << 4),
    FF1 = (1u << 5),
    FOV1 = (1u << 6),

    WKU = (1u << 7),
    SLK = (1u << 8),

    EWG = (1u << 9),
    EPV = (1u << 10),
    BOF = (1u << 11),
    LEC = (1u << 12),
    ERR = (1u << 13),
};

/// @brief CAN Swj时间长度
enum class CanSwj:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03
};

/// @brief CAN Bs1时间长度
enum class CanBs1:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03,
    _5tq = 0x04,
    _6tq = 0x05,
    _7tq = 0x06,
    _8tq = 0x07,
    _9tq = 0x08,
    _10tq = 0x09,
    _11tq = 0x0A,
    _12tq = 0x0B,
    _13tq = 0x0C,
    _14tq = 0x0D,
    _15tq = 0x0E,
    _16tq = 0x0F
};


/// @brief CAN Bs2时间长度
enum class CanBs2:uint8_t{
    _1tq = 0x00,
    _2tq = 0x01,
    _3tq = 0x02,
    _4tq = 0x03,
    _5tq = 0x04,
    _6tq = 0x05,
    _7tq = 0x06,
    _8tq = 0x07,
};


/// @brief CAN发送事件
enum class CanTransmitEvent:uint8_t{
    Failed,
    Success
};

/// @brief CAN接收事件
enum class CanReceiveEvent:uint8_t{
    Fifo0Pending,
    Fifo0Full,
    Fifo0Overrun,
    Fifo1Pending,
    Fifo1Full,
    Fifo1Overrun,
};

/// @brief CAN状态事件
enum class CanStatusEvent:uint8_t{
    Wakeup,
    SleepAcknowledge,
    ErrorWarninh,
    ErrorPassive,
    BusOff,
    LastErrorCode,
    Error
};

struct CanEvent:public Sumtype<CanTransmitEvent, CanReceiveEvent, CanStatusEvent>{};

struct CanBitTimmingCoeffs{
    uint16_t prescale;
    CanSwj swj;
    CanBs1 bs1;
    CanBs2 bs2;
};


class CanBaudrate{
public:
    enum class Kind:uint8_t{
        _10K,
        _20K,
        _50K,
        _100K,
        _125K,
        _250K,
        _500K,
        _800K,
        _1M
    };

    using enum Kind;
    constexpr CanBaudrate(Kind kind):kind_(kind){}
    static constexpr Option<CanBaudrate> from_freq(const uint32_t freq){
        const auto may_kind = freq2kind(freq);
        if(may_kind.is_none()) return None;
        return Some(CanBaudrate(may_kind.unwrap()));
    }

    constexpr Kind kind() const{return kind_;}
    constexpr CanBitTimmingCoeffs to_coeffs() const{
        //works only at 144mhz pclk1 freq
        switch(kind_){
            case Kind::_10K:    return {900,  CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_20K:    return {450,  CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_50K:    return {180,  CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_100K:   return {90,   CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_125K:   return {72,   CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_250K:   return {36,   CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};
            case Kind::_500K:   return {18,   CanSwj::_1tq,     CanBs1::_12tq,     CanBs2::_3tq};

            // datasheet(V2.7) @page 37
            // 4.3.6 内部时钟源特性 ~~~ HSI(RC) 1.6% 
            // use higher swj for stable
            case Kind::_800K:   return {9,    CanSwj::_2tq,     CanBs1::_15tq,     CanBs2::_4tq};
            case Kind::_1M:     return {9,    CanSwj::_2tq,     CanBs1::_12tq,     CanBs2::_3tq};

            default: __builtin_unreachable();
        };
    }
private:
    Kind kind_;

    static constexpr Option<Kind> freq2kind(uint32_t freq){
        switch(freq){
            default: return None;
            case 10'000: return Some(Kind::_10K);
            case 20'000: return Some(Kind::_20K);
            case 50'000: return Some(Kind::_50K);
            case 100'000: return Some(Kind::_100K);
            case 125'000: return Some(Kind::_125K);
            case 250'000: return Some(Kind::_250K);
            case 500'000: return Some(Kind::_500K);
            case 800'000: return Some(Kind::_800K);
            case 1000'000: return Some(Kind::_1M);
        }
    }
    static constexpr uint32_t kind2freq(const Kind kind){
        switch(kind){
            case Kind::_10K: return 10'000;
            case Kind::_20K: return 20'000;
            case Kind::_50K: return 50'000;
            case Kind::_100K: return 100'000;
            case Kind::_125K: return 125'000;
            case Kind::_250K: return 250'000;
            case Kind::_500K: return 500'000;
            case Kind::_800K: return 800'000;
            case Kind::_1M: return 1000'000;
        }
    }

    friend OutputStream & operator<<(OutputStream & os, const CanBaudrate & self){
        switch(self.kind()){
            case Kind::_10K: return os << "10K";
            case Kind::_20K: return os << "20K";
            case Kind::_50K: return os << "50K";
            case Kind::_100K:    return os << "100K";
            case Kind::_125K:    return os << "125K";
            case Kind::_250K:    return os << "250K";
            case Kind::_500K:    return os << "500K";
            case Kind::_800K:    return os << "800K";
            case Kind::_1M:  return os << "1M";
        }
        __builtin_unreachable();
    }
};

enum class CanMode:uint8_t{
    Normal = CAN_Mode_Normal,
    Silent = CAN_Mode_Silent,
    SilentLoopback = CAN_Mode_Silent_LoopBack,
    Loopback = CAN_Mode_LoopBack
};

enum class CanFault:uint8_t{
    Stuff = 0x10,
    Form = 0x20,
    Acknowledge = 0x30,
    BitRecessive = 0x40,
    BitDominant = 0x50,
    Crc = 0x60,
    SoftwareSet = 0x70,
};

OutputStream & operator<<(OutputStream & os, const CanFault & fault);

enum class CanError:uint8_t{
    BlockingTransmitTimeout,
    NoMailboxAvailable,
    SoftFifoOverflow
};

OutputStream & operator<<(OutputStream & os, const CanError & error);



enum class CanRtr:uint8_t{
    Data = 0,
    Remote = 1,
};  

};

