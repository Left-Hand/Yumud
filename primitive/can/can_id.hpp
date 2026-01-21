#pragma once

#include <cstdint>
#include <compare>
#include "core/utils/Option.hpp"
#include "core/utils/bits/bits_set.hpp"

namespace ymd{
class OutputStream;
}

namespace ymd::hal{

struct [[nodiscard]] CanStdId{
    using Self = CanStdId;
    static constexpr uint16_t MAX_VALUE = 0x7ff;
    static constexpr size_t NUM_BITS = 11;

    //从比特位构造id
    static constexpr CanStdId from_bits(const uint16_t bits){
        return CanStdId(bits);
    }

    //从比特位构造id 超限时立即终止
    static constexpr CanStdId from_u11(const uint16_t bits){
        if(bits > MAX_VALUE) [[unlikely]]
            __builtin_trap();
        return CanStdId(bits);
    }

    //尝试从比特位构造id
    static constexpr Option<CanStdId> try_from_u11(const uint16_t bits){
        if(bits > MAX_VALUE) return None;
        return Some(CanStdId(bits));
    }

    constexpr CanStdId(const CanStdId & other) = default;
    constexpr CanStdId(CanStdId && other) = default;

    [[nodiscard]] constexpr bool operator ==(const CanStdId& other) const = default;

    //是否比另一个canid要更优先
    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const {
        //less id means higher priority
        return bits_ < other.bits_;
    }

    [[nodiscard]] constexpr uint16_t to_u11() const {return bits_;}
    [[nodiscard]] constexpr literals::Bs11 to_b11() const {
        return literals::Bs11::from_bits_unchecked(bits_);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const {return bits_;}

    friend OutputStream & operator << (OutputStream & os, const Self & self);
private:
    uint16_t bits_;

    explicit constexpr CanStdId(const uint16_t bits):bits_(bits){;}
};

struct [[nodiscard]] CanExtId{
    using Self = CanExtId;
    static constexpr uint32_t MAX_VALUE = 0x1fffffff;
    static constexpr size_t NUM_BITS = 29;

    //从比特位构造id
    static constexpr CanExtId from_bits(const uint32_t bits){
        return CanExtId(bits);
    }

    //从比特位构造id 超限时立即终止
    static constexpr CanExtId from_u29(const uint32_t bits){
        if(bits > MAX_VALUE) [[unlikely]]
            __builtin_trap();
        return CanExtId(bits);
    }

    //尝试从比特位构造id
    static constexpr Option<CanExtId> try_from_u29(const uint32_t bits){
        if(bits > MAX_VALUE) return None;
        return Some(CanExtId(bits));
    }

    constexpr CanExtId(const CanExtId & other) = default;
    constexpr CanExtId(CanExtId && other) = default;

    [[nodiscard]] constexpr bool operator ==(const CanExtId& other) const = default;

    //是否比另一个canid要更优先
    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const {
        //less id means higher priority
        return bits_ < other.bits_;
    }

    [[nodiscard]] constexpr uint32_t to_u29() const {return bits_;}
    [[nodiscard]] constexpr uint32_t to_bits() const {return bits_;}

    friend OutputStream & operator << (OutputStream & os, const Self & self);
private:
    uint32_t bits_;
    explicit constexpr CanExtId(const uint32_t bits):bits_(bits){;}
};

namespace details{
template<typename T>
concept is_canid = 
    (std::is_same_v<std::decay_t<T>, CanStdId> 
    || std::is_same_v<std::decay_t<T>, CanExtId>)
;
}

}