#pragma once

#include <cstdint>
#include <span>
#include <algorithm>
#include <optional>

namespace ymd::nvcv2{
enum class ApriltagType{
    _16h5 = 1,
    _25h9 = 2,
    _36h10 = 3,
    _36h11 = 4,
};

class ApriltagPattern{
public:
    static constexpr ApriltagPattern from_u64(const uint64_t raw){
        return {raw};
    }

    constexpr uint64_t as_u64() const{
        return code_;
    }
private:
    constexpr ApriltagPattern(const uint64_t raw):code_(raw){}

    uint64_t code_;
};

class ApriltagIndex{
public:
    static constexpr ApriltagIndex from_u16(const uint16_t raw){
        return {raw};
    }

    constexpr uint16_t as_u16() const{
        return index_;
    }
private:
    constexpr ApriltagIndex(uint16_t index):index_(index){}

    uint16_t index_ = 0;
};

class ApriltagDirection{
public:
    enum class Kind:uint8_t{
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3
    };

    using enum Kind;

    constexpr ApriltagDirection(Kind kind):kind_(kind){}
    constexpr Kind kind() const {return kind_;}

    constexpr bool operator==(const ApriltagDirection other) const {return kind_ == other.kind_;}
    constexpr bool operator==(const Kind kind) const {return kind_ == kind;}

    static constexpr ApriltagDirection from_u8(const uint8_t raw){
        return ApriltagDirection(static_cast<Kind>(raw));
    }

    constexpr uint8_t as_u8() const {return static_cast<uint8_t>(kind_);}
private:
    Kind kind_;
};

struct ApriltagResult{
    ApriltagIndex index;
    ApriltagDirection direction;
};


namespace Apriltag_utils{
namespace details{
static constexpr uint16_t reverse_u16_16h5(uint16_t num) {
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

static constexpr uint16_t reverse_u4_16h5(uint16_t num) {
    uint16_t reversed = 0;
    for (int i = 0; i < 16; ++i) {
        reversed <<= 1; // Shift left by one bit
        reversed |= (num & 1); // Add the least significant bit of num to reversed
        num >>= 1; // Shift num right by one bit to process the next bit
    }
    return reversed;
}
}

static constexpr uint16_t get_rcr_code_16h5(const uint16_t code, const ApriltagDirection dir){
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
    return 0;
};


template<typename T, size_t N, typename Fn>
static constexpr std::optional<ApriltagResult> find_code(const std::span<const T, N> code_table, Fn && fn, const ApriltagPattern pattern){
    const auto dst_code = pattern.as_u64();

    for(size_t i = 0; i < N; i++){
        const auto raw_code = uint64_t(code_table[i]);

        const auto up_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Up);
        const auto right_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Right);
        const auto down_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Down);
        const auto left_code = std::forward<Fn>(fn)(raw_code, ApriltagDirection::Left);

        if(dst_code == up_code) return 
            ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Up};
        if(dst_code == right_code) return 
            ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Right};
        if(dst_code == down_code) return 
            ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Down};
        if(dst_code == left_code) return 
            ApriltagResult{ApriltagIndex::from_u16(i), ApriltagDirection::Left};
    }

    return std::nullopt;
}
}
}