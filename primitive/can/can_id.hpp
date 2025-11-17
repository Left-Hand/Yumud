#pragma once

#include <cstdint>
#include <compare>
#include "core/utils/Option.hpp"

namespace ymd{
class OutputStream;
}

namespace ymd::hal{

struct [[nodiscard]] CanStdId{
    using Self = CanStdId;
    static constexpr uint16_t MAX_VALUE = 0x7ff;
    static constexpr size_t NUM_BITS = 11;

    //make CanStdId without checking 11bits
    explicit constexpr CanStdId(const uint16_t bits):bits_(bits){;}
    static constexpr Option<CanStdId> from_u11(const uint16_t bits){
        if(bits > 0x7ff) return None;
        return Some(CanStdId(bits));
    }
    constexpr CanStdId(const CanStdId & other) = default;
    constexpr CanStdId(CanStdId && other) = default;

    [[nodiscard]] constexpr bool operator ==(const CanStdId& other) const = default;

    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const {
        //less id means higher priority
        return bits_ < other.bits_;
    }

    [[nodiscard]] constexpr uint16_t to_u11() const {return bits_;}

    friend OutputStream & operator << (OutputStream & os, const Self & self);
private:
    uint16_t bits_;
};

struct [[nodiscard]] CanExtId{
    using Self = CanExtId;
    static constexpr uint32_t MAX_VALUE = 0x1fffffff;
    static constexpr size_t NUM_BITS = 29;

    //make CanStdId without checking 29bits
    explicit constexpr CanExtId(const uint32_t bits):bits_(bits){;}

    static constexpr Option<CanExtId> from_u29(const uint32_t bits){
        if(bits > 0x1fffffff) return None;
        return Some(CanExtId(bits));
    }

    constexpr CanExtId(const CanExtId & other) = default;
    constexpr CanExtId(CanExtId && other) = default;

    [[nodiscard]] constexpr bool operator ==(const CanExtId& other) const = default;

    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const {
        //less id means higher priority
        return bits_ < other.bits_;
    }
    [[nodiscard]] constexpr uint32_t to_u29() const {return bits_;}

    friend OutputStream & operator << (OutputStream & os, const Self & self);
private:
    uint32_t bits_;
};

namespace details{
template<typename T>
concept is_canid = 
    (std::is_same_v<std::decay_t<T>, CanStdId> 
    || std::is_same_v<std::decay_t<T>, CanExtId>)
;
}

}