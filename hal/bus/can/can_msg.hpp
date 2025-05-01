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

    union{
        struct{
            uint32_t :1;
            
            //是否为远程帧
            uint32_t is_remote_:1;
            
            //是否为扩展帧
            uint32_t is_ext_:1;
            uint32_t id_:29;
        };

        uint32_t raw_;
    };

    union alignas(4){
        uint8_t data_[8];
        uint64_t data64_;
    };
    
    
    uint8_t dlc_:4;     /* Specifies the length of the frame that will be received.
    This parameter can be a value between 0 to 8 */
    
    uint8_t mbox_:4;
    
    uint8_t fmi_;     /* Specifies the index of the filter the message stored in 
    the mailbox passes through. This parameter can be a 
    value between 0 to 0xFF */
    #pragma pack(pop)
private:
    constexpr CanMsg(const CanStdId id, const CanRemoteSpec remote){
        id_ = id.as_raw();
        is_ext_ = false;
        is_remote_ = (remote == CanRemoteSpec::Remote)? true : false;
        dlc_ = 0;
    }

    // constexpr CanMsg(const CanExtId id, const CanRemoteSpec remote){
    //     id_ = id.as_raw();
    //     is_ext_ = true;
    //     is_remote_ = (remote == CanRemoteSpec::Remote)? true : false;
    //     dlc_ = 0;
    // }

    template<typename T>
    constexpr CanMsg(const details::CanId_t<T> id, const std::span<const uint8_t> pdata) : CanMsg(id, CanRemoteSpec::Data){
        is_remote_ = false;
        resize(MIN(pdata.size(), 8));
        for(uint8_t i = 0; i < dlc_; i++){
            data_[i] = uint8_t(pdata[i]);
        }
    }
    
    constexpr CanMsg(const uint32_t raw, const uint64_t data, const uint8_t dlc){
        raw_ = raw;
        data64_ = data;
        dlc_ = dlc;
    }


    template <typename T, typename... Args>
    requires CanUtils::valid_args<Args...>
    constexpr CanMsg(const details::CanId_t<T> id, const std::tuple<Args...>& tup):CanMsg(id, CanRemoteSpec::Data){
        // std::apply(
        //     [&](auto&&... args) {
        //         ((*this << args), ...);
        //     }, tup
        // );

        // for(size_t i = 0; i < sizeof(T) and dlc_ < 8; i++){
        //     data_[dlc_++] = ((const uint8_t *)&val)[i];
        // }
        memcpy(data_, &tup, sizeof(tup));
        dlc_ = sizeof(tup);
        is_remote_ = false;
    }

public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;

    constexpr CanMsg copy(){
        return *this;
    }

    static constexpr CanMsg empty(CanStdId id){return CanMsg(id, CanRemoteSpec::Data);}

    static constexpr CanMsg from_remote(CanStdId id){return CanMsg(id, CanRemoteSpec::Remote);}
    // static constexpr CanMsg from_remote(CanExtId id){return CanMsg(id, CanRemoteSpec::Remote);}
    static constexpr CanMsg from_bytes(CanStdId id, std::span<const uint8_t> pdata){return CanMsg(id, pdata);}
    // static constexpr CanMsg from_bytes(CanExtId id, std::span<const uint8_t> pdata){return CanMsg(id, pdata);}

    static constexpr CanMsg from_regs(uint32_t raw, uint64_t data, uint8_t len){return CanMsg(raw, data, len);}

    template<typename ... Ts>
    static constexpr CanMsg from_tuple(CanStdId id, const std::tuple<Ts...>& tup){return CanMsg(id, tup);}

    // template<typename ... Ts>
    // static constexpr CanMsg from_tuple(CanExtId id, const std::tuple<Ts...>& tup){return CanMsg(id, tup);}

    constexpr uint8_t * begin(){return data_;}
    constexpr uint8_t * end(){return data_ + size();}
    constexpr uint8_t * data() {return data_;}
    constexpr const uint8_t * begin() const {return data_;}
    constexpr const uint8_t * end() const {return data_ + size();}
    constexpr size_t size() const {return dlc_ & 0b111;}
    constexpr size_t dlc() const {return dlc_;}

    constexpr uint64_t as_u64() const{ return data64_;}
    constexpr uint64_t & as_u64() {return data64_;}

    std::span<const uint8_t> to_span() const{
        return std::span(reinterpret_cast<const uint8_t *>(begin()), size());
    }

    constexpr bool is_std() const {return is_ext_ == false;}
    constexpr bool is_ext() const {return is_ext_ == true;}
    constexpr bool is_remote() const {return (is_remote_ == true);}
    constexpr uint8_t mailbox() const {return mbox_;}
    void read(uint8_t * buf, size_t len){
        if(is_remote_ == true) return;
        len = MIN(len, 8);
        // memcpy(buf, data_, len);
        for(uint8_t i = 0; i < len; i++){
            buf[i] = data_[i];
        }
        dlc_ = len;
    }

    uint32_t id() const {
        return id_;
    }

    constexpr const uint8_t operator[](const size_t index) const {return *(data_ + index);};
    constexpr uint8_t & operator[](const size_t index) {return *(data_ + index);};


    template<typename T>
    requires CanUtils::valid_arg<T>
    constexpr operator T () const {
        T ret;
        memcpy((void *)&ret, &data_, MIN(sizeof(T), size()));
        return ret;
    }

    
    void set_ext(const bool en){
        is_ext_ = (en ? true : false);
    }
    
    constexpr void resize(const size_t size) {dlc_ = size;}
};

static_assert(sizeof(CanMsg) == 16);

}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg);
}