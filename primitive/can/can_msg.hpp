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
        return from_unitialized(id, CanClassicDlc::from_zero());
    }


    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_unitialized(ID id, const CanClassicDlc dlc){
        return CanClassicMsg(id, dlc);
    }

    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_id_and_u64(ID id, const uint64_t payload_bits){
        return CanClassicMsg(id, payload_bits);
    }


    template<details::is_canid ID>
    __always_inline static constexpr CanClassicMsg from_remote(ID id){
        return CanClassicMsg(id, CanRtr::Remote);}

    template<details::is_canid ID>
    static constexpr CanClassicMsg from_bytes(ID id, 
        const std::span<const uint8_t> bytes)
        {return CanClassicMsg(id, bytes);}


    template<details::is_canid ID, size_t N>
    requires (N <= 8)
    static constexpr CanClassicMsg from_bytes(
        ID id, 
        const std::span<const uint8_t, N> bytes
    ){
        return CanClassicMsg(id, bytes);
    }


    template<details::is_canid ID, std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, uint8_t>
    static constexpr CanClassicMsg from_bytes(ID id, R && range){
        std::array<uint8_t, 8> buf;
        uint8_t len = 0;
        for(auto && val : range){
            buf[len] = (static_cast<uint8_t>(val));
            len++;
        }
        return CanClassicMsg(id, std::span(buf.data(), len));
    }

    template<details::is_canid ID>
    static constexpr CanClassicMsg from_list(
        ID id, 
        const std::initializer_list<uint8_t> bytes
    ){
        return CanClassicMsg(id, std::span<const uint8_t>(bytes.begin(), bytes.size()));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    static constexpr Option<CanClassicMsg> from_iter(ID id, Iter iter) {
        std::array<uint8_t, 8> buf{};
        size_t len = 0;
        
        while(iter.has_next() && len < buf.size()) {
            buf[len] = iter.next();
            len++;
        }
        
        // 检查是否还有剩余数据未读取
        if(iter.has_next()) {
            return None;  // 数据太长，无法放入8字节缓冲区
        }
        
        // 使用数组视图构造CanMsg
        return Some(CanClassicMsg::from_bytes(id, std::span{buf.data(), len}));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    static constexpr CanClassicMsg from_iter_unchecked(ID id, Iter iter) {
        std::array<uint8_t, 8> buf{};
        size_t len = 0;
        
        // 无条件读取最多8个字节
        while(len < buf.size() && iter.has_next()) {
            buf[len++] = iter.next();
        }

        // 使用unsafe方式构造CanMsg（假设调用者保证有效性）
        return CanClassicMsg::from_bytes(id, std::span{buf.data(), len});
    }

    static constexpr __always_inline CanClassicMsg from_sxx32_regs(
        uint32_t id_bits, 
        uint64_t payload, 
        uint8_t len
    ){
        return CanClassicMsg(id_bits, payload, len);}

    [[nodiscard]] __always_inline constexpr size_t length() const {return dlc().length();}
    [[nodiscard]] __always_inline constexpr CanClassicDlc dlc() const {
        return CanClassicDlc::from_bits(dlc_);}

    [[nodiscard]] constexpr CanClassicMsg clone(){
        return *this;
    }

    [[nodiscard]] __always_inline constexpr std::span<const uint8_t> payload_bytes() const{
        return std::span(payload_bytes_.data(), length());
    }

    [[nodiscard]] __always_inline constexpr std::span<uint8_t> mut_payload_bytes() {
        return std::span(payload_bytes_.data(), length());
    }

    template<size_t N>
    requires (N <= 8)
    [[nodiscard]] __always_inline constexpr std::span<const uint8_t, N> payload_bytes_sized() const{
        if(N > length())
            __builtin_abort();
        return std::span<const uint8_t, N>(payload_bytes_.data(), N);
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
    [[nodiscard]] __always_inline constexpr uint8_t mailbox() const {
        return mbox_;
    }

    [[nodiscard]] __always_inline constexpr uint32_t id_as_u32() const {
        return identifier_.id_as_u32();
    }

    [[nodiscard]] __always_inline constexpr uint32_t sxx32_identifier_as_u32() const {
        return identifier_.as_bits();
    }

    [[nodiscard]] constexpr Option<hal::CanStdId> stdid() const {
        if(not identifier_.is_standard()) return None;
        return Some(hal::CanStdId(identifier_.id_as_u32()));
    }

    [[nodiscard]] constexpr Option<hal::CanExtId> extid() const {
        if(not identifier_.is_extended()) return None;
        return Some(hal::CanExtId(identifier_.id_as_u32()));
    }

    [[nodiscard]] __always_inline constexpr uint64_t payload_as_u64() const {
        return std::bit_cast<uint64_t>(payload_bytes_);
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
        dlc_ = std::min(bytes.size(), size_t(8));

        #pragma GCC unroll 4
        for(size_t i = 0; i < dlc_ ; i++){
            payload_bytes_[i] = (bytes[i]);
        }

        #pragma GCC unroll 4
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

        #pragma GCC unroll 4
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