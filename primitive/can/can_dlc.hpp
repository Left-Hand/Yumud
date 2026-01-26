#pragma once

#include <cstdint>
#include <compare>
#include "core/utils/Option.hpp"


//这个文件描述了BxCanDlc和FdCanBlc类 对于dlc的字段进行类型安全保障

namespace ymd{
class OutputStream;
}

// 不论是bxcan还是fdcan都至少需要四个比特才能表述
namespace ymd::hal{

//bxcan的dlc字段  至少需要四个比特才能表述
struct [[nodiscard]] BxCanDlc final{
    static constexpr size_t NUM_BITS = 4;
    using Self = BxCanDlc;

    static imconstexpr Self from_uninitialized(){
        return Self();
    }

    static consteval Self zero(){
        return Self(static_cast<uint8_t>(0));
    }

    static consteval Self full(){
        return Self(static_cast<uint8_t>(8));
    }

    __attribute__((always_inline))
    static constexpr Self from_bits(const uint8_t bits){
        return Self(bits);
    }

    __attribute__((always_inline))
    static constexpr Option<Self> try_from_length(const size_t length){
        if(length > 8) [[unlikely]]
            return None;
        return Some(Self::from_bits(static_cast<uint8_t>(length)));
    }

    __attribute__((always_inline))
    static constexpr Self from_length(const size_t length){
        if(length > 8) [[unlikely]]
            __builtin_trap();
        return Self::from_bits(static_cast<uint8_t>(length));
    }

    __attribute__((always_inline))
    [[nodiscard]] constexpr size_t length() const {
        return bits_;
    };

    __attribute__((always_inline))
    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits_;
    }

    __attribute__((always_inline))
    [[nodiscard]] constexpr auto operator <=>(const Self & other) const = default;
private:
    uint8_t bits_;

    constexpr explicit BxCanDlc(const uint8_t bits):
        bits_(bits){;}

    constexpr explicit BxCanDlc(){;}

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};

//这个类与平台无关 相关标准在CanFD的官方文档中定义
//fdcan的dlc字段 至少需要四个比特才能表述
struct [[nodiscard]] FdCanDlc final{
    static constexpr size_t NUM_BITS = 4;
    using Self = FdCanDlc;

    //canfd的dlc向下兼容传统can 不要求explicit
    constexpr FdCanDlc(const BxCanDlc & classic_dlc):    
        bits_(classic_dlc.to_bits()){;}

    /// @brief 从比特位开始构造
    /// @return CANFD帧的DLC
    static constexpr Self from_bits(const uint8_t bits){
        return Self(bits);
    }

    static constexpr Option<Self> try_from_bits(const uint8_t bits){
        if(bits > 15) [[unlikely]]
            return None;
        return Some(Self(bits));
    }


    static constexpr Self from_length(const size_t length){
        if(length <= 8) [[likely]]
            return Self::from_bits(static_cast<uint8_t>(length));
        switch(length){
            case 12:    return Self::from_bits(9);
            case 16:    return Self::from_bits(10);
            case 20:    return Self::from_bits(11);
            case 24:    return Self::from_bits(12);
            case 32:    return Self::from_bits(13);
            case 48:    return Self::from_bits(14);
            case 64:    return Self::from_bits(15);
        }
        __builtin_trap();
    }
    static constexpr Option<Self> try_from_length(const size_t length){
        if(length > 64) [[unlikely]]
            return None;
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

    /// @brief 从不少于指定长度构造
    /// @return CANFD帧的DLC
    static constexpr Option<Self> try_from_least_length(const size_t length){
        if(length > 64) [[unlikely]]
            return None;
        if(length <= 8) [[likely]]
            return Some(Self::from_bits(static_cast<uint8_t>(length)));
        switch(length){
            case 9 ... 12:      return Some(Self::from_bits(9));
            case 13 ... 16:     return Some(Self::from_bits(10));
            case 17 ... 20:     return Some(Self::from_bits(11));
            case 21 ... 24:     return Some(Self::from_bits(12));
            case 25 ... 32:     return Some(Self::from_bits(13));
            case 33 ... 48:     return Some(Self::from_bits(14));
            case 49 ... 64:     return Some(Self::from_bits(15));
        }
        return None;
    }

    /// @brief 从零长构造
    /// @return CANFD帧的DLC
    static constexpr Self zero(){
        return from_bits(static_cast<uint8_t>(0));
    }
    
    /// @brief 从满长度构造
    /// @return CANFD帧的DLC
    static constexpr Self full(){
        return from_bits(static_cast<uint8_t>(0x0f));
    }

    [[nodiscard]] constexpr size_t length() const {
        if(bits_ <= 8) [[likely]] return bits_;
        switch(bits_){
            case 9:     return 12u;
            case 10:    return 16u;
            case 11:    return 20u;
            case 12:    return 24u;
            case 13:    return 32u;
            case 14:    return 48u;
            case 15:    return 64u;
        }
        __builtin_unreachable();
    };

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return bits_;
    }
private:
    uint8_t bits_;

    constexpr explicit FdCanDlc(const uint8_t bits):
        bits_(bits){;}

    friend OutputStream & operator <<(OutputStream & os, const Self & self);
};
}