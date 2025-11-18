#pragma once

#include <cstdint>

namespace ymd::hal{
enum class [[nodiscard]] SpiI2sIT:uint8_t{
    TXE =       ((uint8_t)0x71),
    RXNE =      ((uint8_t)0x60),
    ERR =       ((uint8_t)0x50),
    OVR =       ((uint8_t)0x56),
    MODF =      ((uint8_t)0x55),
    CRCERR =    ((uint8_t)0x54),
    UDR =       ((uint8_t)0x53)
};

enum class [[nodiscard]] SpiEvent:uint8_t{
    TransmitBufferEmpty,
    ReceiveBufferNotEmpty
};

class [[nodiscard]] SpiSlaveRank{
public:
    explicit constexpr SpiSlaveRank(const uint8_t rank):
        rank_(rank){}

    [[nodiscard]] uint8_t count() const {return rank_;}
    [[nodiscard]] uint16_t as_unique_id() const {return static_cast<uint16_t>(rank_);}

private:
    uint8_t rank_;
};

enum class [[nodiscard]] SpiMode:uint8_t{
    _0,
    _1,
    _2,
    _3
};

enum class [[nodiscard]] SpiClockPolarity:uint8_t{
    IdleLow = 0,
    IdleHigh = 1
};

enum class [[nodiscard]] SpiClockPhase:uint8_t{
    CaptureOnFirst = 0,
    CaptureOnSecond = 1
};

struct [[nodiscard]] SpiPrescaler{
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

    #ifdef __YMD_HAL_CH32_FEATURE_HSSPI
    enum class [[nodiscard]] HsKind{
        // _2 = 0b000,
        _3 = 0b001,
        // _4 = 0b010,
        _5 = 0b011,
        _6 = 0b100,
        _7 = 0b101,
        // _8 = 0b110,
        _9 = 0b111
    };

    using enum HsKind;
    #endif
    constexpr SpiPrescaler(const Kind kind):kind_(kind){;}
    [[nodiscard]] constexpr Kind kind() const {return kind_;}
private:
    Kind kind_;
};
}