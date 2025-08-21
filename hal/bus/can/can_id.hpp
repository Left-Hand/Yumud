#pragma once

#include <cstdint>
#include <compare>
#include "core/utils/Option.hpp"

namespace ymd::hal{

struct CanStdId{
    static constexpr uint16_t MAX_VALUE = 0x7ff;
    static constexpr size_t LENGTH = 11;

    explicit constexpr CanStdId(const uint16_t raw):raw_(raw){;}

    constexpr CanStdId(const CanStdId & other) = default;
    constexpr CanStdId(CanStdId && other) = default;

    static constexpr Option<CanStdId> from_u11(const uint16_t raw){
        if(raw > 0x7ff) return None;
        return Some(CanStdId(raw));
    }

    constexpr auto operator<=>(const CanStdId& other) const = default;

    constexpr uint16_t to_u11() const {return raw_;}
private:
    uint16_t raw_;
};

struct CanExtId{
    static constexpr uint32_t MAX_VALUE = 0x1fffffff;
    static constexpr size_t LENGTH = 29;

    explicit constexpr CanExtId(const uint32_t raw):raw_(raw){;}

    constexpr CanExtId(const CanExtId & other) = default;
    constexpr CanExtId(CanExtId && other) = default;
    static constexpr Option<CanExtId> from_u29(const uint32_t raw){
        if(raw > 0x1fffffff) return None;
        return Some(CanExtId(raw));
    }

    constexpr auto operator<=>(const CanExtId& other) const = default;

    constexpr uint32_t to_u29() const {return raw_;}
private:
    uint32_t raw_;
};


}