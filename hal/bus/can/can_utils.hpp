#pragma once

#include "core/sdk.hpp"
#include "core/utils/Option.hpp"
#include <memory>
#include <functional>

namespace ymd{
    class OutputStream;
}
namespace ymd::hal{

enum class CanSwj:uint8_t{
    _1 = 0x00,
    _2 = 0x01,
    _3 = 0x02,
    _4 = 0x03
};

enum class CanBs1:uint8_t{
    _1 = 0x00,
    _2 = 0x01,
    _3 = 0x02,
    _4 = 0x03,
    _5 = 0x04,
    _6 = 0x05,
    _7 = 0x06,
    _8 = 0x07,
    _9 = 0x08,
    _10 = 0x09,
    _11 = 0x0A,
    _12 = 0x0B,
    _13 = 0x0C,
    _14 = 0x0D,
    _15 = 0x0E,
    _16 = 0x0F
};

enum class CanBs2:uint8_t{
    _1 = 0x00,
    _2 = 0x01,
    _3 = 0x02,
    _4 = 0x03,
    _5 = 0x04,
    _6 = 0x05,
    _7 = 0x06,
    _8 = 0x07,
};

class CanBaudrate{
public:
    enum class Kind:uint8_t{
        _125K,
        _250K,
        _500K,
        _1M
    };

    using enum Kind;

    struct Setting{
        uint8_t prescale;
        CanSwj swj;
        CanBs1 bs1;
        CanBs2 bs2;
    };

    constexpr CanBaudrate(Kind kind):kind_(kind){}
    static constexpr Option<CanBaudrate> from_freq(const uint32_t freq){
        const auto may_kind = freq2kind(freq);
        if(may_kind.is_none()) return None;
        return Some(CanBaudrate(may_kind.unwrap()));
    }

    constexpr Kind kind() const{return kind_;}
    constexpr Setting dump() const{
        switch(kind_){
            case Kind::_125K:
                return Setting{
                    .prescale = 96, 
                    .swj = CanSwj::_2, 
                    .bs1 = CanBs1::_6, 
                    .bs2 = CanBs2::_5
                };
            case Kind::_250K:
                return Setting{
                    .prescale = 48, 
                    .swj = CanSwj::_2, 
                    .bs1 = CanBs1::_6, 
                    .bs2 = CanBs2::_5
                };
            case Kind::_500K:
                return Setting{
                    .prescale = 24, 
                    .swj = CanSwj::_2, 
                    .bs1 = CanBs1::_6, 
                    .bs2 = CanBs2::_5
                };
            case Kind::_1M:
                return Setting{
                    .prescale = 12, 
                    .swj = CanSwj::_2, 
                    .bs1 = CanBs1::_6, 
                    .bs2 = CanBs2::_5
                };
            default: __builtin_unreachable();
        };
    }
private:
    Kind kind_;

    static constexpr Option<Kind> freq2kind(uint32_t freq){
        switch(freq){
            default: return None;
            case 125'000: return Some(Kind::_125K);
            case 250'000: return Some(Kind::_250K);
            case 500'000: return Some(Kind::_500K);
            case 1000'000: return Some(Kind::_1M);
        }
    }
    static constexpr uint32_t kind2freq(const Kind kind){
        switch(kind){
            case Kind::_125K: return 125'000;
            case Kind::_250K: return 250'000;
            case Kind::_500K: return 500'000;
            case Kind::_1M: return 1000'000;
        }
    }
};

enum class CanMode:uint8_t{
    Normal = CAN_Mode_Normal,
    SilentListen = CAN_Mode_Silent,
    Internal = CAN_Mode_Silent_LoopBack,
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
    NoMailboxFounded,
    SoftFifoOverflow
};

OutputStream & operator<<(OutputStream & os, const CanError & error);

enum class CanMailBox:uint8_t{
    _0 = 0,
    _1 = 1,
    _2 = 2
};

enum class CanRtr:uint8_t{
    Data = 0,
    // Any = 0,
    Remote = 1,
    // Specified = 1
};  

};

