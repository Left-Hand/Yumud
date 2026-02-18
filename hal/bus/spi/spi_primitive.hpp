#pragma once

#include "primitive/nearest_freq.hpp"
#include "core/utils/sumtype.hpp"
#include "spi_layout.hpp"

namespace ymd::hal::spi{
enum class [[nodiscard]] IT:uint8_t{
    TXE =       ((uint8_t)0x71),
    RXNE =      ((uint8_t)0x60),
    ERR =       ((uint8_t)0x50),
    OVR =       ((uint8_t)0x56),
    MODF =      ((uint8_t)0x55),
    CRCERR =    ((uint8_t)0x54),
    UDR =       ((uint8_t)0x53)
};

enum class [[nodiscard]] Event:uint8_t{
    TransmitBufferEmpty,
    ReceiveBufferNotEmpty
};

struct [[nodiscard]] SlaveRank{
public:
    explicit constexpr SlaveRank(const uint8_t count):
        count_(count){}

    [[nodiscard]] uint8_t count() const {return count_;}
    [[nodiscard]] uint16_t as_unique_id() const {return static_cast<uint16_t>(count_);}

private:
    uint8_t count_;
};

//CPOL
enum class [[nodiscard]] ClockPolarity:uint8_t{
    IdleLow = 0,
    IdleHigh = 1
};

//CPHA
enum class [[nodiscard]] ClockPhase:uint8_t{
    CaptureOnFirst = 0,
    CaptureOnSecond = 1
};

struct [[nodiscard]] Prescaler{
    enum class [[nodiscard]] Kind:uint8_t{
        _2 = 0b000,
        _4 = 0b001,
        _8 = 0b010,
        _16 = 0b011,
        _32 = 0b100,
        _64 = 0b101,
        _128 = 0b110,
        _256 = 0b111
    };

    using enum Kind;

    #ifdef __YMD_FEATURE_CH32_HSSPI
    enum class [[nodiscard]] HsKind{
        _3 = 0b001,
        _5 = 0b011,
        _6 = 0b100,
        _7 = 0b101,
        _9 = 0b111
    };

    using enum HsKind;
    #endif
    constexpr Prescaler(const Kind kind):kind_(kind){;}
    [[nodiscard]] constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};


struct [[nodiscard]] Mode{

    enum class [[nodiscard]] Kind:uint8_t{
        _0,
        _1,
        _2,
        _3
    };

    using enum Kind;

    constexpr Mode(const Kind kind):kind_(kind){;}
    constexpr Mode(const ClockPolarity cpol, const ClockPhase cpoa):
        kind_([&]{
            return std::bit_cast<Kind>(uint8_t(
                (cpol == ClockPolarity::IdleHigh ? 0b10 : 0b00) | 
                (cpoa == ClockPhase::CaptureOnSecond ? 0b01 : 0b00))
            );
        }()){;}
    [[nodiscard]] constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};

using Baudrate = Sumtype<Prescaler, NearestFreq, LeastFreq>;

enum class [[nodiscard]] WordSize:uint8_t{
    OneByte = 0b00,
    TwoBytes = 0b01
};

struct [[nodiscard]] Config{
    SpiRemap remap;
    Baudrate baudrate;
    Mode mode = Mode::_3;
};

}

namespace ymd::hal{
using SpiI2sIT = spi::IT;
using SpiEvent = spi::Event;
using SpiSlaveRank = spi::SlaveRank;
using SpiMode = spi::Mode;
using SpiClockPolarity = spi::ClockPolarity;
using SpiClockPhase = spi::ClockPhase;
using SpiPrescaler = spi::Prescaler;
using SpiBaudrate = spi::Baudrate;
using SpiConfig = spi::Config;
using SpiWordSize = spi::WordSize;
}