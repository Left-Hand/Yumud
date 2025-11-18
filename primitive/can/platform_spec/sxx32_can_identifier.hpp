#pragma once

#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"

namespace ymd::hal{

namespace details{

//这个类与平台有关 相关标准在SXX32的can文档中定义

// 寄存器布局
    // const uint32_t __resv__:1 = 1; //硬连线到1
    // uint32_t is_remote_:1;    //是否为远程帧
    // uint32_t is_extended_:1;    //是否为扩展帧
    // uint32_t full_id_:29; //完整id


struct [[nodiscard]]SXX32_CanIdentifier{
    using Self = SXX32_CanIdentifier;

    template<details::is_canid ID>
    __attribute__((always_inline)) 
    static constexpr Self from(
        const ID id,
        const CanRtr rmt
    ){
        if constexpr(std::is_same_v<ID, CanStdId>){
            return from_std_id(id, rmt);
        }else{
            return from_ext_id(id, rmt);
        }
    }

    __attribute__((always_inline)) 
    static constexpr Self from_bits(uint32_t id_bits){
        return std::bit_cast<Self>(id_bits);
    }

    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_extended() const{
        return is_extended_;
    }

    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_standard() const{
        return !is_extended();
    }

    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_remote() const {
        return is_remote_;
    }

    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t id_as_u32() const {
        if(is_extended_)
            return full_id_;
        else
            return full_id_ >> (29-11);
    }

    const uint32_t __resv__:1 = 1;
    
    //是否为远程帧
    uint32_t is_remote_:1;
    
    //是否为扩展帧
    uint32_t is_extended_:1;
    uint32_t full_id_:29;
private:
    __attribute__((always_inline)) 
    static constexpr Self from_std_id(
        const CanStdId id, 
        const CanRtr is_remote
    ){
        return Self{
            .is_remote_ = (is_remote == CanRtr::Remote), 
            .is_extended_ = false, 
            .full_id_ = static_cast<uint32_t>(id.to_u11()) << 18
        };
    }

    __attribute__((always_inline)) 
    static constexpr Self from_ext_id(
        const CanExtId id, 
        const CanRtr rtr
    ){
        return Self{
            .is_remote_ = (rtr == CanRtr::Remote), 
            .is_extended_ = true, 
            .full_id_ = id.to_u29()
        };
    }
};
static_assert(sizeof(SXX32_CanIdentifier) == 4);


}
}