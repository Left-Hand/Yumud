#pragma once

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "can_dlc.hpp"

namespace ymd::hal{

//描述了CAN2.0B(传统CAN)的数据载荷
struct [[nodiscard]] ClassicCanPayload final{
public:    
    using Self = ClassicCanPayload;

    using U8X8 = std::array<uint8_t, 8>;


    //这里并没有用零拷贝，原因是对齐排列的uint64比零拷贝效率更高
    static constexpr U8X8 ZERO_U8X8 = std::bit_cast<U8X8>(uint64_t(0));

    //从给定的id和连续数据切片创建一个数据帧 当数据超长时立即终止程序
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
            ClassicCanDlc::from_bits(std::ranges::size(bytes))
        );
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
            ClassicCanDlc::from_bits(std::ranges::size(bytes))
        ));
    }

    __attribute__((always_inline)) static constexpr Self from_uninitialized(){
        return Self();
    }

    // __attribute__((always_inline)) static constexpr Self with_capacity(size_t len){
    //     Self self = Self();
    //     self.dlc = ClassicCanDlc::from_length(len);
    //     return self;
    // }

    __attribute__((always_inline)) static constexpr Self from_u8x8(std::array<uint8_t, 8> array){
        return Self(std::move(array), ClassicCanDlc::full());
    }

    __attribute__((always_inline)) static constexpr Self from_empty(){
        return Self(ZERO_U8X8, ClassicCanDlc::zero());
    }

    /// \brief 从给定的id和迭代器创建一个数据帧 当数据超长时立即终止程序
    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    __attribute__((always_inline)) static constexpr Self from_iter(Iter iter) {
        U8X8 buf = ZERO_U8X8;
        size_t len = 0;
        
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

        return Self(
            buf, 
            ClassicCanDlc::from_bits(len)
        );
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
            ClassicCanDlc::from_bits(len)
        ));
    }


    /// \brief 从给定的id和初始化列表创建一个数据帧 当数据超长时立即终止
    __attribute__((always_inline)) static constexpr Self from_list(
        const std::initializer_list<uint8_t> bytes
    ){
        if(bytes.size() > 8) [[unlikely]]
            __builtin_trap();
        U8X8 buf = ZERO_U8X8;
        std::copy(bytes.begin(), bytes.end(), buf.begin());
        return Self(
            buf, 
            ClassicCanDlc::from_bits(bytes.size())
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
            ClassicCanDlc::from_bits(bytes.size())
        ));
    }

    /// \brief 从64位整数和长度创建一个数据帧
    __attribute__((always_inline)) static constexpr Self from_u64_and_dlc(
        const uint64_t int_val,
        const ClassicCanDlc dlc
    ){
        return Self(std::bit_cast<U8X8>(int_val),dlc);
    }

    /// \brief 从64位整数创建一个长度为8的数据帧
    __attribute__((always_inline)) static constexpr Self from_u64(
        const uint64_t int_val
    ){
        return Self(std::bit_cast<U8X8>(int_val), ClassicCanDlc::full());
    }

    /// \brief 从数组和长度创建一个数据帧
    __attribute__((always_inline)) static constexpr Self from_parts(
        const U8X8 arr,
        const ClassicCanDlc dlc
    ){
        return Self(arr,dlc);
    }

    /// \brief 零
    __attribute__((always_inline)) static constexpr Self zero(
    ){
        return Self(ZERO_U8X8, ClassicCanDlc::zero());
    }


    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * data() {return u8x8.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t * begin() {return u8x8.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * data() const noexcept {return u8x8.data();}
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    const uint8_t * begin() const noexcept {return u8x8.begin();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t size() const noexcept {return length();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t length() const noexcept {return dlc.length();}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t operator[](uint8_t i) const noexcept {return u8x8[i];}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t & operator[](uint8_t i) noexcept {return u8x8[i];}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<const uint8_t> bytes() const noexcept {
        return std::span(u8x8.data(), dlc.length());
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> bytes() noexcept {
        return bytes_mut();
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::span<uint8_t> bytes_mut() noexcept {
        return std::span(u8x8.data(), dlc.length());
    }


    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t get() const noexcept {return get_element<I>(*this);}

    template<size_t I>
    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint8_t & get() {return get_element<I>(*this);}

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    std::array<uint32_t, 2> to_u32x2() const noexcept {
        // return u8x8;
        if(std::is_constant_evaluated()){
            return std::bit_cast<std::array<uint32_t, 2>>(u8x8);
        }else{
            return {
                *reinterpret_cast<const uint32_t *>(&u8x8[0]),
                *reinterpret_cast<const uint32_t *>(&u8x8[4])
            };
        }
    }

    [[nodiscard]] __attribute__((always_inline)) constexpr 
    uint64_t to_u64() const noexcept {
        const auto [low32, high32] = to_u32x2();
        return static_cast<uint64_t>(low32) | (static_cast<uint64_t>(high32) << 32);
    }

    #if 0
    alignas(4) U8X8 u8x8;
    alignas(4) ClassicCanDlc dlc;
    #else
    #pragma pack(push, 1)
    U8X8 u8x8;
    ClassicCanDlc dlc;
    #pragma pack(pop)
    #endif

    __attribute__((always_inline)) constexpr explicit
    ClassicCanPayload(const ClassicCanPayload & other):dlc(other.dlc){
        if(std::is_constant_evaluated()){
            u8x8 = other.u8x8;
        }else{
            const auto p_dst = reinterpret_cast<uint32_t *>(u8x8.data());
            const auto p_src = reinterpret_cast<const uint32_t *>(other.u8x8.data());
            p_dst[0] = p_src[0];
            p_dst[1] = p_src[1];
        }
    }

    constexpr ClassicCanPayload clone() const noexcept {
        return ClassicCanPayload(*this);
    }

private:
    __attribute__((always_inline)) constexpr explicit
    ClassicCanPayload(const U8X8 _u8x8, const ClassicCanDlc _dlc):
        u8x8(_u8x8), dlc(_dlc){
            if(std::is_constant_evaluated()){
                u8x8 = _u8x8;
            }else{
                const auto p_dst = reinterpret_cast<uint32_t *>(u8x8.data());
                const auto p_src = reinterpret_cast<const uint32_t *>(_u8x8.data());
                p_dst[0] = p_src[0];
                p_dst[1] = p_src[1];
            }
        }

    __attribute__((always_inline)) constexpr explicit
    ClassicCanPayload():dlc(ClassicCanDlc::from_uninitialized()){;}

    friend class ClassicCanFrame;

    template<size_t I>
    [[nodiscard]] static __attribute__((always_inline)) constexpr 
    auto get_element(auto & self) noexcept {
        return std::get<I>(self.u8x8);
    }
};

static_assert(sizeof(ClassicCanPayload) == 8 + 1);


}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::ClassicCanPayload & payload);
}