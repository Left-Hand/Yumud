#pragma once

#include "can_enum.hpp"
#include "can_id.hpp"
#include "can_dlc.hpp"
#include "can_identifier.hpp"

#include "core/utils/Option.hpp"


//这个文件描述了CanClassicMsg类 表示标准Can2.0(bxcan)的消息

namespace ymd::hal{

struct alignas(16) [[nodiscard]] CanClassicMsg{
public:
    constexpr CanClassicMsg() = default;

    constexpr CanClassicMsg(const CanClassicMsg & other) = default;
    
    constexpr CanClassicMsg & operator = (const CanClassicMsg & other) = default;
    constexpr CanClassicMsg(CanClassicMsg && other) = default;
    constexpr CanClassicMsg & operator = (CanClassicMsg && other) = default;


    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_empty(ID id){
        return from_uninitialized(id, CanClassicDlc::zero());
    }


    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_uninitialized(
        ID id, 
        const CanClassicDlc dlc
    ){
        return CanClassicMsg(id, dlc);
    }

    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_id_and_payload_u64(
        ID id, 
        const uint64_t payload_bits
    ){
        return CanClassicMsg(id, payload_bits);
    }


    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_remote(ID id){
        return CanClassicMsg(id, CanRtr::Remote);}

    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_bytes(ID id, 
        const std::span<const uint8_t> bytes)
        {return CanClassicMsg(id, bytes);}


    template<details::is_canid ID, size_t N>
    requires (N <= 8)
    __always_inline static constexpr CanClassicMsg from_bytes(
        ID id, 
        const std::span<const uint8_t, N> bytes
    ){
        return CanClassicMsg(id, bytes);
    }


    template<details::is_canid ID, std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, uint8_t>
    __always_inline static constexpr CanClassicMsg from_bytes(ID id, R && range){
        std::array<uint8_t, 8> buf;
        uint8_t len = 0;
        for(auto && val : range){
            buf[len] = (static_cast<uint8_t>(val));
            len++;
        }
        return CanClassicMsg(id, std::span(buf.data(), len));
    }

    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_list(
        ID id, 
        const std::initializer_list<uint8_t> bytes
    ){
        return CanClassicMsg(id, std::span<const uint8_t>(bytes.begin(), bytes.size()));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    static constexpr Option<CanClassicMsg> try_from_iter(ID id, Iter iter) {
        std::array<uint8_t, 8> buf{};
        size_t len = 0;
        
        while(iter.has_next()) {
            buf[len] = iter.next();
            len++;
            if(len >= 8) return None;
        }
        
        // 使用数组视图构造CanMsg
        return Some(CanClassicMsg::from_bytes(id, std::span{buf.data(), len}));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    __always_inline static constexpr CanClassicMsg from_iter(ID id, Iter iter) {
        std::array<uint8_t, 8> buf{};
        size_t len = 0;
        
        // 无条件读取最多8个字节
        while(len < buf.size() && iter.has_next()) {
            buf[len++] = iter.next();
        }

        // 使用unsafe方式构造CanMsg（假设调用者保证有效性）
        return CanClassicMsg::from_bytes(id, std::span{buf.data(), len});
    }


    /// \brief (SXX32专属)直接获取载荷的数据长度
    __always_inline static constexpr CanClassicMsg from_sxx32_regs(
        uint32_t id_bits, 
        uint64_t payload, 
        uint8_t len
    ){
        return CanClassicMsg(id_bits, payload, len);}

    /// \brief 直接获取载荷的数据长度
    [[nodiscard]] __always_inline constexpr size_t length() const {return dlc().length();}
    [[nodiscard]] __always_inline constexpr CanClassicDlc dlc() const {
        return CanClassicDlc::from_bits(dlc_);}

    [[nodiscard]] constexpr CanClassicMsg clone(){
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
    __always_inline constexpr void set_payload_bytes(
        std::span<const uint8_t> bytes
    ) {
        if(bytes.size() > 8) [[unlikely]]
            __builtin_trap();
        dlc_ = bytes.size();
        std::copy(bytes.begin(), bytes.end(), payload_bytes_.begin());
    }

    __always_inline constexpr void set_payload_u64(
        uint64_t int_val
    ) {
        payload_bytes_ = std::bit_cast<std::array<uint8_t, 8>>(int_val);
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

    /// @brief 邮箱编号
    [[nodiscard]] __always_inline constexpr uint8_t mailbox() const {
        return mbox_;
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
    template<details::is_canid ID>
    __always_inline constexpr CanClassicMsg(
        const ID id, 
        const CanRtr rtr
    ):
        identifier_(details::SXX32_CanIdentifier::from(id, rtr))
    {
        dlc_ = 0;
    }

    template<details::is_canid ID>
    __always_inline constexpr CanClassicMsg(
        const ID id, 
        const CanClassicDlc _dlc
    ):
        identifier_(details::SXX32_CanIdentifier::from(id, CanRtr::Data))
    {
        dlc_ = _dlc.length();
    }


    template<details::is_canid ID>
    __always_inline constexpr CanClassicMsg(
        const ID id, 
        const std::span<const uint8_t> bytes
    ) : 
        CanClassicMsg(id, CanRtr::Data)
    {
        if(bytes.size() > 8) [[unlikely]]
            __builtin_trap();

        dlc_ = bytes.size();

        #pragma GCC unroll 8
        for(size_t i = 0; i < dlc_ ; i++){
            payload_bytes_[i] = (bytes[i]);
        }

        #pragma GCC unroll 8
        for(size_t i = dlc_; i < 8; i++){
            payload_bytes_[i] = 0;
        }
    }

    template<details::is_canid ID>
    __always_inline constexpr CanClassicMsg(
        const ID id, 
        const uint64_t u64_val
    ) : 
        CanClassicMsg(id, CanRtr::Data)
    {
        dlc_ = 8;
        payload_bytes_ = std::bit_cast<std::array<uint8_t, 8>>(u64_val);
    }

    __always_inline constexpr CanClassicMsg(const uint32_t id_bits, const uint64_t data, const uint8_t dlc):
        identifier_(details::SXX32_CanIdentifier::from_bits(id_bits))
    {
        const auto buf = std::bit_cast<std::array<uint8_t, 8>>(data);

        #pragma GCC unroll 8
        for(size_t i = 0; i < dlc; i++){
            payload_bytes_[i] = buf[i];
        }
        dlc_ = dlc;
    }


    alignas(4) CanIdentifier identifier_;
    alignas(4) std::array<uint8_t, 8> payload_bytes_;
    uint8_t dlc_:4;     
    /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    uint8_t mbox_:4;
    
    uint8_t fmi_;     
    /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */
};


}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanClassicMsg & msg);
}