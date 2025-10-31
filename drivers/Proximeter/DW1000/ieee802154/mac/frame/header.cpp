#pragma once

#include "core/stream/ostream.hpp"

namespace ymd::ieee802154{

struct [[nodiscard]] PanId{
    static constexpr uint16_t BROADCAST = 0xffff;
    uint16_t count;

    static constexpr PanId from_broadcast(){
        return PanId{PanId::BROADCAST};
    }

    std::span<uint8_t, 2> as_mut_bytes(){
        return std::span<uint8_t, 2>(reinterpret_cast<uint8_t*>(this), 2);
    }

    std::span<const uint8_t, 2> as_bytes() const {
        return std::span<const uint8_t, 2>(reinterpret_cast<const uint8_t*>(this), 2);
    }

    constexpr bool operator ==(const PanId & other) const = default;
};

struct [[nodiscard]] ShortAddress{
    static constexpr uint16_t BROADCAST = 0xffff;
    uint16_t count;

    static constexpr ShortAddress from_broadcast(){
        return ShortAddress{ShortAddress::BROADCAST};
    }

    std::span<uint8_t, 2> as_mut_bytes(){
        return std::span<uint8_t, 2>(reinterpret_cast<uint8_t*>(this), 2);
    }

    std::span<const uint8_t, 2> as_bytes() const {
        return std::span<const uint8_t, 2>(reinterpret_cast<const uint8_t*>(this), 2);
    }

    constexpr bool operator ==(const ShortAddress & other) const = default;
};

struct [[nodiscard]] ExtendedAddress{
    static constexpr uint64_t BROADCAST = 0xffff;
    uint64_t count;

    static constexpr ExtendedAddress from_broadcast(){
        return ExtendedAddress{ExtendedAddress::BROADCAST};
    }

    std::span<uint8_t, 8> as_mut_bytes(){
        return std::span<uint8_t, 8>(reinterpret_cast<uint8_t*>(this), 8);
    }

    std::span<const uint8_t, 8> as_bytes() const {
        return std::span<const uint8_t, 8>(reinterpret_cast<const uint8_t*>(this), 8);
    }

    constexpr bool operator ==(const ExtendedAddress & other) const = default;
};

enum class AddressEncoding:uint8_t{
    Normal, Compressed
};



}