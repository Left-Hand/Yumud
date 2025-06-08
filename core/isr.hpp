#pragma once

#include "core/sys_defs.hpp"

#ifdef __cplusplus
extern "C"{
#endif


__interrupt void NMI_Handler(void);
__interrupt void HardFault_Handler(void);



#ifdef __cplusplus
}
#endif

