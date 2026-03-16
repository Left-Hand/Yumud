#pragma once

#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"

namespace ymd::hal{

namespace details{

//这个类与平台有关 相关标准在SXX32的can文档中定义

// 寄存器布局
    // const uint32_t __resv__:1 = 1; //硬连线到1
    // uint32_t is_remote:1;    //是否为远程帧
    // uint32_t is_extended:1;    //是否为扩展帧
    // uint32_t full_id:29; //完整id


struct alignas(4) [[nodiscard]] SXX32_CanIdentifier{
    using Self = SXX32_CanIdentifier;
    struct BitFields{
        //此位置1 启动发送请求
        uint32_t txrq:1;
        
        //是否为远程帧
        uint32_t is_remote:1;
        
        //是否为扩展帧
        uint32_t is_extended:1;
        uint32_t full_id:29;

        [[nodiscard]] constexpr uint32_t extid() const {
            return full_id;
        }

        [[nodiscard]] constexpr uint32_t stdid() const {
            return full_id >> (29 - 11);
        }
    };

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
    static constexpr Self from_sxx32_rxmir(uint32_t bits){
        return std::bit_cast<Self>(bits | 0x01);
    }

    /// @brief 从未初始化值构造
    static constexpr Self from_uninitialized() {
        return Self{};
    }

    /// @brief 转换为原始32位bit
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t to_sxx32_txmir_with_txrq() const{
        return static_cast<uint32_t>(bits | 0x01);
    }

    /// @brief 是否为拓展帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_extended() const{
        return std::bit_cast<BitFields>(bits).is_extended;
    }

    /// @brief 是否为标准帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_standard() const{
        return !is_extended();
    }

    /// @brief 是否为远程帧
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr bool is_remote() const {
        return std::bit_cast<BitFields>(bits).is_remote;
    }

    /// @brief 不顾帧格式，直接获取帧ID大小
    [[nodiscard]] __attribute__((always_inline)) 
    constexpr uint32_t id_u32() const {
        if(std::bit_cast<BitFields>(bits).is_extended)
            return std::bit_cast<BitFields>(bits).extid();
        else
            return std::bit_cast<BitFields>(bits).stdid();
    }

    /// @brief 尝试将帧ID转为标准帧ID
    __attribute__((always_inline)) 
    [[nodiscard]] constexpr Option<CanStdId> try_to_stdid() const {
        const auto fields = std::bit_cast<BitFields>(bits);
        if(fields.is_extended == true) [[unlikely]]
            return None;
        return Some(CanStdId::from_bits(fields.stdid()));
    }

    /// @brief 尝试将帧ID转为 帧ID
    __attribute__((always_inline)) 
    [[nodiscard]] constexpr Option<CanExtId> try_to_extid() const {
        const auto fields = std::bit_cast<BitFields>(bits);
        if(fields.is_extended == false) [[unlikely]]
            return None;
        return Some(CanExtId::from_bits(fields.extid()));
    }

    /// @brief 不顾帧格式，直接获取标准帧
    __attribute__((always_inline)) 
    [[nodiscard]] constexpr CanStdId to_stdid() const {
        const auto fields = std::bit_cast<BitFields>(bits);
        if(fields.is_extended == true) [[unlikely]]
            __builtin_trap();
        return CanStdId::from_bits((fields.stdid()));
    }

    /// @brief 不顾帧格式，直接获取拓展帧
    __attribute__((always_inline)) 
    [[nodiscard]] constexpr CanExtId to_extid() const {
        const auto fields = std::bit_cast<BitFields>(bits);
        if(fields.is_extended == false) [[unlikely]]
            __builtin_trap();
        return CanExtId::from_bits((fields.extid()));
    }



    uint32_t bits;
private:
    __attribute__((always_inline)) 
    static constexpr Self from_std_id(
        const CanStdId id, 
        const CanRtr rtr
    ){
        const BitFields fields = {
            .txrq = 1,
            .is_remote = (rtr == CanRtr::Remote), 
            .is_extended = false, 
            .full_id = static_cast<uint32_t>(id.to_u11()) << 18
        };

        const uint32_t bits = std::bit_cast<uint32_t>(fields);
        return Self{bits};
    }

    __attribute__((always_inline)) 
    static constexpr Self from_ext_id(
        const CanExtId id, 
        const CanRtr rtr
    ){
        const BitFields fields = {
            .txrq = 1,
            .is_remote = (rtr == CanRtr::Remote), 
            .is_extended = true, 
            .full_id = id.to_u29()
        };

        const uint32_t bits = std::bit_cast<uint32_t>(fields);
        return Self{bits};
    }
};
static_assert(sizeof(SXX32_CanIdentifier) == 4);


}
}