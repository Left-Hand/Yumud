#include "can.hpp"

#include "can_layout.hpp"

#include "hal/sysmisc/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/intrinsics/volatile.hpp"
#include "core/sdk.hpp"
#include "ral/can.hpp"

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, CAN_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::CAN_Def)>(x))

using namespace ymd;

static void set_or_reset_reg_bits(bool cond, volatile uint32_t & reg, uint32_t mask){
    if(cond){
        reg = reg | mask;
    }else{
        reg = reg & (~mask);
    }
}

[[maybe_unused]] static void ch32v20xd6_can_bugfix();

#if defined (CH32H417)
[[maybe_unused]] static void ch32h417_can_bugfix();
#endif

namespace ymd::lld{

struct FinitGuard{
    FinitGuard(){
        ral::CAN_Filt->FCTLR.FINIT = 1;
    };

    ~FinitGuard(){
        ral::CAN_Filt->FCTLR.FINIT = 0;
    }
};



void can_configure_filter(
    const size_t filter_nth, 
    const hal::CanFifoIndex fifo_idx,
    const hal::CanFilterConfig & filter_cfg
){
    auto * p_inst = ral::CAN_Filt;
    
    const uint32_t bitmask = 1u << filter_nth;
    auto guard = FinitGuard();

    set_or_reset_reg_bits(false, p_inst->FWR.BITS, bitmask);

    uint32_t FR1;
    uint32_t FR2;

    if (filter_cfg.is_32bit()) {
        FR1 = (static_cast<uint32_t>(filter_cfg.id16[1]) << 16) |
            static_cast<uint32_t>(filter_cfg.id16[0]);
        
        FR2 = (static_cast<uint32_t>(filter_cfg.mask16[1]) << 16) |
            static_cast<uint32_t>(filter_cfg.mask16[0]);
    } else {
        FR1 = (static_cast<uint32_t>(filter_cfg.mask16[0]) << 16) |
            static_cast<uint32_t>(filter_cfg.id16[0]);
        
        FR2 = (static_cast<uint32_t>(filter_cfg.mask16[1]) << 16) |
            static_cast<uint32_t>(filter_cfg.id16[1]);
    }

    p_inst->FILTER_PAIR[filter_nth].FR1.BITS = FR1;
    p_inst->FILTER_PAIR[filter_nth].FR2.BITS = FR2;

    set_or_reset_reg_bits(
        filter_cfg.is_32bit(),
        p_inst->FSCFGR.BITS,
        bitmask
    );

    #if defined (CH32V20x_D6)
    ch32v20xd6_can_bugfix();
    #endif

    
    set_or_reset_reg_bits(
        filter_cfg.is_list_mode(),
        p_inst->FMCFGR.BITS,
        bitmask
    );

    set_or_reset_reg_bits(
        fifo_idx == hal::CanFifoIndex::_1,
        p_inst->FAFIFOR.BITS,
        bitmask
    );

    set_or_reset_reg_bits(
        // filter_en == EN,
        true,
        p_inst->FWR.BITS,
        bitmask
    );
}

void can_set_filter_origin(
    const size_t inst_nth, 
    [[maybe_unused]] const size_t origin
){
    auto guard = FinitGuard();
    auto temp_reg = intrinsics::load_volatile(&ral::CAN_Filt->FCTLR);
    
    switch(inst_nth){
        #ifdef CAN1_PRESENT
        case 1:
            return;
        #endif
        #ifdef CAN2_PRESENT
        case 2:
            temp_reg.CAN2SB = origin;
            break;
        #endif
        #ifdef CAN3_PRESENT
        case 3:
            temp_reg.CAN3SB = origin;
            break;
        #endif
    }

    intrinsics::store_volatile(&ral::CAN_Filt->FCTLR, temp_reg);
    return;
}

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
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, en == EN);
            return;
        }
        #endif

        #ifdef CAN2_PRESENT
        case 2:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, en == EN);
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


void can_transmit_nott(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::BxCanFrame & frame
){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    auto & mailbox_inst = SPL_INST(p_inst)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_txmir_with_txrq();
    const auto [low32, high32] = frame.payload().to_u32x2();


    mailbox_inst.TXMDTR = [&]{
        uint32_t temp_txmdtr = 0;

        #if 0
        temp_txmdtr |= static_cast<uint32_t>(frame.dlc().to_bits()) & 0xf;
        #else
        //dlc 为4位以上被视为ub 不需要进行掩码操作
        temp_txmdtr |= static_cast<uint32_t>(frame.dlc().to_bits());
        #endif

        return temp_txmdtr;
    }();

    //将低四字节装载到txmdlr
    mailbox_inst.TXMDLR = low32;

    //将高四字节装载到txmdhr
    mailbox_inst.TXMDHR = high32;

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    mailbox_inst.TXMIR = tempmir;
}

void can_transmit_ttcan(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::BxCanFrame & frame,
    uint16_t tick
){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    auto & mailbox_inst = SPL_INST(p_inst)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_txmir_with_txrq();
    const auto [low32, high32] = frame.payload().to_u32x2();
    
    mailbox_inst.TXMDTR = [&]{
        uint32_t temp_txmdtr = 0;
        temp_txmdtr |= static_cast<uint32_t>(tick) << 16;
        temp_txmdtr |= static_cast<uint32_t>(1u) << 8;

        #if 0
        temp_txmdtr |= static_cast<uint32_t>(frame.dlc().to_bits()) & 0xf;
        #else
        //dlc 为4位以上被视为ub 不需要进行掩码操作
        temp_txmdtr |= static_cast<uint32_t>(frame.dlc().to_bits());
        #endif

        return temp_txmdtr;
    }();

    //将低四字节装载到txmdlr
    mailbox_inst.TXMDLR = low32;

    //将高四字节装载到txmdhr
    mailbox_inst.TXMDHR = high32;

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    mailbox_inst.TXMIR = tempmir;
}

hal::BxCanFrame can_receive(void * p_inst, const hal::CanFifoIndex fifo_idx){
    const auto & mailbox = SPL_INST(p_inst)->sFIFOMailBox[std::bit_cast<uint8_t>(fifo_idx)];
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





uint8_t my_barecan_init(void * _CANx, const void * _CAN_InitStruct)
{
    CAN_TypeDef* CANx = reinterpret_cast<CAN_TypeDef*>(_CANx);
    const CAN_InitTypeDef * CAN_InitStruct = reinterpret_cast<const CAN_InitTypeDef *>(_CAN_InitStruct);
    static constexpr size_t INAK_TIMEOUT = 0x0000FFFF;
	uint8_t InitStatus = CAN_InitStatus_Failed;
	uint32_t wait_ack = 0x00000000;

    #ifdef CH32H417
    ch32h417_can_bugfix();
    #endif

	CANx->CTLR &= (~(uint32_t)CAN_CTLR_SLEEP);
	CANx->CTLR |= CAN_CTLR_INRQ ;

	while (((CANx->STATR & CAN_STATR_INAK) != CAN_STATR_INAK) && (wait_ack != INAK_TIMEOUT))
	{
		wait_ack++;
	}

	if ((CANx->STATR & CAN_STATR_INAK) != CAN_STATR_INAK)
	{
		InitStatus = CAN_InitStatus_Failed;
	}
	else 
	{
		if (CAN_InitStruct->CAN_TTCM == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_TTCM;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_TTCM;
		}

		if (CAN_InitStruct->CAN_ABOM == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_ABOM;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_ABOM;
		}

		if (CAN_InitStruct->CAN_AWUM == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_AWUM;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_AWUM;
		}

		if (CAN_InitStruct->CAN_NART == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_NART;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_NART;
		}

		if (CAN_InitStruct->CAN_RFLM == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_RFLM;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_RFLM;
		}

		if (CAN_InitStruct->CAN_TXFP == ENABLE)
		{
			CANx->CTLR |= CAN_CTLR_TXFP;
		}
		else
		{
			CANx->CTLR &= ~(uint32_t)CAN_CTLR_TXFP;
		}

		CANx->BTIMR = (uint32_t)((uint32_t)CAN_InitStruct->CAN_Mode << 30) | \
								((uint32_t)CAN_InitStruct->CAN_SJW << 24) | \
								((uint32_t)CAN_InitStruct->CAN_BS1 << 16) | \
								((uint32_t)CAN_InitStruct->CAN_BS2 << 20) | \
								((uint32_t)CAN_InitStruct->CAN_Prescaler - 1);
		CANx->CTLR &= ~(uint32_t)CAN_CTLR_INRQ;
		wait_ack = 0;

		while (((CANx->STATR & CAN_STATR_INAK) == CAN_STATR_INAK) && (wait_ack != INAK_TIMEOUT))
		{
			wait_ack++;
		}

		if ((CANx->STATR & CAN_STATR_INAK) == CAN_STATR_INAK)
		{
			InitStatus = CAN_InitStatus_Failed;
		}
		else
		{
			InitStatus = CAN_InitStatus_Success ;
		}
	}

	return InitStatus;
}


}

[[maybe_unused]] static void ch32v20xd6_can_bugfix(){
    //见数据手册 我认为是给usbd和can的共享sram打补丁

    if(((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)
    {
        uint32_t i;

        for(i = 0; i < 64; i++){
            *(volatile uint16_t *)(0x40006000 + 512 + 4 * i) = *(volatile uint16_t *)(0x40006000 + 768 + 4 * i);
        }
    }
}

#ifdef CH32H417
[[maybe_unused]] static void ch32h417_can_bugfix(){
	uint32_t chip = DBGMCU_GetCHIPID();
	if((chip & 0x000000F0) == 0)
	{
		if(CAN1 == CANx)
        {
            (*(__IO uint32_t *)(0x40021014)) |= 0x02000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x02000000);
        }else if(CAN2 == CANx)
        {
            (*(__IO uint32_t *)(0x40021014)) |= 0x04000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x04000000);
        }else if(CAN3 == CANx)
		{
			(*(__IO uint32_t *)(0x40021014)) |= 0x01000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x01000000);
		}
        
        CANx->CTLR &= ~0x2;
        CANx->CTLR |= 0x1;
        
        while(!(CANx->STATR & 0x1) && (wait_ack != 0x0000FFFF))
        {
            wait_ack++;
        }

        if((CANx->STATR & 0x1))
        {
            CANx->BTIMR = ( uint32_t)0xC1100000| \
                                    ((uint32_t)SystemCoreClock/(((((*(__IO uint32_t *)(0x40021004)) >> 8) & 0x7) < 0x4) ? 1 : (uint32_t)0x2<<(((*(__IO uint32_t *)(0x40021004)) >> 8) & 0x3))/4000000 - 1);
        }
        else
        {
            return CAN_InitStatus_Failed;
        }
        CANx->CTLR &= ~0x1;
        wait_ack = 0;
        while((CANx->STATR & 0x1) && (wait_ack != 0x0000FFFF))
        {
            wait_ack++;
        }

        if((CANx->STATR & 0x1)){
            return CAN_InitStatus_Failed;
        }

        (*(__IO uint32_t *)(0x4000660C)) |= 0xFFFFFFF;
		(*(__IO uint32_t *)(0x40006620)) |= 0x3FF;
        (*(__IO uint32_t *)(0x40006640)) = 0x0;
        (*(__IO uint32_t *)(0x40006644)) = 0x0;
        (*(__IO uint32_t *)(0x40006648)) = 0x0;
        (*(__IO uint32_t *)(0x4000664C)) = 0x0;
		(*(__IO uint32_t *)(0x40006650)) = 0x0;
        (*(__IO uint32_t *)(0x40006654)) = 0x0;
        (*(__IO uint32_t *)(0x4000661C)) |= 0xFFFFFFF;	
		(*(__IO uint32_t *)(0x40006620)) |= 0x3FF;	
		(*(__IO uint32_t *)(0x40006600)) = 0x2A010101;
        (*(__IO uint32_t *)(0x40006600)) &= ~0x1; 	
        if(CAN1 == CANx)
        {
            (*(__IO uint32_t *)(0x40006580)) |= 0x3;
            while(!((*(__IO uint32_t *)(0x4000640C)) & 0x3));
            (*(__IO uint32_t *)(0x4000640C)) = 0x38;
        }else if (CAN2 == CANx)
        {
            (*(__IO uint32_t *)(0x40006980)) |= 0x3;
            while(!((*(__IO uint32_t *)(0x4000680C)) & 0x3));
            (*(__IO uint32_t *)(0x4000680C)) = 0x38;
        }else if (CAN3 == CANx)
        {
            (*(__IO uint32_t *)(0x40007980)) |= 0x3;
            while(!((*(__IO uint32_t *)(0x4000780C)) & 0x3));
            (*(__IO uint32_t *)(0x4000780C)) = 0x38;
        }

        if(CAN1 == CANx)
        {
            (*(__IO uint32_t *)(0x40021014)) |= 0x02000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x02000000);
        }else if(CAN2 == CANx)
        {
            (*(__IO uint32_t *)(0x40021014)) |= 0x04000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x04000000);
        }else if(CAN3 == CANx)
		{
			(*(__IO uint32_t *)(0x40021014)) |= 0x01000000;
            (*(__IO uint32_t *)(0x40021014)) &= ~(0x01000000);
		}
        (*(__IO uint32_t *)(0x40006600)) |= 0x1; 	
        (*(__IO uint32_t *)(0x4000660C)) |= 0xFFFFFFF;	
        (*(__IO uint32_t *)(0x4000661C)) |= 0xFFFFFFF;	
        (*(__IO uint32_t *)(0x40006600)) &= ~0x1;
		(*(__IO uint32_t *)(0x40006600)) |= 0x1; 	
		(*(__IO uint32_t *)(0x40006600)) = 0x2A010101;
		(*(__IO uint32_t *)(0x40006600)) &= ~0x1;
        wait_ack = 0;
	}
}
#endif