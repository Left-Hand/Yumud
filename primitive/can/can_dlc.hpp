#pragma once

#include <cstdint>
#include "core/utils/Option.hpp"

//这个类与平台无关 相关标准在CanFD的官方文档中定义
//这个文件描述了CanDlc类 对于dlc的字段进行类型安全保障

namespace ymd{
class OutputStream;
}

namespace ymd::hal{


struct [[nodiscard]] CanClassicDlc{
    static constexpr size_t NUM_BITS = 4;
    using Self = CanClassicDlc;

    static constexpr Self from_zero(){
        return Self(static_cast<uint8_t>(0));
    }

    static constexpr Self from_bits(const uint8_t bits){
        return Self(bits);
    }

    static constexpr Option<Self> from_length(const size_t length){
        if(length <= 8) [[likely]]
            return Some(Self::from_bits(static_cast<uint8_t>(length)));
        return None;
    }

    [[nodiscard]] constexpr size_t length() const {
        return bits_;
    };

    [[nodiscard]] constexpr uint8_t as_bits() const {
        return bits_;
    }
private:
    uint8_t bits_;

    constexpr explicit CanClassicDlc(const uint8_t bits):
        bits_(bits){;}

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};

struct [[nodiscard]] CanFdDlc{
    static constexpr size_t NUM_BITS = 4;
    using Self = CanFdDlc;

    static constexpr Self from_bits(const uint8_t bits){
        return Self(bits);
    }

    static constexpr Option<Self> from_length(const size_t length){
        if(length <= 8) [[likely]]
            return Some(Self::from_bits(static_cast<uint8_t>(length)));
        switch(length){
            case 12:    return Some(Self::from_bits(9));
            case 16:    return Some(Self::from_bits(10));
            case 20:    return Some(Self::from_bits(11));
            case 24:    return Some(Self::from_bits(12));
            case 32:    return Some(Self::from_bits(13));
            case 48:    return Some(Self::from_bits(14));
            case 64:    return Some(Self::from_bits(15));
        }
        return None;
    }

    [[nodiscard]] constexpr size_t length() const {
        if(bits_ <= 8) [[likely]] return bits_;
        switch(bits_){
            case 9:     return 12;
            case 10:    return 16;
            case 11:    return 20;
            case 12:    return 24;
            case 13:    return 32;
            case 14:    return 48;
            case 15:    return 64;
        }
        __builtin_unreachable();
    };

    [[nodiscard]] constexpr uint8_t as_bits() const {
        return bits_;
    }
private:
    uint8_t bits_;

    constexpr explicit CanFdDlc(const uint8_t bits):
        bits_(bits){;}

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};
}