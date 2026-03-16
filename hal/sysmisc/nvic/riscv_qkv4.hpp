#pragma once

#include <bit>
#include "core/arch/riscv/riscv_v4c.hpp"

namespace ymd::arch::riscv::qkv4c{

__attribute__((always_inline))
R32_INTSYSCR get_intsyscr(void){
    uint32_t result;

    __asm volatile ("csrr %0," "intsyscr" : "=r" (result) );
    return std::bit_cast<R32_INTSYSCR>(result);
}

__attribute__((always_inline))
R32_CFGR get_cfgr(void){
    uint32_t result;
    __asm volatile("csrr %0," "cfgr" : "=r" (result));
    return std::bit_cast<R32_CFGR>(result);
}

__attribute__((always_inline))
R32_GISR get_gisr(void){
    uint32_t result;
    __asm volatile("csrr %0," "gisr" : "=r" (result));
    return std::bit_cast<R32_GISR>(result);
}

__attribute__((always_inline))
R32_SCTLR get_sctlr(void){
    uint32_t result;
    __asm volatile("csrr %0," "ctlr" : "=r" (result));
    return std::bit_cast<R32_SCTLR>(result);
}

__attribute__((always_inline))
R32_INTSYSCR get_intsyscr(void){
    uint32_t result;
    __asm volatile("csrr %0," "yscr" : "=r" (result));
    return std::bit_cast<R32_INTSYSCR>(result);
}

__attribute__((always_inline))
R32_MTVEC get_mtvec(void){
    uint32_t result;
    __asm volatile("csrr %0," "tvec" : "=r" (result));
    return std::bit_cast<R32_MTVEC>(result);
}

__attribute__((always_inline))
R32_ITHRESDR get_ithresdr(void){
    uint32_t result;
    __asm volatile("csrr %0," "esdr" : "=r" (result));
    return std::bit_cast<R32_ITHRESDR>(result);
}

}