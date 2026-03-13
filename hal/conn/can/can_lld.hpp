#pragma once

#include "ral/can.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "primitive/can/can_event.hpp"
#include "primitive/can/can_filter_config.hpp"



namespace ymd::lld{


#ifdef CAN3_PRESENT
static constexpr size_t NUM_CAN_FILTERS = 14 * 3;
#else
static constexpr size_t NUM_CAN_FILTERS = 14 * 2;
#endif



Nth can_to_nth(const uintptr_t inst_base);
void can_enable_rcc(const Nth can_nth, const Enable en);
void can_set_remap(const Nth can_nth, const hal::CanRemap remap);

uint8_t my_barecan_init(void * _CANx, const void * _CAN_InitStruct);

//can发送数据帧(非ttcan)
void can_transmit_nott(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::BxCanFrame & frame
);

//can发送数据帧(ttcan)
void can_transmit_ttcan(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::BxCanFrame & frame,
    const uint16_t tick
);


hal::BxCanFrame can_receive(void * p_inst, const hal::CanFifoIndex fifo_idx);

void can_configure_filter(
    const size_t filter_nth, 
    const hal::CanFifoIndex route_fifo_idx,
    const hal::CanFilterConfig & filter_cfg
);

void can_set_filter_origin(const size_t inst_nth, const size_t origin);


[[nodiscard]] static constexpr 
size_t can_statr_mask_shift(const hal::CanMailboxIndex mbox_idx){
    switch(mbox_idx){
        case hal::CanMailboxIndex::_0: return 0;
        case hal::CanMailboxIndex::_1: return 8;
        case hal::CanMailboxIndex::_2: return 16;
    }
    __builtin_unreachable();
}

[[nodiscard]] static constexpr 
uint32_t can_tstatr_tme_mask(const hal::CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case hal::CanMailboxIndex::_0: return 1u << 26;
        case hal::CanMailboxIndex::_1: return 1u << 27;
        case hal::CanMailboxIndex::_2: return 1u << 28;
    }
    __builtin_unreachable();
}

[[nodiscard]] static constexpr 
uint32_t can_statr_rqcp_mask(const hal::CanMailboxIndex mbox_idx){ 
    return 1u << can_statr_mask_shift(mbox_idx);
}

[[nodiscard]] static constexpr 
uint32_t can_statr_tkok_mask(const hal::CanMailboxIndex mbox_idx){ 
    return 1u << (can_statr_mask_shift(mbox_idx) + 1);
}


[[nodiscard]] static constexpr 
uint32_t can_statr_alst_mask(const hal::CanMailboxIndex mbox_idx){ 
    return 1u << (can_statr_mask_shift(mbox_idx) + 2);
}

[[nodiscard]] static constexpr 
uint32_t can_statr_terr_mask(const hal::CanMailboxIndex mbox_idx){ 
    return 1u << (can_statr_mask_shift(mbox_idx) + 3);
}

[[nodiscard]] static constexpr 
uint32_t can_statr_abrq_mask(const hal::CanMailboxIndex mbox_idx){ 
    return 1u << (can_statr_mask_shift(mbox_idx) + 7);
}



}