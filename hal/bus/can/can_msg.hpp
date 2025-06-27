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

struct CanMsgDetails{


    #pragma pack(push, 1)

    struct SXX32_CanIdentifier{

        template<typename T>
        static constexpr SXX32_CanIdentifier from(
            const details::CanId_t<T> id,
            const CanRemoteSpec rmt
        ){
            if constexpr(std::is_same_v<details::CanId_t<T>, CanStdId>){
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

        constexpr bool is_ext() const{
            return is_ext_;
        }

        constexpr bool is_std() const{
            return !is_ext();
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
                .ext_id_ = uint32_t(id.as_raw()) << 18
            };
        }

        static constexpr SXX32_CanIdentifier from_ext_id(
            const CanExtId id, 
            const CanRemoteSpec is_remote
        ){
            return SXX32_CanIdentifier{
                .is_remote_ = (is_remote == CanRemoteSpec::Remote), 
                .is_ext_ = true, 
                .ext_id_ = id.as_raw()
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


struct alignas(16) CanMsg final:public details::CanMsgDetails{
public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;


    template<typename T>
    static constexpr CanMsg empty(details::CanId_t<T> id){
        return CanMsg(id, CanRemoteSpec::Data);}

    template<typename T>
    static constexpr CanMsg from_remote(details::CanId_t<T> id){
        return CanMsg(id, CanRemoteSpec::Remote);}

    template<typename T>
    static constexpr CanMsg from_bytes(details::CanId_t<T> id, 
        const std::span<const uint8_t> pbuf)
        {return CanMsg(id, pbuf);}


    template<typename T, size_t N>
    requires (N <= 8)
    static constexpr CanMsg from_bytes(
        details::CanId_t<T> id, 
        const std::span<const uint8_t, N> pbuf
    ){
        return CanMsg(id, pbuf);
    }


    template<typename T, std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, uint8_t>
    static constexpr CanMsg from_bytes(details::CanId_t<T> id, R && range){
        std::array<uint8_t, 8> buf;
        uint8_t len = 0;
        for(auto && val : range){
            buf[len] = (static_cast<uint8_t>(val));
            len++;
        }
        return CanMsg(id, std::span(buf.data(), len));
    }

    template<typename T>
    static constexpr CanMsg from_list(
        details::CanId_t<T> id, 
        const std::initializer_list<uint8_t> pbuf
    ){
        return CanMsg(id, std::span<const uint8_t>(pbuf.begin(), pbuf.size()));
    }

    static constexpr CanMsg from_sxx32_regs(uint32_t raw, uint64_t payload, uint8_t len){
        return CanMsg(raw, payload, len);}

    constexpr size_t size() const {return dlc_;}
    constexpr size_t dlc() const {return dlc_;}

    constexpr CanMsg clone(){
        return *this;
    }

    constexpr std::span<const uint8_t> payload() const{
        return std::span(payload_.begin(), size());
    }

    template<size_t N>
    requires (N <= 8)
    constexpr std::span<const uint8_t, N> to_span_with_length() const{
        if(N <= size())
            return std::span<const uint8_t, N>(payload_.data(), N);
        else __builtin_abort();
    }

    constexpr bool is_std() const {return identifier_.is_std();}
    constexpr bool is_ext() const {return identifier_.is_ext();}
    constexpr bool is_remote() const {return identifier_.is_remote();}
    constexpr uint8_t mailbox() const {return mbox_;}

    constexpr uint32_t id_as_u32() const {
        return identifier_.id();
    }

    constexpr Option<hal::CanStdId> stdid() const {
        if(not identifier_.is_std()) return None;
        return Some(hal::CanStdId::from_raw(identifier_.id()));
    }

    constexpr Option<hal::CanExtId> extid() const {
        if(not identifier_.is_ext()) return None;
        return Some(hal::CanExtId::from_raw(identifier_.id()));
    }

    constexpr uint64_t payload_as_u64() const {
        return std::bit_cast<uint64_t>(payload_);
    }

    constexpr uint32_t sxx32_identifier_as_u32() const {
        return identifier_.to_raw();
    }

private:
    template<typename T>
    __fast_inline constexpr CanMsg(const details::CanId_t<T> id, const CanRemoteSpec remote):
        identifier_(SXX32_CanIdentifier::from(id, remote))
    {
        dlc_ = 0;
    }

    template<typename T>
    __fast_inline constexpr CanMsg(
            const details::CanId_t<T> id, 
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