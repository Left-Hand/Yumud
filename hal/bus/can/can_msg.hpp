#pragma once

#include "can_utils.hpp"
#include "can_id.hpp"

#include <span>
#include <memory.h>
#include <tuple>
#include <ranges>

#include "core/utils/Option.hpp"

namespace ymd::hal{

namespace details{
template<typename T>
concept valid_arg = (sizeof(T) <= 8) 
    && (std::is_same_v<std::decay_t<T>, T>) 
;

template<typename ... Args>
concept valid_args = 
    (sizeof(std::tuple<Args...>) <= 8) 
    && (std::is_same_v<std::decay_t<Args>, Args> && ...)
;

template<typename T>
concept is_canid = 
    (std::is_same_v<std::decay_t<T>, CanStdId> 
    || std::is_same_v<std::decay_t<T>, CanExtId>)
;

struct CanMsg_Prelude{


    #pragma pack(push, 1)

    struct SXX32_CanIdentifier{

        template<details::is_canid ID>
        static constexpr SXX32_CanIdentifier from(
            const ID id,
            const CanRemoteSpec rmt
        ){
            if constexpr(std::is_same_v<ID, CanStdId>){
                return from_std_id(id, rmt);
            }else{
                return from_ext_id(id, rmt);
            }
        }

        static constexpr SXX32_CanIdentifier from_raw(uint32_t raw){
            return std::bit_cast<SXX32_CanIdentifier>(raw);
        }

        constexpr uint32_t to_raw() const{
            return std::bit_cast<uint32_t>(*this);
        }

        constexpr bool is_extended() const{
            return is_ext_;
        }

        constexpr bool is_standard() const{
            return !is_extended();
        }

        constexpr bool is_remote() const {
            return is_remote_;
        }

        constexpr uint32_t id() const {
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
            const CanRemoteSpec is_remote
        ){
            return SXX32_CanIdentifier{
                .is_remote_ = (is_remote == CanRemoteSpec::Remote), 
                .is_ext_ = false, 
                .ext_id_ = uint32_t(id.to_u11()) << 18
            };
        }

        static constexpr SXX32_CanIdentifier from_ext_id(
            const CanExtId id, 
            const CanRemoteSpec is_remote
        ){
            return SXX32_CanIdentifier{
                .is_remote_ = (is_remote == CanRemoteSpec::Remote), 
                .is_ext_ = true, 
                .ext_id_ = id.to_u29()
            };
        }
    };


    static_assert(sizeof(SXX32_CanIdentifier) == 4);

    struct Payload{
    public:
        constexpr uint8_t * data() {return buf_.data();}
        constexpr uint8_t * begin() {return buf_.begin();}

        constexpr const uint8_t * data() const {return buf_.data();}
        constexpr const uint8_t * begin() const {return buf_.begin();}

        constexpr uint8_t size() const {return length_;}
        constexpr uint8_t operator[](uint8_t i) const {return buf_[i];}
    private:
        std::array<uint8_t, 8> buf_;
        uint8_t length_;

        static constexpr Payload from_pu8(const std::span<const uint8_t> pbuf){
            Payload ret;
            std::copy(pbuf.begin(), pbuf.end(), ret.begin());
            ret.length_ = pbuf.size();
            return ret;
        }

        friend class CanMsg;
    };
    #pragma pack(pop)
};

}


struct alignas(16) CanMsg final:public details::CanMsg_Prelude{
public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;


    template<details::is_canid ID>
    static constexpr CanMsg empty(ID id){
        return CanMsg(id, CanRemoteSpec::Data);}

    template<details::is_canid ID>
    static constexpr CanMsg from_remote(ID id){
        return CanMsg(id, CanRemoteSpec::Remote);}

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

    static constexpr CanMsg from_sxx32_regs(uint32_t raw, uint64_t payload, uint8_t len){
        return CanMsg(raw, payload, len);}

    constexpr size_t size() const {return dlc_;}
    constexpr size_t dlc() const {return dlc_;}

    constexpr CanMsg clone(){
        return *this;
    }

    constexpr std::span<const uint8_t> iter_payload() const{
        return std::span(payload_.begin(), size());
    }

    template<size_t N>
    requires (N <= 8)
    constexpr std::span<const uint8_t, N> to_span_with_length() const{
        if(N <= size())
            return std::span<const uint8_t, N>(payload_.data(), N);
        else __builtin_abort();
    }

    constexpr bool is_standard() const {return identifier_.is_standard();}
    constexpr bool is_extended() const {return identifier_.is_extended();}
    constexpr bool is_remote() const {return identifier_.is_remote();}
    constexpr uint8_t mailbox() const {return mbox_;}

    constexpr uint32_t id_as_u32() const {
        return identifier_.id();
    }

    constexpr Option<hal::CanStdId> stdid() const {
        if(not identifier_.is_standard()) return None;
        return Some(hal::CanStdId(identifier_.id()));
    }

    constexpr Option<hal::CanExtId> extid() const {
        if(not identifier_.is_extended()) return None;
        return Some(hal::CanExtId(identifier_.id()));
    }

    constexpr uint64_t payload_as_u64() const {
        return std::bit_cast<uint64_t>(payload_);
    }

    constexpr uint32_t sxx32_identifier_as_u32() const {
        return identifier_.to_raw();
    }

private:
    template<details::is_canid ID>
    __fast_inline constexpr CanMsg(const ID id, const CanRemoteSpec remote):
        identifier_(SXX32_CanIdentifier::from(id, remote))
    {
        dlc_ = 0;
    }

    template<details::is_canid ID>
    __fast_inline constexpr CanMsg(
            const ID id, 
            const std::span<const uint8_t> pbuf
    ) : 
        CanMsg(id, CanRemoteSpec::Data)
    {
        dlc_ = MIN(pbuf.size(), 8);

        for(size_t i = 0; i < dlc_ ; i++){
            payload_[i] = (pbuf[i]);
        }

        for(size_t i = dlc_; i < 8; i++){
            payload_[i] = 0;
        }
    }

    __fast_inline constexpr CanMsg(const uint32_t raw, const uint64_t data, const uint8_t dlc):
        identifier_(SXX32_CanIdentifier::from_raw(raw))
    {
        const auto buf = std::bit_cast<std::array<uint8_t, 8>>(data);
        for(size_t i = 0; i < dlc; i++){
            payload_[i] = buf[i];
        }
        dlc_ = dlc;
    }

    SXX32_CanIdentifier identifier_;
    std::array<uint8_t, 8> payload_;

    uint8_t dlc_:4;     
    /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    uint8_t mbox_:4;
    
    uint8_t fmi_;     
    /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */
};

// static_assert(sizeof(CanMsg) == 16);

}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg);
}