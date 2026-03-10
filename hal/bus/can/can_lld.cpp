#include "can.hpp"
#include "can_filter.hpp"

#include "can_layout.hpp"

#include "hal/sysmisc/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/sdk.hpp"
#include "ral/can.hpp"

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, CAN_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::CAN_Def)>(x))

namespace ymd::lld{

Nth can_to_nth(const uintptr_t inst_base){
    switch(inst_base){
        #ifdef CAN1_PRESENT
        case CAN1_BASE:
            return Nth(1);
        #endif
        #ifdef CAN2_PRESENT
        case CAN2_BASE:
            return Nth(2);
        #endif
        #ifdef CAN3_PRESENT
        case CAN3_BASE:
            return Nth(3);
        #endif
    }
    __builtin_trap();
}


void can_enable_rcc(const Nth can_nth, const Enable en){
    switch(can_nth.count()){
        #ifdef CAN1_PRESENT
        case 1:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
            return;
        }
        #endif

        #ifdef CAN2_PRESENT
        case 2:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
            return;
        }
        #endif

        #ifdef CAN3_PRESENT
        case 3:{
            // TODO: 暂不支持CAN3
            __builtin_trap();
        }
        #endif
    }

    //如果运行到这里 说明调用了预期外的外设 请检查是否正确配置开关宏
    __builtin_trap();
}


void can_set_remap(const Nth can_nth, const hal::CanRemap remap){
    switch(can_nth.count()){
        #ifdef CAN1_PRESENT
        case 1:{
            switch(remap){
                case hal::CanRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
                    return;
                case hal::CanRemap::_1:
                case hal::CanRemap::_2:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
                    return;
                case hal::CanRemap::_3:
                    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);//for TEST
                    GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);
                    return;
            }
        }
        break;
        #endif

        #ifdef CAN2_PRESENT
        case 2:{
            switch(remap){
                case hal::CanRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, DISABLE);
                    return;
                case hal::CanRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
                    return;
            }
        }
        break;
        #endif

        #ifdef CAN3_PRESENT
        case 3:{
            //还未实现
            __builtin_trap();
        }
        break;
        #endif
    }
    //如果运行到这里 说明调用了预期外的外设 请检查是否正确配置开关宏
    __builtin_trap();
}


void can_transmit(void * p_inst, const hal::CanMailboxIndex mbox_idx, const hal::BxCanFrame & frame){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    auto & mailbox_inst = SDK_INST(p_inst)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_reg_bits();
    const uint64_t payload_u64 = frame.payload_u64();


    mailbox_inst.TXMDTR = uint32_t(0xFFFF0000 | (frame.dlc().to_bits() & 0xf));

    //将低四字节装载到txmdlr
    mailbox_inst.TXMDLR = static_cast<uint32_t>(payload_u64);

    //将高四字节装载到txmdhr
    mailbox_inst.TXMDHR = static_cast<uint32_t>(payload_u64 >> 32);

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    mailbox_inst.TXMIR = tempmir;
}

hal::BxCanFrame can_receive(void * p_inst, const hal::CanFifoIndex fifo_idx){
    const auto & mailbox = SDK_INST(p_inst)->sFIFOMailBox[std::bit_cast<uint8_t>(fifo_idx)];
    const uint32_t rxmir = mailbox.RXMIR;
    const uint32_t rxmdtr = mailbox.RXMDTR;

    //获取载荷长度
    const uint8_t dlc_bits = static_cast<uint8_t>(rxmdtr & (0x0F << 0u));

    //将低四位的和高四位拼接为完整的8x8的载荷
    const uint64_t payload_u64 = 
        static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDLR)) 
        | (static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDHR)) << 32);

    return hal::BxCanFrame::from_sxx32_regs(rxmir, payload_u64, dlc_bits);
}


}