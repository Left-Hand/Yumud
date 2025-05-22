#pragma once

#include "can_utils.hpp"
#include "can_id.hpp"

#include <span>
#include <memory.h>
#include <tuple>

namespace ymd{
class OutputStream;
}
namespace ymd::hal{

namespace CanUtils{
    template<typename T>
    concept valid_arg = (sizeof(T) <= 8) 
        && (std::is_same_v<std::decay_t<T>, T>) 
    ;

    template<typename ... Args>
    concept valid_args = 
        (sizeof(std::tuple<Args...>) <= 8) 
        && (std::is_same_v<std::decay_t<Args>, Args> && ...)
    ;
}


struct CanMsg{
protected:
    #pragma pack(push, 1)

    struct Identifier{

        template<typename T>
        static constexpr Identifier from(
            const details::CanId_t<T> id,
            const CanRemoteSpec rmt
        ){
            if constexpr(std::is_same_v<details::CanId_t<T>, CanStdId>){
                return from_std_id(id, rmt);
            }else{
                return from_ext_id(id, rmt);
            }
        }

        static constexpr Identifier from_raw(uint32_t raw){
            return std::bit_cast<Identifier>(raw);
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
                return id_;
            else
                return id_ >> 18;
        }

    // private:
        const uint32_t __resv__:1 = 1;
        
        //是否为远程帧
        uint32_t is_remote_:1;
        
        //是否为扩展帧
        uint32_t is_ext_:1;
        uint32_t id_:29;
    private:
        static constexpr Identifier from_std_id(
            const CanStdId id, 
            const CanRemoteSpec is_remote
        ){
            return Identifier{
                .is_remote_ = (is_remote == CanRemoteSpec::Remote), 
                .is_ext_ = false, 
                .id_ = uint32_t(id.as_raw() << 18)
            };
        }

        static constexpr Identifier from_ext_id(
            const CanExtId id, 
            const CanRemoteSpec is_remote
        ){
            return Identifier{
                .is_remote_ = (is_remote == CanRemoteSpec::Remote), 
                .is_ext_ = true, 
                .id_ = id.as_raw()
            };
        }
    };

    Identifier identifier_;
    static_assert(sizeof(Identifier) == 4);

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

    std::array<uint8_t, 8> buf_;

    
    
    uint8_t dlc_:4;     /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    uint8_t mbox_:4;
    
    uint8_t fmi_;     /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */
    #pragma pack(pop)
private:
    template<typename T>
    __fast_inline constexpr CanMsg(const details::CanId_t<T> id, const CanRemoteSpec remote):
        identifier_(Identifier::from(id, remote))
    {
        dlc_ = 0;
    }

    template<typename T>
    __fast_inline constexpr CanMsg(
            const details::CanId_t<T> id, 
            const std::span<const uint8_t> pdata
    ) : 
        CanMsg(id, CanRemoteSpec::Data)
    {
        dlc_ = MIN(pdata.size(), 8);

        for(size_t i = 0; i < size(); i++){
            buf_[i] = (pdata[i]);
        }

        for(size_t i = size(); i < 8; i++){
            buf_[i] = 0;
        }
    }

    __fast_inline constexpr CanMsg(const uint32_t raw, const uint64_t data, const uint8_t dlc):
        identifier_(Identifier::from_raw(raw))
    {
        const auto buf = std::bit_cast<std::array<uint8_t, 8>>(data);
        for(size_t i = 0; i < dlc; i++){
            buf_[i] = buf[i];
        }
        dlc_ = dlc;
    }

    template <typename T, typename... Args>
    requires CanUtils::valid_args<Args...>
    __fast_inline constexpr CanMsg(const details::CanId_t<T> id, const std::tuple<Args...>& tup):
        CanMsg(id, CanRemoteSpec::Data)
    {
        memcpy(buf_.begin(), &tup, sizeof(tup));
        dlc_ = sizeof(tup);
    }

public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;


    template<typename T>
    static constexpr CanMsg empty(details::CanId_t<T> id){return CanMsg(id, CanRemoteSpec::Data);}

    template<typename T>
    static constexpr CanMsg from_remote(details::CanId_t<T> id){return CanMsg(id, CanRemoteSpec::Remote);}

    template<typename T>
    static constexpr CanMsg from_bytes(details::CanId_t<T> id, const std::span<const uint8_t> pdata)
        {return CanMsg(id, pdata);}

    template<typename T>
    static constexpr CanMsg from_list(details::CanId_t<T> id, const std::initializer_list<uint8_t> pdata)
        {return CanMsg(id, std::span<const uint8_t>(pdata.begin(), pdata.size()));}

    static constexpr CanMsg from_regs(uint32_t raw, uint64_t data, uint8_t len){
        return CanMsg(raw, data, len);}

    template<typename ... Ts, typename T>
    static constexpr CanMsg from_tuple(details::CanId_t<T> id, const std::tuple<Ts...>& tup){
        return CanMsg(id, tup);}

    constexpr uint8_t * begin(){return buf_.begin();}
    constexpr uint8_t * end(){return buf_.end();}
    constexpr uint8_t * data() {return buf_.data();}
    constexpr const uint8_t * begin() const {return buf_.begin();}
    constexpr const uint8_t * end() const {return buf_.end();}

    constexpr const uint8_t operator[](const size_t index) const {return buf_[index];};
    constexpr uint8_t & operator[](const size_t index) {return buf_[index];};

    constexpr size_t size() const {return dlc_ & 0b111;}
    constexpr size_t dlc() const {return dlc_;}

    constexpr CanMsg clone(){
        return *this;
    }

    constexpr std::span<const uint8_t> payload() const{
        return std::span(begin(), size());
    }

    template<size_t N>
    requires (N <= 8)
    constexpr std::span<const uint8_t, N> to_span_with_length() const{
        if(N <= size())
            return std::span<const uint8_t, N>(begin(), N);
        else __builtin_abort();
    }

    constexpr bool is_std() const {return identifier_.is_std();}
    constexpr bool is_ext() const {return identifier_.is_ext();}
    constexpr bool is_remote() const {return identifier_.is_remote();}
    constexpr uint8_t mailbox() const {return mbox_;}

    constexpr uint32_t id() const {
        return identifier_.id();
    }


    template<typename T>
    requires CanUtils::valid_arg<T>
    constexpr T to() const {
        T ret;
        memcpy((void *)&ret, &buf_, MIN(sizeof(T), size()));
        return ret;
    }

    constexpr uint64_t payload_as_u64() const {
        return std::bit_cast<uint64_t>(buf_);
    }

    constexpr uint32_t identifier_as_u32() const {
        return identifier_.to_raw();
    }

    
    // constexpr void set_ext(const bool en){
    //     is_ext_ = (en ? true : false);
    // }
    
    // constexpr void set_size(const size_t size) {dlc_ = size;}
};

// static_assert(sizeof(CanMsg) == 16);

}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg);
}