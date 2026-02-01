#include "hw_singleton.hpp"
#include "core/sdk.hpp"

namespace ymd::hal{

#ifdef CAN1_PRESENT
Can can1 = Can{CAN1};
#endif

#ifdef CAN2_PRESENT
Can can2 = Can{CAN2};
#endif

#ifdef CAN3_PRESENT
Can can3 = Can{CAN3};
#endif
}



extern "C"{
#ifdef CAN1_PRESENT
__interrupt void USB_HP_CAN1_TX_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_tx(ymd::hal::can1);
}

__interrupt void USB_LP_CAN1_RX0_IRQHandler() {
    ymd::hal::CanInterruptDispatcher::isr_rx0(ymd::hal::can1);
}

__interrupt void CAN1_RX1_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_rx1(ymd::hal::can1);
}


#ifdef CAN_SCE_ENABLED
__interrupt void CAN1_SCE_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_sce(ymd::hal::can1);
}
#endif
#endif

#ifdef CAN2_PRESENT
__interrupt void CAN2_TX_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_tx(ymd::hal::can2);
}

__interrupt void CAN2_RX0_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_rx0(ymd::hal::can2);
}

__interrupt void CAN2_RX1_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_rx1(ymd::hal::can2);
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN2_SCE_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_sce(ymd::hal::can2);
}

#endif
#endif

#ifdef CAN3_PRESENT
__interrupt void CAN3_TX_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_tx(ymd::hal::can3);
}

__interrupt void CAN3_RX0_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_rx0(ymd::hal::can3);
}

__interrupt void CAN3_RX1_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_rx1(ymd::hal::can3);
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN3_SCE_IRQHandler(){
    ymd::hal::CanInterruptDispatcher::isr_sce(ymd::hal::can3);
}

#endif
#endif
}