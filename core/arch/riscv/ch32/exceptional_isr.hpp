#pragma once

#include "core/attributes.hpp"

#ifdef __cplusplus
extern "C"{
#endif


__interrupt void NMI_Handler(void);
__interrupt void HardFault_Handler(void);



#ifdef __cplusplus
}
#endif

