#include "isr.hpp"
#include "core/debug/debug.hpp"
#include "core/tmp/reflect/enum.hpp"

#include "arch/riscv/cpu.hpp"
#include "sdk.hpp"
#include "core/system.hpp"

using namespace ymd;


enum class FaultMcause:uint8_t{
    InstructionAddressMisaligned        = 0,
    InstructionAccessFault              = 1,
    IllegalInstruction,
    Breakpoint,
    LoadAddressMisaligned,
    LoadAccessFault,
    StoreAddressMisaligned,
    StoreAccessFault,
    EnvironmentCallFromUmode,
    EnvironmentCallFromSmode,
    EnvironmentCallFromHmode,
    EnvironmentCallFromMmode,
    Reserved,
};

DEF_DERIVE_DEBUG(FaultMcause)


void NMI_Handler(void){
    while(true);
}


#define PRINT_HARDFULT_CAUSE 0

void HardFault_Handler(void){
    sys::trip();

    #if PRINT_HARDFULT_CAUSE
    DEBUG_PRINTLN("hardfault\n");
    
    const auto v_mepc=__get_MEPC();
    const auto v_mcause=__get_MCAUSE();
    const auto v_mtval=__get_MTVAL();

    DEBUG_PRINTLN("mepc", std::hex, v_mepc);
    DEBUG_PRINTLN("mcause", std::hex, v_mcause, FaultMcause(uint8_t(v_mcause)));
    DEBUG_PRINTLN("mtval", std::hex, v_mtval);
    clock::delay(10ms);


    #endif
    while(true);
}
