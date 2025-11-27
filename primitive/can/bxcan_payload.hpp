#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "can_dlc.hpp"

namespace ymd::hal{

struct alignas(4) [[nodiscard]] BxCanPayload{
public:    
    using Self = BxCanPayload;

    using U8X8 = std::array<uint8_t, 8>;
    //这里并没有用零拷贝，原因是对齐排列的uint64比零拷贝效率更高
    static constexpr U8X8 ZERO_U8X8 = std::bit_cast<U8X8>(uint64_t(0));

    /// \brief 从给定的id和连续数据切片创建一个数据帧 当数据超长时立即终止程序
    template<std::ranges::input_range R>
    requires (std::same_as<std::ranges::range_value_t<R>, uint8_t>)
    __attribute__((always_inline)) static constexpr Self from_bytes(
        R&& bytes
    ){
        // 检查是否在编译期可以获取大小
        if constexpr (std::ranges::sized_range<R> && 
                    requires { std::ranges::size(bytes); } &&
                    requires { std::bool_constant<(std::ranges::size(bytes) <= 8)>{}; }) {
            // 编译期大小检查
            constexpr size_t size = std::ranges::size(bytes);
            static_assert(size <= 8, "Range size must be <= 8 at compile time");
        } else {
            // 运行时大小检查
            if (std::ranges::size(bytes) > 8) [[unlikely]]
                __builtin_trap();
        }
        U8X8 buf = ZERO_U8X8;
        std::ranges::copy(bytes, buf.begin());
        return Self(
            buf, 
            CanClassicDlc::from_bits(std::ranges::size(bytes))
        );
    }

    __attribute__((always_inline)) static constexpr Self from_u8x8(std::array<uint8_t, 8> array){
        return Self(std::move(array), CanClassicDlc::full());
    }


    /// \brief 从给定的id和连续数据切片创建一个数据帧 当数据超长时返回空
    template<std::ranges::input_range R>
    requires (std::same_as<std::ranges::range_value_t<R>, uint8_t>)
    __attribute__((always_inline)) static constexpr Option<Self> try_from_bytes(
        R&& bytes
    ){
        // 检查是否在编译期可以获取大小
        if constexpr (std::ranges::sized_range<R> && 
                    requires { std::ranges::size(bytes); } &&
                    requires { std::bool_constant<(std::ranges::size(bytes) <= 8)>{}; }) {
            // 编译期大小检查
            constexpr size_t size = std::ranges::size(bytes);
            static_assert(size <= 8, "Range size must be <= 8 at compile time");
        } else {
            // 运行时大小检查
            if (std::ranges::size(bytes) > 8) [[unlikely]]
                return None;
        }
        U8X8 buf = ZERO_U8X8;
        std::ranges::copy(bytes, buf.begin());
        return Some(Self(
            buf, 
            CanClassicDlc::from_bits(std::ranges::size(bytes))
        ));
    }


    /// \brief 从给定的id和迭代器创建一个数据帧 当数据超长时立即终止程序
    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    __attribute__((always_inline)) static constexpr Self from_iter(Iter iter) {
        U8X8 buf = ZERO_U8X8;
        size_t len = 0;
        
        #pragma GCC unroll 8
        for(;len < 8; len++) {
            if(not iter.has_next()) break;
            buf[len] = iter.next();
        }
        if(iter.has_next()) [[unlikely]]
            __builtin_trap();
        #pragma GCC unroll 8
        for(;len < 8; len++){
            buf[len] = 0;
        }

        return Some(Self(
            buf, 
            CanClassicDlc::from_bits(len)
        ));
    }

    /// \brief 尝试从给定的id和迭代器创建一个数据帧 当数据超长时返回空
    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    static constexpr Option<Self> try_from_iter(Iter iter) {
        U8X8 buf = ZERO_U8X8;
        size_t len = 0;
        #pragma GCC unroll 8
        for(;len < 8; len++) {
            if(not iter.has_next()) break;
            buf[len] = iter.next();
        }
        if(iter.has_next()) [[unlikely]]
            return None;
        #pragma GCC unroll 8
        for(;len < 8; len++){
            buf[len] = 0;
        }
        // 使用数组视图构造CanFrame
        return Some(Self(
            buf, 
            CanClassicDlc::from_bits(len)
        ));
    }


    /// \brief 尝试从给定的id和初始化列表创建一个数据帧 当数据超长时立即终止
    __attribute__((always_inline)) static constexpr Self from_list(
        const std::initializer_list<uint8_t> bytes
    ){
        if(bytes.size() > 8) [[unlikely]]
            __builtin_trap();
        U8X8 buf = ZERO_U8X8;
        std::copy(bytes.begin(), bytes.end(), buf.begin());
        return Self(
            buf, 
            CanClassicDlc::from_bits(bytes.size())
        );
    }

    /// \brief 尝试从给定的id和初始化列表创建一个数据帧 当数据超长时返回空
    __attribute__((always_inline)) static constexpr Option<Self> try_from_list(
        const std::initializer_list<uint8_t> bytes
    ){
        if(bytes.size() > 8) [[unlikely]]
            return None;
        U8X8 buf = ZERO_U8X8;
        std::copy(bytes.begin(), bytes.end(), buf.begin());
        return Some(Self(
            buf, 
            CanClassicDlc::from_bits(bytes.size())
        ));
    }

    /// \brief 
    __attribute__((always_inline)) static constexpr Self from_u64_and_dlc(
        const uint64_t int_val,
        const CanClassicDlc dlc
    ){
        return Self(std::bit_cast<U8X8>(int_val),dlc);
    }

    /// \brief 
    __attribute__((always_inline)) static constexpr Self from_u64(
        const uint64_t int_val
    ){
        return Self(std::bit_cast<U8X8>(int_val), CanClassicDlc::full());
    }

    /// \brief 
    __attribute__((always_inline)) static constexpr Self from_parts(
        const U8X8 arr,
        const CanClassicDlc dlc
    ){
        return Self(arr,dlc);
    }

    /// \brief 
    __attribute__((always_inline)) static constexpr Self zero(
    ){
        return Self(ZERO_U8X8, CanClassicDlc::zero());
    }


    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * data() {return bytes_.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * begin() {return bytes_.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * data() const noexcept {return bytes_.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * begin() const noexcept {return bytes_.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t size() const noexcept {return dlc_.to_bits();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t operator[](uint8_t i) const noexcept {return bytes_[i];}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t & operator[](uint8_t i) noexcept {return bytes_[i];}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<const uint8_t> bytes() const noexcept {
        return std::span(bytes_.data(), dlc_.length());
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> bytes() noexcept {
        return bytes_mut();
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> bytes_mut() noexcept {
        return std::span(bytes_.data(), dlc_.length());
    }


    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t get() const noexcept {return get_element<I>(*this);}

    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t & get() {return get_element<I>(*this);}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const U8X8 & u8x8() const {return bytes_;}
private:
    alignas(4) U8X8 bytes_;
    CanClassicDlc dlc_;

    [[nodiscard]] __attribute__((always_inline)) constexpr explicit
    BxCanPayload(const U8X8 bytes, const CanClassicDlc dlc):
        bytes_(bytes), dlc_(dlc){;}

    friend class CanClassicFrame;

    template<size_t I>
    [[nodiscard]] static __attribute__((always_inline)) constexpr 
    auto get_element(auto & self) noexcept {
        return std::get<I>(self.bytes_);
    }
};

static_assert(sizeof(BxCanPayload) == 12);
using CanClassicPayload = BxCanPayload;

}