#pragma once

#ifdef STM32F30x

__attribute__ ((section(".co_stack")))

/********************************************************************************
 * Section for linker script
 ********************************************************************************/

extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

extern char __StackTop;

typedef void (* pvf)();

/********************************************************************************
 * Function prototype
 ********************************************************************************/

extern int main (void);         // The entry point for the application

/********************************************************************************
 * Class
 ********************************************************************************/

#endif