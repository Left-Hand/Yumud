#pragma once

#include "can_enum.hpp"
#include "can_id.hpp"
#include "can_dlc.hpp"
#include "can_identifier.hpp"
#include "bxcan_payload.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"


//这个文件描述了CanClassicFrame类 表示标准Can2.0(bxcan)的消息

namespace ymd::hal{

struct alignas(16) [[nodiscard]] CanClassicFrame{
public:
    using Payload = BxCanPayload;

    using U8X8 = std::array<uint8_t, 8>;
    //这里并没有用零拷贝，原因是对齐排列的uint64比零拷贝效率更高
    static constexpr U8X8 ZERO_U8X8 = std::bit_cast<U8X8>(uint64_t(0));

    using Self = CanClassicFrame;
    constexpr CanClassicFrame(const CanClassicFrame & other) = default;
    constexpr CanClassicFrame & operator = (const CanClassicFrame & other) = default;
    constexpr CanClassicFrame(CanClassicFrame && other) = default;
    constexpr CanClassicFrame & operator = (CanClassicFrame && other) = default;
    // constexpr Self() = default;

    /// \brief 从给定的id创建一个远程帧
    __always_inline static constexpr Self from_remote(
        details::is_canid auto id
    ){
        return Self(
            CanIdentifier::from_parts(id, CanRtr::Remote), 
            Payload::zero()
        );
    }

    /// \brief 从给定的id创建一个空的数据帧

    __always_inline static constexpr Self from_empty(
        details::is_canid auto id
    ){
        return Self(CanIdentifier::from_parts(id, CanRtr::Data), Payload::zero());
    }


    __always_inline static constexpr Self from_parts(
        details::is_canid auto id,
        Payload payload
    ){
        return Self(CanIdentifier::from_parts(id, CanRtr::Data), payload);
    }

    __always_inline constexpr CanClassicFrame(
        details::is_canid auto id,
        const Payload payload
    ):
        identifier_(CanIdentifier::from_parts(id, CanRtr::Data)),
        payload_(payload){}

    /// \brief (SXX32专属)从寄存器值构造报文 不对比特做任何检查
    __always_inline static constexpr Self from_sxx32_regs(
        uint32_t id_bits, 
        uint64_t int_val, 
        uint8_t len
    ){
        return Self(
            CanIdentifier::from_bits(id_bits), 
            Payload::from_u64_and_dlc(int_val, CanClassicDlc::from_bits(len))
        );
    }



    /// \brief 获取载荷的数据长度
    [[nodiscard]] __always_inline constexpr size_t length() const {return dlc().length();}

    /// \brief 获取dlc标识符
    [[nodiscard]] __always_inline constexpr CanClassicDlc dlc() const {
        return payload_.dlc_;}

    /// \brief 获取dlc标识符
    [[nodiscard]] __always_inline constexpr const Payload & payload() const {
        return payload_;}

    [[nodiscard]] constexpr Self clone() const {
        return *this;
    }

    /// \brief 直接获取载荷的数据而不检查
    [[nodiscard]] __always_inline constexpr uint8_t operator[](size_t idx) const {return payload_[idx];}

    /// \brief 直接获取载荷的可变数据而不检查
    [[nodiscard]] __always_inline constexpr uint8_t & operator[](size_t idx) {return payload_[idx];}

    /// \brief 获取载荷的数据 如超界则立即终止
    [[nodiscard]] __always_inline constexpr uint8_t at(size_t idx) const {
        if(idx >= length()) [[unlikely]]
            __builtin_trap();
        return payload_[idx];
    }

    /// \brief 获取载荷的可变数据 如超界则立即终止
    [[nodiscard]] __always_inline constexpr uint8_t & at(size_t idx) {
        if(idx >= length()) [[unlikely]]
            __builtin_trap();
        return payload_[idx];
    }

    /// \brief 获取载荷的数据 如超界则返回空
    [[nodiscard]] __always_inline constexpr Option<uint8_t> try_at(size_t idx) const {
        if(idx >= length()) [[unlikely]]
            return None;
        return Some(static_cast<uint8_t>(payload_[idx]));
    }

    /// \brief 获取载荷的可变数据 如超界则返回空
    [[nodiscard]] __always_inline constexpr Option<uint8_t &> try_at(size_t idx) {
        if(idx >= length()) [[unlikely]]
            return None;
        return Some(payload_.begin() + idx);
    }

    /// \brief 获取载荷的可变数据 如超界则使使用备选值
    [[nodiscard]] __always_inline constexpr uint8_t at_or(size_t idx, uint8_t other) const {
        if(idx >= length()) [[unlikely]]
            return other;
        return payload_[idx];
    }

    /// \brief 获取载荷的切片
    [[nodiscard]] __always_inline constexpr std::span<const uint8_t> payload_bytes() const{
        return std::span(payload_.data(), length());
    }

    /// \brief 获取载荷的可变切片
    [[nodiscard]] __always_inline constexpr std::span<uint8_t> payload_bytes_mut() {
        return std::span(payload_.data(), length());
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
        payload_ = bytes.size();
        std::copy(bytes.begin(), bytes.end(), payload_.begin());
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
        payload_ = bytes.size();
        std::copy(bytes.begin(), bytes.end(), payload_.begin());
        return Ok();
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
        return std::bit_cast<uint64_t>(payload_.bytes_);
    }

    /// @brief 获取首部标识符
    [[nodiscard]] __always_inline constexpr auto identifier() const {
        return identifier_;
    }
private:



    alignas(4) CanIdentifier identifier_;
    Payload payload_;
    /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    // uint8_t mbox_:4;
    
    // uint8_t fmi_;     
    /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */

    __always_inline constexpr CanClassicFrame(
        CanIdentifier id,
        const Payload payload
    ):
        identifier_(id),
        payload_(payload){}


};

static_assert(sizeof(CanClassicFrame) == 16);


}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanClassicFrame & frame);
}