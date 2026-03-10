#pragma once


#include "core/utils/Result.hpp"
#include "core/utils/nth.hpp"
#include "core/container/ringbuf.hpp"
#include "ral/can.hpp"
#include "can_layout.hpp"

#include "primitive/can/bxcan_frame.hpp"
#include "primitive/can/can_event.hpp"

#include "can_filter.hpp"


namespace ymd::lld{
Nth can_to_nth(const uintptr_t inst_base);

void can_enable_rcc(const Nth can_nth, const Enable en);
void can_set_remap(const Nth can_nth, const hal::CanRemap remap);

void can_transmit(void * p_inst, const hal::CanMailboxIndex mbox_idx, const hal::BxCanFrame & frame);
hal::BxCanFrame can_receive(void * p_inst, const hal::CanFifoIndex fifo_idx);


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
    switch(mbox_idx){

        case hal::CanMailboxIndex::_0: return 1u << 0;
        case hal::CanMailboxIndex::_1: return 1u << 8;
        case hal::CanMailboxIndex::_2: return 1u << 16;
    }
    __builtin_unreachable();
}

[[nodiscard]] static constexpr 
uint32_t can_statr_tkok_mask(const hal::CanMailboxIndex mbox_idx){ 
    switch(mbox_idx){
        case hal::CanMailboxIndex::_0: return 0b10u << 0;
        case hal::CanMailboxIndex::_1: return 0b10u << 8;
        case hal::CanMailboxIndex::_2: return 0b10u << 16;
    }
    __builtin_unreachable();
}
}