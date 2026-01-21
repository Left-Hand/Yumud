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


    /// @brief  构造函数
    /// @tparam ID id 类型(CanStdId/CanExtId)
    /// @param id id对象
    /// @param rmt remote标识符(Data/Remote)
    template<details::is_canid ID>
    __attribute__((always_inline)) 
    static constexpr Self from_parts(
        const ID id,
        const CanRtr rmt
    ){
        if constexpr(std::is_same_v<ID, CanStdId>){
            return from_std_id(id, rmt);
        }else{
            return from_ext_id(id, rmt);
        }
    }

    /// @brief 从原始32位bit构造
    __attribute__((always_inline)) 
    static constexpr Self from_sxx32_reg_bits(uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    /// @brief 从未初始化值构造
    static Self from_uninitialized() {
        return Self{};
    }

    /// @brief 转换为原始32位bit
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t to_sxx32_reg_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    /// @brief 是否为拓展帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_extended() const{
        return is_extended_;
    }

    /// @brief 是否为标准帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_standard() const{
        return !is_extended();
    }

    /// @brief 是否为远程帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_remote() const {
        return is_remote_;
    }

    /// @brief 不顾帧格式，直接获取帧ID大小
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t id_u32() const {
        if(is_extended_)
            return full_id_;
        else
            return full_id_ >> (29-11);
    }

    /// @brief 尝试将帧ID转为标准帧ID
    [[nodiscard]] constexpr Option<CanStdId> try_to_stdid() const {
        if(is_extended_ == true) [[unlikely]]
            return None;
        return Some(CanStdId::from_bits(full_id_ >> (29-11)));
    }

    /// @brief 尝试将帧ID转为 帧ID
    [[nodiscard]] constexpr Option<CanExtId> try_to_extid() const {
        if(is_extended_ == false) [[unlikely]]
            return None;
        return Some(CanExtId::from_bits(full_id_));
    }

    /// @brief 不顾帧格式，直接获取标准帧
    [[nodiscard]] constexpr CanStdId to_stdid() const {
        if(is_extended_ == true) [[unlikely]]
            __builtin_trap();
        return CanStdId::from_bits((full_id_ >> (29-11)));
    }

    /// @brief 不顾帧格式，直接获取拓展帧
    [[nodiscard]] constexpr CanExtId to_extid() const {
        if(is_extended_ == false) [[unlikely]]
            __builtin_trap();
        return CanExtId::from_bits((full_id_ ));
    }

    //此位置1 启动发送请求
    const uint32_t __txrq__:1 = 1;
    
    //是否为远程帧
    uint32_t is_remote_:1;
    
    //是否为扩展帧
    uint32_t is_extended_:1;
    uint32_t full_id_:29;

private:
    __attribute__((always_inline)) 
    static constexpr Self from_std_id(
        const CanStdId id, 
        const CanRtr rtr
    ){
        return Self{
            .is_remote_ = (rtr == CanRtr::Remote), 
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