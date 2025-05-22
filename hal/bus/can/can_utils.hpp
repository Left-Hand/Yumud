#pragma once

#include "core/platform.hpp"
#include "core/sdk.hpp"

#include <memory>
#include <functional>

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
        enum class Kind{
            _125K,
            _250K,
            _500K,
            _1M
        };

        using enum Kind;

        constexpr CanBaudrate(Kind kind):kind_(kind){}
        constexpr explicit CanBaudrate(const uint32_t freq):
            kind_(freq2kind(freq)){}

        constexpr operator Kind() const{return kind_;}
        constexpr Kind kind() const{
            return kind_;
        }

        constexpr auto dump() const{
            struct Ret{
                uint8_t prescale;
                CanSwj swj;
                CanBs1 bs1;
                CanBs2 bs2;
            };


            switch(kind_){
                default: __builtin_unreachable();
                case Kind::_125K:
                    return Ret{96, CanSwj::_2, CanBs1::_6, CanBs2::_5};
                case Kind::_250K:
                    return Ret{48, CanSwj::_2, CanBs1::_6, CanBs2::_5};
                case Kind::_500K:
                    return Ret{24, CanSwj::_2, CanBs1::_6, CanBs2::_5};
                case Kind::_1M:
                    return Ret{12, CanSwj::_2, CanBs1::_6, CanBs2::_5};
            };
        }
    private:
        Kind kind_;

        static constexpr Kind freq2kind(uint32_t freq){
            switch(freq){
                default: while(true);
                case 125000: return _125K;
                case 250000: return _250K;
                case 500000: return _500K;
                case 1000000: return _1M;
            }
        }
        static constexpr uint32_t kind2freq(const Kind kind){
            switch(kind){
                default: __builtin_unreachable();
                case Kind::_125K: return 125000;
                case Kind::_250K: return 250000;
                case Kind::_500K: return 500000;
                case Kind::_1M: return 1000000;
            }
        }
    };

    enum class CanMode:uint8_t{
        Normal = CAN_Mode_Normal,
        Silent = CAN_Mode_Silent,
        Internal = CAN_Mode_Silent_LoopBack,
        Loopback = CAN_Mode_LoopBack
    };

    enum class CanError:uint8_t{
        Overrun,
        Bit,
        Stuff,
        Crc,
        Form,
        Acknowledge,
        Other,
    };

    enum class CanRemoteSpec:uint8_t{
        Data = 0,
        Any = 0,
        Remote = 1,
        Specified = 1
    };  

};

