#pragma once


#include <ostream>

#include "sys/core/system.hpp"
#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"


namespace RiscvCpu{
    struct sp{};
    struct t0{};
    struct mepc{};
    struct mstatus{};

    struct x1{};
    struct x3{};
    struct x4{};
    struct x5{};
    struct x6{};
    struct x7{};
    struct x8{};
    struct x9{};
    struct x10{};
    struct x11{};
    struct x12{};
    struct x13{};
    struct x14{};
    struct x15{};
    struct x16{};
    struct x17{};
    struct x18{};
    struct x19{};
    struct x20{};
    struct x21{};
    struct x22{};
    struct x23{};
    struct x24{};
    struct x25{};
    struct x26{};
    struct x27{};
    struct x28{};
    struct x29{};
    struct x30{};
    struct x31{};

    #ifdef __riscv_f
    struct f1{};
    struct f2{};
    struct f3{};
    struct f4{};
    struct f5{};
    struct f6{};
    struct f7{};
    struct f8{};
    struct f9{};
    struct f10{};
    struct f11{};
    struct f12{};
    struct f13{};
    struct f14{};
    struct f15{};
    struct f16{};
    struct f17{};
    struct f18{};
    struct f19{};
    struct f20{};
    struct f21{};
    struct f22{};
    struct f23{};
    struct f24{};
    struct f25{};
    struct f26{};
    struct f27{};
    struct f28{};
    struct f29{};
    struct f30{};
    struct f31{};
    #endif

    struct TaskCtrlBlock{
        using Entry = void(void *);

        enum class State:uint8_t{
            Running,
            Sleeping,
            Waiting,
            Blocked,
            Suspended,
            Deleted
        };

        using Priority = uint8_t;

        void * sp;
        const char * p_name;
        Entry * p_entry;
        const void * p_args;
        State state;
        Priority priority;

        uint8_t * stask_base;
        size_t stack_size;
    };

    __attribute__((noreturn))
    void restore_context(void);

    void int_disable(void){
        asm("csrw mstatus, %0" : :"r"(0x1800));
    }
    
    void int_enable(void){
        asm("csrw mstatus, %0" : :"r"(0x1888));
    }
    
    size_t cpsr_save(void){
        size_t value;
        asm("csrrw %0, mstatus, %1":"=r"(value):"r"(0x1800));
        return value;
    }

    void cpsr_restore(size_t cpsr){
      asm("csrw mstatus, %0": :"r"(cpsr));
    }
    
    
    void cpu_reset(void){
        NVIC_SystemReset();
    }
    
    void sw_clearpend(void)
    {
        SysTick->CTLR &= ~(1<<31);
    }
    
    void context_switch(void)
    {
        SysTick->CTLR |= (1<<31);
    }
    
    void irq_context_switch(void)
    {
        SysTick->CTLR |= (1<<31);
    }
    
    void systick_config(uint32_t cycle_per_tick)
    {
        SysTick->CTLR=0;
        SysTick->SR=0;
        SysTick->CNT=0;
        SysTick->CMP=cycle_per_tick-1;
        SysTick->CTLR=0xF;
    }
}

extern "C"{

__interrupt __attribute__((used))
__attribute__((noreturn))
void SW_Handler(void);
}