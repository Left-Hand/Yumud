#pragma once

#include "can.hpp"



#ifdef CAN1_PRESENT
extern "C"{
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void);

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void);

__interrupt
void CAN1_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN1_SCE_IRQHandler(void);
#endif
}
#endif

#ifdef CAN2_PRESENT
extern "C"{
__interrupt
void CAN2_TX_IRQHandler(void);

__interrupt
void CAN2_RX0_IRQHandler(void);

__interrupt
void CAN2_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN2_SCE_IRQHandler(void);
#endif
}
#endif

#ifdef CAN3_PRESENT
extern "C"{
__interrupt
void CAN3_TX_IRQHandler(void);

__interrupt
void CAN3_RX0_IRQHandler(void);

__interrupt
void CAN3_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN3_SCE_IRQHandler(void);
#endif
}

#endif


namespace ymd::hal{
#ifdef CAN1_PRESENT
extern Can can1;
#endif

#ifdef CAN2_PRESENT
extern Can can2;
#endif

#ifdef CAN3_PRESENT
extern Can can3;
#endif
}