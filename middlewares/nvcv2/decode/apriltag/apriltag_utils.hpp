#pragma once

#include <cstdint>
#include <span>
#include <algorithm>
#include "core/utils/Option.hpp"

namespace ymd::nvcv2::apriltag{
enum class [[nodiscard]] ApriltagType{
    _16h5 = 1,
    _25h9 = 2,
    _36h10 = 3,
    _36h11 = 4,
};

struct [[nodiscard]] ApriltagPattern{
    uint64_t code;
};

struct [[nodiscard]] ApriltagIndex{
public:
    static constexpr ApriltagIndex from_u16(const uint16_t raw){
        return {raw};
    }

    constexpr uint16_t to_u16() const{
        return index_;
    }
private:
    constexpr ApriltagIndex(uint16_t index):index_(index){}

    uint16_t index_ = 0;
};

struct [[nodiscard]] ApriltagDirection{
public:
    enum class [[nodiscard]] Kind:uint8_t{
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3
    };

    using enum Kind;

    constexpr ApriltagDirection(Kind kind):kind_(kind){}
    constexpr Kind kind() const {return kind_;}

    [[nodiscard]] constexpr bool operator==(const ApriltagDirection other) const {
        return kind_ == other.kind_;
    }

    [[nodiscard]] constexpr bool operator==(const Kind kind) const {
        return kind_ == kind;
    }

private:
    Kind kind_;
};

struct ApriltagResult{
    ApriltagIndex index;
    ApriltagDirection direction;
};


namespace utils{
namespace details{
[[nodiscard]] static constexpr uint16_t reverse_u16_16h5(uint16_t num) {
    uint16_t reversed = 0;

    reversed |= (num & (1 << 12)) ? (1 << 15) : 0;
    reversed |= (num & (1 << 8)) ? (1 << 14) : 0;
    reversed |= (num & (1 << 4)) ? (1 << 13) : 0;
    reversed |= (num & (1 << 0)) ? (1 << 12) : 0;

    reversed |= (num & (1 << 13)) ? (1 << 11) : 0;
    reversed |= (num & (1 << 9)) ? (1 << 10) : 0;
    reversed |= (num & (1 << 5)) ? (1 << 9) : 0;
    reversed |= (num & (1 << 1)) ? (1 << 8) : 0;

    reversed |= (num & (1 << 14)) ? (1 << 7) : 0;
    reversed |= (num & (1 << 10)) ? (1 << 6) : 0;
    reversed |= (num & (1 << 6)) ? (1 << 5) : 0;
    reversed |= (num & (1 << 2)) ? (1 << 4) : 0;

    reversed |= (num & (1 << 15)) ? (1 << 3) : 0;
    reversed |= (num & (1 << 11)) ? (1 << 2) : 0;
    reversed |= (num & (1 << 7)) ? (1 << 1) : 0;
    reversed |= (num & (1 << 3)) ? (1 << 0) : 0;
    return reversed;
}

[[nodiscard]] static constexpr uint16_t reverse_u4_16h5(uint16_t num) {
    uint16_t reversed = 0;
    for (size_t i = 0; i < 16; ++i) {
        reversed <<= 1; // Shift left by one bit
        reversed |= (num & 1); // Add the least significant bit of num to reversed
        num >>= 1; // Shift num right by one bit to process the next bit
    }
    return reversed;
}
}

[[nodiscard]] static constexpr uint16_t get_rcr_code_16h5(
    const uint16_t code, 
    const ApriltagDirection dir
){
    switch(dir.kind()){
        case ApriltagDirection::Right :
            return code;
        case ApriltagDirection::Left :
            return details::reverse_u4_16h5(code);
        case ApriltagDirection::Up :
            return details::reverse_u16_16h5(code);
        case ApriltagDirection::Down :
            return details::reverse_u16_16h5(details::reverse_u4_16h5(code));
    }
    __builtin_unreachable();
};


template<typename T, size_t N, typename Fn>
[[nodiscard]] static constexpr Option<ApriltagResult> find_code(
    const std::span<const T, N> code_table, 
    Fn && fn, 
    const ApriltagPattern pattern
){
    const auto dst_code = pattern.code;

    for(size_t i = 0; i < N; i++){
        const auto raw_code = uint64_t(code_table[i]);

        const auto up_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Kind::Up);
        const auto right_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Kind::Right);
        const auto down_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Kind::Down);
        const auto left_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Kind::Left);

        if(dst_code == up_code) return 
            Some(ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Kind::Up});
        if(dst_code == right_code) return 
            Some(ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Kind::Right});
        if(dst_code == down_code) return 
            Some(ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Kind::Down});
        if(dst_code == left_code) return 
            Some(ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Kind::Left});
    }

    return None;
}
}
}