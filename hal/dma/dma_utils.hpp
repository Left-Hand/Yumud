#pragma once

#include <cstdint>

namespace ymd{
class OutputStream;
}


namespace ymd::hal::dma{

struct [[nodiscard]] Mode{
public:
    enum class Kind:uint8_t{
        ToMemory = 0b000,
        ToPeriph = 0b001,
        Synergy = 0b010,
        Distribute = 0b011,

        ToMemCircular = 0b100,
        ToPeriphCircular = 0b101,
        SynergyCircular = 0b110,
        DistributeCircular = 0b111,
    };

    using enum Kind;

    constexpr Mode(Kind kind):kind_(kind){}

    [[nodiscard]] constexpr Kind kind()const{return kind_;}

    [[nodiscard]] constexpr bool is_circular()const{
        return (std::bit_cast<uint8_t>(kind()) & 0b100) != 0;
    }

    [[nodiscard]] constexpr bool dst_is_periph() const {
        switch(kind()){
            case Kind::ToPeriph:
            case Kind::ToPeriphCircular:
                return true;
            default:
                return false;
        }
    }

    friend OutputStream& operator<<(OutputStream& os, Mode::Kind kind);
    friend OutputStream& operator<<(OutputStream& os, Mode mode){
        return os << mode.kind();}
private:
    Kind kind_;
};

struct [[nodiscard]] Direction{
public:
    enum class Kind:uint8_t{
        ToMemory = 0,
        ToPeriph,
        Synergy,
        Distribute
    };

    using enum Kind;
    constexpr Direction(Kind kind):kind_(kind){}

    constexpr Direction(Mode mode):kind_(
        Kind{static_cast<uint8_t>(std::bit_cast<uint8_t>(mode.kind()) & 0b011)}){}

    [[nodiscard]] constexpr Kind kind()const{return kind_;}

    [[nodiscard]] constexpr Mode with_circular()const{
        return std::bit_cast<Mode>(static_cast<uint8_t>(std::bit_cast<uint8_t>(kind()) | 0b100));
    }

    [[nodiscard]] constexpr Mode without_circular()const{
        return std::bit_cast<Mode>(static_cast<uint8_t>(std::bit_cast<uint8_t>(kind()) & 0b011));
    }

    friend OutputStream& operator<<(OutputStream& os, Direction::Kind kind);
    friend OutputStream& operator<<(OutputStream& os, Direction direction){
        return os << direction.kind();}
private:
    Kind kind_;
};


enum class [[nodiscard]] Priority:uint16_t{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};


enum class [[nodiscard]] IT:uint8_t{
    Done,
    Half
};

enum class [[nodiscard]] Event:uint8_t{
    TransferComplete,
    HalfTransfer,
    TransferError
};
}
namespace ymd::hal{
using DmaMode = dma::Mode;
using DmaDirection = dma::Direction;
using DmaEvent = dma::Event;
using DmaIT = dma::IT;
using DmaPriority = dma::Priority;
}