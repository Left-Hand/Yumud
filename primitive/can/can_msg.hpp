#pragma once

#include "can_enum.hpp"
#include "can_id.hpp"
#include "can_dlc.hpp"
#include "can_identifier.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"


//这个文件描述了CanClassicMsg类 表示标准Can2.0(bxcan)的消息

namespace ymd::hal{

struct alignas(16) [[nodiscard]] CanClassicMsg{
public:
    using U8X8 = std::array<uint8_t, 8>;
    //这里并没有用零拷贝，原因是对齐排列的uint64比零拷贝效率更高
    static constexpr U8X8 ZERO_U8X8 = std::bit_cast<U8X8>(uint64_t(0));

    using Self = CanClassicMsg;
    constexpr CanClassicMsg(const CanClassicMsg & other) = default;
    constexpr CanClassicMsg & operator = (const CanClassicMsg & other) = default;
    constexpr CanClassicMsg(CanClassicMsg && other) = default;
    constexpr CanClassicMsg & operator = (CanClassicMsg && other) = default;
    // constexpr Self() = default;

    /// \brief 从给定的id创建一个远程帧
    __always_inline static constexpr Self from_remote(
        details::is_canid auto id
    ){
        return Self(
            CanIdentifier::from_parts(id, CanRtr::Remote), 
            ZERO_U8X8, 
            CanClassicDlc::from_bits(0)
        );
    }

    /// \brief 从给定的id创建一个空的数据帧

    __always_inline static constexpr Self from_empty(
        details::is_canid auto id
    ){
        return Self(
            CanIdentifier::from_parts(id, CanRtr::Data), 
            ZERO_U8X8, 
            CanClassicDlc::from_bits(0)
        );
    }



    /// \brief 从给定的id和长度为8的u64创建一个填满的消息，其中u64为小端序
    __always_inline static constexpr Self from_id_and_payload_u64(
        details::is_canid auto id, 
        const uint64_t payload_bits
    ){
        return Self(
            CanIdentifier::from_parts(id, CanRtr::Data), 
            std::bit_cast<U8X8>(payload_bits), 
            CanClassicDlc::from_bits(8)
        );
    }



    /// \brief 从给定的id和连续数据切片创建一个数据帧 当数据超长时立即终止程序
    template<std::ranges::input_range R>
    requires (std::same_as<std::ranges::range_value_t<R>, uint8_t>)
    __always_inline static constexpr Self from_bytes(
        details::is_canid auto id, 
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
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(std::ranges::size(bytes))
        );
    }


    /// \brief 从给定的id和连续数据切片创建一个数据帧 当数据超长时返回空
    template<std::ranges::input_range R>
    requires (std::same_as<std::ranges::range_value_t<R>, uint8_t>)
    __always_inline static constexpr Option<Self> try_from_bytes(
        details::is_canid auto id, 
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
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(std::ranges::size(bytes))
        ));
    }


    /// \brief 从给定的id和迭代器创建一个数据帧 当数据超长时立即终止程序
    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    __always_inline static constexpr Self from_iter(details::is_canid auto id, Iter iter) {
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
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(len)
        ));
    }

    /// \brief 尝试从给定的id和迭代器创建一个数据帧 当数据超长时返回空
    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    static constexpr Option<Self> try_from_iter(details::is_canid auto id, Iter iter) {
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
        // 使用数组视图构造CanMsg
        return Some(Self(
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(len)
        ));
    }


    /// \brief 尝试从给定的id和初始化列表创建一个数据帧 当数据超长时立即终止
    __always_inline static constexpr Self from_list(
        details::is_canid auto id, 
        const std::initializer_list<uint8_t> bytes
    ){
        if(bytes.size() > 8) [[unlikely]]
            __builtin_trap();
        U8X8 buf = ZERO_U8X8;
        std::copy(bytes.begin(), bytes.end(), buf.begin());
        return Self(
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(bytes.size())
        );
    }

    /// \brief 尝试从给定的id和初始化列表创建一个数据帧 当数据超长时返回空
    __always_inline static constexpr Option<Self> try_from_list(
        details::is_canid auto id, 
        const std::initializer_list<uint8_t> bytes
    ){
        if(bytes.size() > 8) [[unlikely]]
            return None;
        U8X8 buf = ZERO_U8X8;
        std::copy(bytes.begin(), bytes.end(), buf.begin());
        return Some(Self(
            CanIdentifier::from_parts(id, CanRtr::Data), 
            buf, 
            CanClassicDlc::from_bits(bytes.size())
        ));
    }


    /// \brief (SXX32专属)从寄存器值构造报文 不对比特做任何检查
    __always_inline static constexpr Self from_sxx32_regs(
        uint32_t id_bits, 
        uint64_t payload, 
        uint8_t len
    ){
        return Self(
            CanIdentifier::from_bits(id_bits), 
            std::bit_cast<U8X8>(payload), 
            CanClassicDlc::from_bits(len)
        );
    }

    /// \brief 获取载荷的数据长度
    [[nodiscard]] __always_inline constexpr size_t length() const {return dlc().length();}

    /// \brief 获取dlc标识符
    [[nodiscard]] __always_inline constexpr CanClassicDlc dlc() const {
        return CanClassicDlc::from_bits(dlc_);}

    [[nodiscard]] constexpr Self clone() const {
        return *this;
    }

    /// \brief 直接获取载荷的数据而不检查
    [[nodiscard]] __always_inline constexpr uint8_t operator[](size_t idx) const {return payload_bytes_[idx];}

    /// \brief 直接获取载荷的可变数据而不检查
    [[nodiscard]] __always_inline constexpr uint8_t & operator[](size_t idx) {return payload_bytes_[idx];}

    /// \brief 获取载荷的数据 如超界则立即终止
    [[nodiscard]] __always_inline constexpr uint8_t at(size_t idx) const {
        if(idx >= length()) [[unlikely]]
            __builtin_trap();
        return payload_bytes_[idx];
    }

    /// \brief 获取载荷的可变数据 如超界则立即终止
    [[nodiscard]] __always_inline constexpr uint8_t & at(size_t idx) {
        if(idx >= length()) [[unlikely]]
            __builtin_trap();
        return payload_bytes_[idx];
    }

    /// \brief 获取载荷的数据 如超界则返回空
    [[nodiscard]] __always_inline constexpr Option<uint8_t> try_at(size_t idx) const {
        if(idx >= length()) [[unlikely]]
            return None;
        return Some(static_cast<uint8_t>(payload_bytes_.at(idx)));
    }

    /// \brief 获取载荷的可变数据 如超界则返回空
    [[nodiscard]] __always_inline constexpr Option<uint8_t &> try_at(size_t idx) {
        if(idx >= length()) [[unlikely]]
            return None;
        return Some(payload_bytes_.begin() + idx);
    }

    /// \brief 获取载荷的可变数据 如超界则使使用备选值
    [[nodiscard]] __always_inline constexpr uint8_t at_or(size_t idx, uint8_t other) const {
        if(idx >= length()) [[unlikely]]
            return other;
        return payload_bytes_[idx];
    }

    /// \brief 获取载荷的切片
    [[nodiscard]] __always_inline constexpr std::span<const uint8_t> payload_bytes() const{
        return std::span(payload_bytes_.data(), length());
    }

    /// \brief 获取载荷的可变切片
    [[nodiscard]] __always_inline constexpr std::span<uint8_t> payload_bytes_mut() {
        return std::span(payload_bytes_.data(), length());
    }

    /// @brief 设置载荷数据，同时修改报文长度。如果数据超长立即终止
    template<size_t Extents>
    requires (Extents <= 8 || Extents == std::dynamic_extent)
    __always_inline constexpr void set_payload_bytes(
        std::span<const uint8_t, Extents> bytes
    ) {
        if constexpr(Extents == std::dynamic_extent)
            if(bytes.size() > 8) [[unlikely]]
                __builtin_trap();
        dlc_ = bytes.size();
        std::copy(bytes.begin(), bytes.end(), payload_bytes_.begin());
    }

    /// @brief 设置载荷数据，同时修改报文长度。如果数据超长返回错误
    template<size_t Extents>
    requires (Extents <= 8 || Extents == std::dynamic_extent)
    __always_inline constexpr Result<void, void> try_set_payload_bytes(
        std::span<const uint8_t, Extents> bytes
    ) {
        if constexpr(Extents == std::dynamic_extent)
            if(bytes.size() > 8) [[unlikely]]
                return Err();
        dlc_ = bytes.size();
        std::copy(bytes.begin(), bytes.end(), payload_bytes_.begin());
        return Ok();
    }

    __always_inline constexpr void set_payload_u64(
        uint64_t int_val
    ) {
        dlc_ = 8;
        payload_bytes_ = std::bit_cast<U8X8>(int_val);
    }


    /// @brief 是否为标准帧 
    [[nodiscard]] __always_inline constexpr bool is_standard() const {
        return identifier_.is_standard();
    }

    /// @brief 是否为拓展帧 
    [[nodiscard]] __always_inline constexpr bool is_extended() const {
        return identifier_.is_extended();
    }

    /// @brief 是否为远程帧 
    [[nodiscard]] __always_inline constexpr bool is_remote() const {
        return identifier_.is_remote();
    }

    /// @brief 不顾帧格式直接获取id的数据大小
    [[nodiscard]] __always_inline constexpr uint32_t id_u32() const {
        return identifier_.id_u32();
    }

    /// @brief 不顾帧长度直接获取载荷的64位数据
    [[nodiscard]] __always_inline constexpr uint64_t payload_u64() const {
        return std::bit_cast<uint64_t>(payload_bytes_);
    }

    /// @brief 获取首部标识符
    [[nodiscard]] __always_inline constexpr auto identifier() const {
        return identifier_;
    }
private:

    __always_inline constexpr CanClassicMsg(
        const CanIdentifier identifier, 
        const U8X8 bytes,
        const CanClassicDlc dlc
    ):
        payload_bytes_ (bytes),
        identifier_(identifier),
        dlc_(dlc.to_bits()){}

    alignas(4) U8X8 payload_bytes_;
    alignas(4) CanIdentifier identifier_;
    uint8_t dlc_;     
    /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    // uint8_t mbox_:4;
    
    // uint8_t fmi_;     
    /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */




};


}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanClassicMsg & msg);
}