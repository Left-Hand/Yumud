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

using namespace ymd;

extern "C"{
#ifdef CAN1_PRESENT
__interrupt void USB_HP_CAN1_TX_IRQHandler(){
    hal::CanIrqHandler::isr_tx(hal::can1);
}

__interrupt void USB_LP_CAN1_RX0_IRQHandler() {
    hal::CanIrqHandler::isr_rx(
        hal::can1, 
        hal::CanFifoIndex::_0
    );
}

__interrupt void CAN1_RX1_IRQHandler(){
    hal::CanIrqHandler::isr_rx(
        hal::can1, 
        hal::CanFifoIndex::_1
    );
}


#ifdef CAN_SCE_ENABLED
__interrupt void CAN1_SCE_IRQHandler(){
    hal::CanIrqHandler::isr_sce(hal::can1);
}
#endif
#endif

#ifdef CAN2_PRESENT
__interrupt void CAN2_TX_IRQHandler(){
    hal::CanIrqHandler::isr_tx(hal::can2);
}

__interrupt void CAN2_RX0_IRQHandler(){
    hal::CanIrqHandler::isr_rx(
        hal::can2, 
        hal::CanFifoIndex::_0
    );
}

__interrupt void CAN2_RX1_IRQHandler(){
    CanIrqHandler::isr_rx(
        hal::can2, 
        hal::CanFifoIndex::_1
    );
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN2_SCE_IRQHandler(){
    hal::CanIrqHandler::isr_sce(hal::can2);
}

#endif
#endif

#ifdef CAN3_PRESENT
__interrupt void CAN3_TX_IRQHandler(){
    hal::CanIrqHandler::isr_tx(hal::can3);
}

__interrupt void CAN3_RX0_IRQHandler(){
    CanIrqHandler::isr_rx(
        hal::can3, 
        hal::CanFifoIndex::_0
    );
}

__interrupt void CAN3_RX1_IRQHandler(){
    hal::CanIrqHandler::isr_rx(
        hal::can3, 
        hal::CanFifoIndex::_1
    );
}

#ifdef CAN_SCE_ENABLED
__interrupt void CAN3_SCE_IRQHandler(){
    hal::CanIrqHandler::isr_sce(hal::can3);
}

#endif
#endif
}