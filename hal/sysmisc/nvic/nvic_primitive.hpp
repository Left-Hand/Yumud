#pragma once

#include <cstdint>
#include <cstddef>

namespace ymd::hal{


#if defined(CH32V203)
#ifndef QINGKE_V4B
#define QINGKE_V4B
#endif
#endif

#if defined(CH32V307) || defined(CH32V317)
#ifndef QINGKE_V4F
#define QINGKE_V4F
#endif
#endif


#if defined(QINGKE_V4A) || defined(QINGKE_V4B) || defined(QINGKE_V4C) || defined(QINGKE_V4J) || defined(QINGKE_V4F)

// 中断 0 优先级配置：
// 对于 V4A：
// [7:4]：优先级控制位
// 若配置无嵌套，无抢占位
// 若配置 2 级嵌套，bit7 为抢
// 占位
// [3:0]：保留，固定为 0 

// 对于 V4B/C/J：
// [7:5]：优先级控制位
// 若配置无嵌套，无抢占位
// 若配置 2 级嵌套，bit7 为抢
// 占位
// [4:0]：保留，固定为 0 

// 对于 V4F：
// [7:5]：优先级控制位
// 若配置无嵌套，无抢占位
// 若配置 2 级嵌套，bit7 为抢
// 占位
// 若配置 4 级嵌套，bit7-bit6
// 为抢占位
// 若配置 8 级嵌套，bit7-bit5
// 为抢占位
// [4:0]：保留，固定为 0

struct [[nodiscard]] NvicPriorityCode final{
    using Self = NvicPriorityCode;

    uint8_t bits;

    #if defined(QINGKE_V4A)
    static constexpr size_t LOWESET_SHIFT = 4;
    static constexpr size_t MAX_NEST_DEEPTH = 2;
    static constexpr uint8_t SUB_MASK_D2 = 0b111;
    #elif defined(QINGKE_V4B) || defined(QINGKE_V4C) || defined(QINGKE_V4J)
    static constexpr size_t LOWESET_SHIFT = 5;
    static constexpr size_t MAX_NEST_DEEPTH = 2;
    static constexpr uint8_t SUB_MASK_D2 = 0b11;
    #elif defined(QINGKE_V4F)
    static constexpr size_t LOWESET_SHIFT = 4;
    static constexpr size_t MAX_NEST_DEEPTH = 8;
    static constexpr uint8_t SUB_MASK_D2 = 0b111;
    static constexpr uint8_t SUB_MASK_D4 = 0b11;
    static constexpr uint8_t SUB_MASK_D8 = 0b1;
    #endif


    __attribute__((always_inline)) 
    static constexpr Self from_u8(uint8_t bits){
        return Self{bits};
    }
    
    __attribute__((always_inline)) 
    static consteval Self highest(){
        return Self{0};
    }

    __attribute__((always_inline)) 
    static consteval Self lowest(){
        constexpr uint8_t LOWEST_CODE = static_cast<uint8_t>(0xff << LOWESET_SHIFT);
        return Self{LOWEST_CODE};
    }

    //2级嵌套
    __attribute__((always_inline)) 
    static constexpr NvicPriorityCode from_pre_sub_d2(uint8_t pre, uint8_t sub){
        if(pre > 1) __builtin_trap();
        if((sub) > SUB_MASK_D2) __builtin_trap();
        return Self{static_cast<uint8_t>((pre << 7) | ((sub & SUB_MASK_D2) << LOWESET_SHIFT))};
    }

    #if defined(QINGKE_V4F)


    //4级嵌套
    __attribute__((always_inline)) 
    static constexpr NvicPriorityCode from_pre_sub_d4(uint8_t pre, uint8_t sub){
        if(pre > 3) __builtin_trap();
        if((sub) > SUB_MASK_D4) __builtin_trap();
        return Self{static_cast<uint8_t>((pre << 6) | ((sub & SUB_MASK_D4) << LOWESET_SHIFT))};
    }

    //4级嵌套
    __attribute__((always_inline)) 
    static constexpr NvicPriorityCode from_pre_sub_d8(uint8_t pre, uint8_t sub){
        if(pre > 7) __builtin_trap();
        if((sub) > SUB_MASK_D8) __builtin_trap();
        return Self{static_cast<uint8_t>((pre << 5) | ((sub & SUB_MASK_D8) << LOWESET_SHIFT))};
    }

    #endif

    template<size_t N>
    __attribute__((always_inline)) 
    static constexpr NvicPriorityCode from_pre_sub_dn(const uint8_t pre, uint8_t sub){
        if constexpr(N == 2) return from_pre_sub_d2(pre, sub);
        #if defined(QINGKE_V4F)
        else if constexpr(N == 4) return from_pre_sub_d4(pre, sub);
        else if constexpr(N == 8) return from_pre_sub_d8(pre, sub);
        #endif
        __builtin_trap();
    }
};


}

#endif