#pragma once

#include "CanUtils.hpp"

#include <span>
#include <memory.h>
#include <tuple>

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
    uint32_t :1;
    
    //是否为远程帧
    uint32_t is_remote_:1;
    
    //是否为扩展帧
    uint32_t is_ext_:1;
    uint32_t id_:29;
    
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

public:
    constexpr CanMsg() = default;

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;

    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;

    constexpr CanMsg copy(){
        return *this;
    }

    constexpr CanMsg(const uint32_t id, const CanRemoteSpec remote = CanRemoteSpec::Remote){
        id_ = id;
        is_ext_ = (id > 0x7FF ? true : false);
        is_remote_ = (remote == CanRemoteSpec::Remote)? true : false;
        dlc_ = 0;
    }

    explicit constexpr CanMsg(const uint32_t id, const uint64_t data, const uint8_t dlc){
        id_ = id;
        is_ext_ = (id > 0x7FF ? true : false);
        is_remote_ = false;
        data64_ = data;
        dlc_ = dlc;
    }





    template <typename... Args>
    requires CanUtils::valid_args<Args...>
    constexpr CanMsg(const uint32_t id, const std::tuple<Args...>& tup):CanMsg(id, CanRemoteSpec::Data){
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

    constexpr CanMsg(const uint32_t id, const std::span<const std::byte> pdata) : CanMsg(id, CanRemoteSpec::Data) {
        resize(MIN(pdata.size(), 8));
        for(uint8_t i = 0; i < dlc_; i++){
            data_[i] = uint8_t(pdata[i]);
        }
        is_remote_ = false;
    }

    constexpr uint8_t * begin(){return data_;}
    constexpr uint8_t * end(){return data_ + size();}
    constexpr uint8_t * data() {return data_;}
    constexpr const uint8_t * begin() const {return data_;}
    constexpr const uint8_t * end() const {return data_ + size();}
    constexpr size_t size() const {return dlc_ & 0b111;}

    constexpr uint64_t data64() const{ return data64_;}
    constexpr uint64_t & data64() {return data64_;}

    std::span<const std::byte> span() const{
        return std::span(reinterpret_cast<const std::byte *>(begin()), size());
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

}

namespace ymd{
    class OutputStream;
    OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg);
}