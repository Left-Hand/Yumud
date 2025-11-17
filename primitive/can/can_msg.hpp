#pragma once

#include "can_enum.hpp"
#include "can_id.hpp"

#include <span>
#include <memory.h>
#include <tuple>
#include <ranges>

#include "core/utils/Option.hpp"

namespace ymd::hal{

namespace details{

template<typename T>
concept is_canid = 
    (std::is_same_v<std::decay_t<T>, CanStdId> 
    || std::is_same_v<std::decay_t<T>, CanExtId>)
;


struct [[nodiscard]]SXX32_CanIdentifier{

    template<details::is_canid ID>
    static constexpr SXX32_CanIdentifier from(
        const ID id,
        const CanRtr rmt
    ){
        if constexpr(std::is_same_v<ID, CanStdId>){
            return from_std_id(id, rmt);
        }else{
            return from_ext_id(id, rmt);
        }
    }

    static constexpr SXX32_CanIdentifier from_bits(uint32_t id_bits){
        return std::bit_cast<SXX32_CanIdentifier>(id_bits);
    }

    [[nodiscard]] __always_inline constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] __always_inline constexpr bool is_extended() const{
        return is_ext_;
    }

    [[nodiscard]] __always_inline constexpr bool is_standard() const{
        return !is_extended();
    }

    [[nodiscard]] __always_inline constexpr bool is_remote() const {
        return is_remote_;
    }

    [[nodiscard]] __always_inline constexpr uint32_t id_u32() const {
        if(is_ext_)
            return ext_id_;
        else
            return ext_id_ >> (29-11);
    }

    const uint32_t __resv__:1 = 1;
    
    //是否为远程帧
    uint32_t is_remote_:1;
    
    //是否为扩展帧
    uint32_t is_ext_:1;
    uint32_t ext_id_:29;
private:
    static constexpr SXX32_CanIdentifier from_std_id(
        const CanStdId id, 
        const CanRtr is_remote
    ){
        return SXX32_CanIdentifier{
            .is_remote_ = (is_remote == CanRtr::Remote), 
            .is_ext_ = false, 
            .ext_id_ = uint32_t(id.to_u11()) << 18
        };
    }

    static constexpr SXX32_CanIdentifier from_ext_id(
        const CanExtId id, 
        const CanRtr rtr
    ){
        return SXX32_CanIdentifier{
            .is_remote_ = (rtr == CanRtr::Remote), 
            .is_ext_ = true, 
            .ext_id_ = id.to_u29()
        };
    }
};
static_assert(sizeof(SXX32_CanIdentifier) == 4);

#if 0
struct [[nodiscard]] Can2B_Payload{
public:
    [[nodiscard]] __fast_inline constexpr uint8_t * data() {return buf_.data();}
    [[nodiscard]] __fast_inline constexpr uint8_t * begin() {return buf_.begin();}

    [[nodiscard]] __fast_inline constexpr const uint8_t * data() const {return buf_.data();}
    [[nodiscard]] __fast_inline constexpr const uint8_t * begin() const {return buf_.begin();}

    [[nodiscard]] __fast_inline constexpr uint8_t size() const {return length_;}
    [[nodiscard]] __fast_inline constexpr uint8_t operator[](uint8_t i) const {return buf_[i];}
private:
    std::array<uint8_t, 8> buf_;
    uint8_t length_;

    static constexpr Can2B_Payload from_bytes(const std::span<const uint8_t> pbuf){
        Can2B_Payload ret;
        std::copy(pbuf.begin(), pbuf.end(), ret.begin());
        ret.length_ = pbuf.size();
        return ret;
    }

    friend class CanMsg;
};

static_assert(sizeof(Can2B_Payload) == 8);

#endif

}


struct alignas(16) [[nodiscard]] CanMsg{
public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;


    template<details::is_canid ID>
    static constexpr CanMsg from_empty(ID id){
        return CanMsg(id, CanRtr::Data);}

    template<details::is_canid ID>
    static constexpr CanMsg from_remote(ID id){
        return CanMsg(id, CanRtr::Remote);}

    template<details::is_canid ID>
    static constexpr CanMsg from_bytes(ID id, 
        const std::span<const uint8_t> pbuf)
        {return CanMsg(id, pbuf);}


    template<details::is_canid ID, size_t N>
    requires (N <= 8)
    static constexpr CanMsg from_bytes(
        ID id, 
        const std::span<const uint8_t, N> pbuf
    ){
        return CanMsg(id, pbuf);
    }


    template<details::is_canid ID, std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, uint8_t>
    static constexpr CanMsg from_bytes(ID id, R && range){
        std::array<uint8_t, 8> buf;
        uint8_t len = 0;
        for(auto && val : range){
            buf[len] = (static_cast<uint8_t>(val));
            len++;
        }
        return CanMsg(id, std::span(buf.data(), len));
    }

    template<details::is_canid ID>
    static constexpr CanMsg from_list(
        ID id, 
        const std::initializer_list<uint8_t> pbuf
    ){
        return CanMsg(id, std::span<const uint8_t>(pbuf.begin(), pbuf.size()));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    static constexpr Option<CanMsg> from_iter(ID id, Iter iter) {
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
        return Some(CanMsg::from_bytes(id, std::span{buf.data(), len}));
    }

    template<details::is_canid ID, typename Iter>
    requires is_next_based_iter_v<Iter>
    static constexpr CanMsg from_iter_unchecked(ID id, Iter iter) {
        std::array<uint8_t, 8> buf{};
        size_t len = 0;
        
        // 无条件读取最多8个字节
        while(len < buf.size() && iter.has_next()) {
            buf[len++] = iter.next();
        }

        // 使用unsafe方式构造CanMsg（假设调用者保证有效性）
        return CanMsg::from_bytes(id, std::span{buf.data(), len});
    }

    static constexpr CanMsg from_sxx32_regs(uint32_t id_bits, uint64_t payload, uint8_t len){
        return CanMsg(id_bits, payload, len);}

    [[nodiscard]] constexpr size_t size() const {return dlc_;}
    [[nodiscard]] constexpr size_t dlc() const {return dlc_;}

    [[nodiscard]] constexpr CanMsg clone(){
        return *this;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> payload_bytes() const{
        return std::span(payload_bytes_.begin(), size());
    }

    template<size_t N>
    requires (N <= 8)
    [[nodiscard]] constexpr std::span<const uint8_t, N> payload_bytes_with_length() const{
        if(N > size())
            __builtin_abort();
        return std::span<const uint8_t, N>(payload_bytes_.data(), N);
    }

    [[nodiscard]] constexpr bool is_standard() const {return identifier_.is_standard();}
    [[nodiscard]] constexpr bool is_extended() const {return identifier_.is_extended();}
    [[nodiscard]] constexpr bool is_remote() const {return identifier_.is_remote();}
    [[nodiscard]] constexpr uint8_t mailbox() const {return mbox_;}

    [[nodiscard]] constexpr uint32_t id_as_u32() const {
        return identifier_.id_u32();
    }

    [[nodiscard]] constexpr uint32_t sxx32_identifier_as_u32() const {
        return identifier_.as_bits();
    }

    [[nodiscard]] constexpr Option<hal::CanStdId> stdid() const {
        if(not identifier_.is_standard()) return None;
        return Some(hal::CanStdId(identifier_.id_u32()));
    }

    [[nodiscard]] constexpr Option<hal::CanExtId> extid() const {
        if(not identifier_.is_extended()) return None;
        return Some(hal::CanExtId(identifier_.id_u32()));
    }

    [[nodiscard]] constexpr uint64_t payload_as_u64() const {
        return std::bit_cast<uint64_t>(payload_bytes_);
    }



private:
    template<details::is_canid ID>
    __fast_inline constexpr CanMsg(const ID id, const CanRtr remote):
        identifier_(details::SXX32_CanIdentifier::from(id, remote))
    {
        dlc_ = 0;
    }

    template<details::is_canid ID>
    __fast_inline constexpr CanMsg(
            const ID id, 
            const std::span<const uint8_t> pbuf
    ) : 
        CanMsg(id, CanRtr::Data)
    {
        dlc_ = std::min(pbuf.size(), size_t(8));

        for(size_t i = 0; i < dlc_ ; i++){
            payload_bytes_[i] = (pbuf[i]);
        }

        for(size_t i = dlc_; i < 8; i++){
            payload_bytes_[i] = 0;
        }
    }

    __fast_inline constexpr CanMsg(const uint32_t id_bits, const uint64_t data, const uint8_t dlc):
        identifier_(details::SXX32_CanIdentifier::from_bits(id_bits))
    {
        const auto buf = std::bit_cast<std::array<uint8_t, 8>>(data);
        for(size_t i = 0; i < dlc; i++){
            payload_bytes_[i] = buf[i];
        }
        dlc_ = dlc;
    }

    details::SXX32_CanIdentifier identifier_;
    std::array<uint8_t, 8> payload_bytes_;

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
    OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg);
}