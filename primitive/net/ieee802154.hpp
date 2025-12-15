#pragma once


#include "core/stream/ostream.hpp"

#include <cstdint>

// https://docs.rs/smoltcp/0.12.0/src/smoltcp/wire/ieee802154.rs.html
namespace ymd::net::ieee802154{



enum class [[nodiscard]] FrameType:uint8_t{
    Beacon = 0b000,
    Data = 0b001,
    Acknowledgement = 0b010,
    MacCommand = 0b011,
    Multipurpose = 0b101,
    FragmentOrFrak = 0b110,
    Extended = 0b111,
};

enum class [[nodiscard]] AddressingMode:uint8_t{ 
    Absent    = 0b00,
    Short     = 0b10,
    Extended  = 0b11,
};

inline OutputStream& operator<<(OutputStream& os, const FrameType& self) {
    switch(self) {
        case FrameType::Beacon: return os << "Beacon";
        case FrameType::Data: return os << "Data";
        case FrameType::Acknowledgement: return os << "Acknowledgement";
        case FrameType::MacCommand: return os << "MacCommand";
        case FrameType::Multipurpose: return os << "Multipurpose";
        case FrameType::FragmentOrFrak: return os << "FragmentOrFrak";
        case FrameType::Extended: return os << "Extended";
    }
    return os << "Unknown" << static_cast<uint8_t>(self);
}

inline OutputStream& operator<<(OutputStream& os, const AddressingMode& self) {
    switch(self) {
        case AddressingMode::Absent: return os << "Absent";
        case AddressingMode::Short: return os << "Short";
        case AddressingMode::Extended: return os << "Extended";
    }
    return os << "Unknown" << static_cast<uint8_t>(self);
}

struct [[nodiscard]] PanId{
    uint16_t bits;
};



enum class [[nodiscard]] FrameVersion:uint8_t{
    Ieee802154_2003 = 0b00,
    Ieee802154_2006 = 0b01,
    Ieee802154 = 0b10,
};
}