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

[[nodiscard]] static constexpr uint32_t set_or_reset_bit(
    const uint32_t origin, 
    const bool cond, 
    const uint32_t mask
){ 
    return cond ? (origin | mask) : (origin & ~mask);
};


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
    
    const uint32_t filter_pos_mask = 1u << filter_nth;
    auto guard = FinitGuard();

    p_inst->FWR.BITS = set_or_reset_bit(p_inst->FWR.BITS, false, filter_pos_mask);

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

    p_inst->FSCFGR.BITS = set_or_reset_bit(
        p_inst->FSCFGR.BITS,
        filter_cfg.is_32bit(),
        filter_pos_mask
    );

    #if defined (CH32V20x_D6)
    ch32v20xd6_can_bugfix();
    #endif

    
    p_inst->FMCFGR.BITS = set_or_reset_bit(
        p_inst->FMCFGR.BITS,
        filter_cfg.is_list_mode(),
        filter_pos_mask
    );

    p_inst->FAFIFOR.BITS = set_or_reset_bit(
        p_inst->FAFIFOR.BITS,
        fifo_idx == hal::CanFifoIndex::_1,
        filter_pos_mask
    );

    p_inst->FWR.BITS = set_or_reset_bit(
        // filter_en == EN,
        p_inst->FWR.BITS,
        true,
        filter_pos_mask
    );
}

void can_set_filter_origin(
    const size_t inst_nth, 
    [[maybe_unused]] const size_t origin
){
    auto guard = FinitGuard();
    auto tempreg_u32 = intrinsics::load_volatile_to_u32(&ral::CAN_Filt->FCTLR);
    
    switch(inst_nth){
        #ifdef CAN1_PRESENT
        case 1:
            return;
        #endif
        #ifdef CAN2_PRESENT
        case 2:
            tempreg_u32.CAN2SB = origin;
            break;
        #endif
        #ifdef CAN3_PRESENT
        case 3:
            tempreg_u32.CAN3SB = origin;
            break;
        #endif
    }

    intrinsics::store_volatile_with_u32(&ral::CAN_Filt->FCTLR, tempreg_u32);
    return;
}



void can_reset(void * p_inst){
    intrinsics::store_volatile_with_u32(&RAL_INST(p_inst)->CTLR, 1u << 15);
}

void can_request_initialization(void * p_inst, const Enable en){
    SPL_INST(p_inst)->CTLR = set_or_reset_bit(
        SPL_INST(p_inst)->CTLR,
        en == EN,
        1u << 0
    );
}

void can_request_sleep(void * p_inst, const Enable en){
    SPL_INST(p_inst)->CTLR = set_or_reset_bit(
        SPL_INST(p_inst)->CTLR,
        en == EN,
        1u << 1
    );
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


void can_deinit(const Nth can_nth){
    switch(can_nth.count()){
        #ifdef CAN1_PRESENT
        case 1:{
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, ENABLE);
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1, DISABLE);
            break;
        }
        #endif

        #ifdef CAN2_PRESENT
        case 2:{
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2, ENABLE);
            RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2, DISABLE);
            break;
        }
        #endif

        #ifdef CAN3_PRESENT 
        #error "can3 not supported yet"
        #endif
    }
}


#ifdef CANFD_PRESENT
static void notify_thisis_bxnotfd(){
	CANx->CANFD_CR &= ~(1);
}
#endif

void can_transmit_nott(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::ClassicCanFrame & frame
){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    #ifdef CANFD_PRESENT
    notify_thisis_bxnotfd();
    #endif

    auto & mailbox = SPL_INST(p_inst)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_txmir_with_txrq();
    const auto [low32, high32] = frame.payload().to_u32x2();


    mailbox.TXMDTR = [&]{
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
    mailbox.TXMDLR = low32;

    //将高四字节装载到txmdhr
    mailbox.TXMDHR = high32;

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    mailbox.TXMIR = tempmir;
}

void can_transmit_ttcan(
    void * p_inst, 
    const hal::CanMailboxIndex mbox_idx, 
    const hal::ClassicCanFrame & frame,
    uint16_t tick
){
    if(size_t(mbox_idx) > 2) __builtin_unreachable();

    #ifdef CANFD_PRESENT
    notify_thisis_bxnotfd();
    #endif

    auto & mailbox = RAL_INST(p_inst)->sTxMailBox[
        static_cast<size_t>(mbox_idx)];

    const uint32_t tempmir = frame.identifier().to_sxx32_txmir_with_txrq();
    const auto [low32, high32] = frame.payload().to_u32x2();
    
    const uint32_t temp_txmdtr = [&]{
        uint32_t temp_txmdtr_ = 0;
        temp_txmdtr_ |= static_cast<uint32_t>(tick) << 16;
        temp_txmdtr_ |= static_cast<uint32_t>(1u) << 8;

        #if 0
        temp_txmdtr_ |= static_cast<uint32_t>(frame.dlc().to_bits()) & 0xf;
        #else
        //dlc 为4位以上被视为ub 不需要进行掩码操作
        temp_txmdtr_ |= static_cast<uint32_t>(frame.dlc().to_bits());
        #endif

        return temp_txmdtr_;
    }();


    intrinsics::store_volatile_with_u32(&mailbox.TXMDTR, temp_txmdtr);

    //将低四字节装载到txmdlr
    mailbox.TXMDLR = low32;

    //将高四字节装载到txmdhr
    mailbox.TXMDHR = high32;

    //有关TXMIR和TXMDTR的描述，请参考芯片数据手册
    //!txmir必须最后填写 因为填写txmir时会导致当前正在填充的报文被发出
    intrinsics::store_volatile_with_u32(&mailbox.TXMIR, tempmir);
}

hal::ClassicCanFrame can_receive(void * p_inst, const hal::CanFifoIndex fifo_idx){
    auto & mailbox = RAL_INST(p_inst)->sFifoMailBox[std::bit_cast<uint8_t>(fifo_idx)];
    const uint32_t rxmir = intrinsics::load_volatile_to_u32(&mailbox.RXMIR);
    const uint32_t rxmdtr = intrinsics::load_volatile_to_u32(&mailbox.RXMDTR);

    //获取载荷长度
    const uint8_t dlc_bits = static_cast<uint8_t>(rxmdtr & (0x0F << 0u));

    //将低四位的和高四位拼接为完整的8x8的载荷
    const uint64_t payload_u64 = 
        static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDLR)) 
        | (static_cast<uint64_t>(static_cast<uint32_t>(mailbox.RXMDHR)) << 32);

    return hal::ClassicCanFrame::from_sxx32_regs(rxmir, payload_u64, dlc_bits);
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


Result<void, void> can_initialze(void * p_inst, const void * _CAN_InitStruct)
{
    CAN_TypeDef* CANx = reinterpret_cast<CAN_TypeDef*>(p_inst);
    const CAN_InitTypeDef * CAN_InitStruct = reinterpret_cast<const CAN_InitTypeDef *>(_CAN_InitStruct);
    static constexpr size_t INAK_TIMEOUT = 0x0000FFFF;

    #ifdef CANFD_PRESENT
	notify_thisis_bxnotfd();
    #endif

    #ifdef CH32H417
    ch32h417_can_bugfix();
    #endif

	CANx->CTLR &= (~(uint32_t)CAN_CTLR_SLEEP);
	CANx->CTLR |= CAN_CTLR_INRQ ;

    auto is_init_mode = [&] -> bool { 
        return (CANx->STATR & CAN_STATR_INAK) == CAN_STATR_INAK;
    };


    for(volatile size_t wait_times = 0;; wait_times++){
        //进入初始化模式
        if(is_init_mode()) break;
        if(wait_times >= INAK_TIMEOUT) return Err();
    }


    {
        uint32_t tempreg_u32 = intrinsics::load_volatile_to_u32(&RAL_INST(p_inst)->CTLR);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_TTCM == ENABLE, CAN_CTLR_TTCM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_ABOM == ENABLE, CAN_CTLR_ABOM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_AWUM == ENABLE, CAN_CTLR_AWUM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_NART == ENABLE, CAN_CTLR_NART);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_RFLM == ENABLE, CAN_CTLR_RFLM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_TXFP == ENABLE, CAN_CTLR_TXFP);
        intrinsics::store_volatile_with_u32(&RAL_INST(p_inst)->CTLR, tempreg_u32);
    }

    {
        uint32_t tempreg_u32 = intrinsics::load_volatile_to_u32(&RAL_INST(p_inst)->BTIMR);
        tempreg_u32 = tempreg_u32 | ((uint32_t)CAN_InitStruct->CAN_Mode << 30);
        tempreg_u32 = tempreg_u32 | ((uint32_t)CAN_InitStruct->CAN_SJW << 24);
        tempreg_u32 = tempreg_u32 | ((uint32_t)CAN_InitStruct->CAN_BS1 << 16);
        tempreg_u32 = tempreg_u32 | ((uint32_t)CAN_InitStruct->CAN_BS2 << 20);
        tempreg_u32 = tempreg_u32 | ((uint32_t)CAN_InitStruct->CAN_Prescaler - 1);
        intrinsics::store_volatile_with_u32(&RAL_INST(p_inst)->BTIMR, tempreg_u32);
    }

    CANx->CTLR &= ~(uint32_t)CAN_CTLR_INRQ;

    for(volatile size_t wait_times = 0;; wait_times++){
        //退出初始化模式
        if(not is_init_mode()) break;
        if(wait_times >= INAK_TIMEOUT) return Err();
    }

	return Ok();
}

}


/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v20x_can.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : This file provides all the CAN firmware functions.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

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




/********************************** (C) COPYRIGHT  *******************************
* File Name          : ch32h417_can.c
* Author             : WCH
* Version            : V1.0.2
* Date               : 2025/10/21
* Description        : This file provides all the CAN firmware functions.
*********************************************************************************
* Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/



#ifdef CH32H417
[[maybe_unused]] static void ch32h417_can_bugfix(){
	uint32_t chip = DBGMCU_GetCHIPID();
	if((chip & 0x000000F0) == 0)
	{
		if(CAN1 == CANx)
        {
            (*(volatile uint32_t *)(0x40021014)) |= 0x02000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x02000000);
        }else if(CAN2 == CANx)
        {
            (*(volatile uint32_t *)(0x40021014)) |= 0x04000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x04000000);
        }else if(CAN3 == CANx)
		{
			(*(volatile uint32_t *)(0x40021014)) |= 0x01000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x01000000);
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
                                    ((uint32_t)SystemCoreClock/(((((*(volatile uint32_t *)(0x40021004)) >> 8) & 0x7) < 0x4) ? 1 : (uint32_t)0x2<<(((*(volatile uint32_t *)(0x40021004)) >> 8) & 0x3))/4000000 - 1);
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

        (*(volatile uint32_t *)(0x4000660C)) |= 0xFFFFFFF;
		(*(volatile uint32_t *)(0x40006620)) |= 0x3FF;
        (*(volatile uint32_t *)(0x40006640)) = 0x0;
        (*(volatile uint32_t *)(0x40006644)) = 0x0;
        (*(volatile uint32_t *)(0x40006648)) = 0x0;
        (*(volatile uint32_t *)(0x4000664C)) = 0x0;
		(*(volatile uint32_t *)(0x40006650)) = 0x0;
        (*(volatile uint32_t *)(0x40006654)) = 0x0;
        (*(volatile uint32_t *)(0x4000661C)) |= 0xFFFFFFF;	
		(*(volatile uint32_t *)(0x40006620)) |= 0x3FF;	
		(*(volatile uint32_t *)(0x40006600)) = 0x2A010101;
        (*(volatile uint32_t *)(0x40006600)) &= ~0x1; 	
        if(CAN1 == CANx)
        {
            (*(volatile uint32_t *)(0x40006580)) |= 0x3;
            while(!((*(volatile uint32_t *)(0x4000640C)) & 0x3));
            (*(volatile uint32_t *)(0x4000640C)) = 0x38;
        }else if (CAN2 == CANx)
        {
            (*(volatile uint32_t *)(0x40006980)) |= 0x3;
            while(!((*(volatile uint32_t *)(0x4000680C)) & 0x3));
            (*(volatile uint32_t *)(0x4000680C)) = 0x38;
        }else if (CAN3 == CANx)
        {
            (*(volatile uint32_t *)(0x40007980)) |= 0x3;
            while(!((*(volatile uint32_t *)(0x4000780C)) & 0x3));
            (*(volatile uint32_t *)(0x4000780C)) = 0x38;
        }

        if(CAN1 == CANx)
        {
            (*(volatile uint32_t *)(0x40021014)) |= 0x02000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x02000000);
        }else if(CAN2 == CANx)
        {
            (*(volatile uint32_t *)(0x40021014)) |= 0x04000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x04000000);
        }else if(CAN3 == CANx)
		{
			(*(volatile uint32_t *)(0x40021014)) |= 0x01000000;
            (*(volatile uint32_t *)(0x40021014)) &= ~(0x01000000);
		}
        (*(volatile uint32_t *)(0x40006600)) |= 0x1; 	
        (*(volatile uint32_t *)(0x4000660C)) |= 0xFFFFFFF;	
        (*(volatile uint32_t *)(0x4000661C)) |= 0xFFFFFFF;	
        (*(volatile uint32_t *)(0x40006600)) &= ~0x1;
		(*(volatile uint32_t *)(0x40006600)) |= 0x1; 	
		(*(volatile uint32_t *)(0x40006600)) = 0x2A010101;
		(*(volatile uint32_t *)(0x40006600)) &= ~0x1;
        wait_ack = 0;
	}
}
#endif


#ifdef FDCAN_PRESENT
/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32l103_can.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/07/08
 * Description        : This file provides all the CAN firmware functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/


uint8_t     CANFD_Init(CAN_TypeDef *CANx, CANFD_InitTypeDef *CANFD_InitStruct);
uint8_t     CANFD_Transmit(CAN_TypeDef *CANx, CanFDTxMsg *TxMessage);
ErrorStatus CANFD_Receive(CAN_TypeDef *CANx, uint8_t FIFONumber, CanFDRxMsg *RxMessage);


/*********************************************************************
 * @fn      CANFD_Init
 *
 * @brief   Initializes the CAN peripheral according to the specified
 *        parameters in the CANFD_InitStruct.
 *
 * @param   CANx - where x can be 1 to select the CAN peripheral.
 *          CANFD_InitStruct - pointer to a CANFD_InitTypeDef structure that
 *        contains the configuration information for the CAN peripheral.
 *
 * @return  InitStatus - CAN InitStatus state.
*             CAN_InitStatus_Failed.
*             CAN_InitStatus_Success.
 */
Result<void, void> CANFD_Init(CAN_TypeDef* CANx, CANFD_InitTypeDef* CANFD_InitStruct)
{
    uint8_t InitStatus = CAN_InitStatus_Failed;
    uint32_t wait_ack = 0x00000000;

    CANx->CANFD_CR |= 1;

    CANx->CTLR &= (~(uint32_t)CAN_CTLR_SLEEP);
    CANx->CTLR |= CAN_CTLR_INRQ ;

	CANx->CTLR &= (~(uint32_t)CAN_CTLR_SLEEP);
	CANx->CTLR |= CAN_CTLR_INRQ ;

    auto is_init_mode = [&] -> bool { 
        return (CANx->STATR & CAN_STATR_INAK) == CAN_STATR_INAK;
    };

    static constexpr size_t INAK_TIMEOUT = 0xffff;    
    for(volatile size_t wait_times = 0;; wait_times++){
        //进入初始化模式
        if(is_init_mode()) break;
        if(wait_times >= INAK_TIMEOUT) return Err();
    }


    {
        uint32_t tempreg_u32 = SPL_INST(_CANx)->CTLR;
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_TTCM == ENABLE, CAN_CTLR_TTCM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_ABOM == ENABLE, CAN_CTLR_ABOM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_AWUM == ENABLE, CAN_CTLR_AWUM);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_NART == ENABLE, CAN_CTLR_NART);
        #if 0
        // tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_RFLM == ENABLE, CAN_CTLR_RFLM);
        #endif
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_TXFP == ENABLE, CAN_CTLR_TXFP);
        SPL_INST(_CANx)->CTLR = tempreg_u32;
    }

    {
        uint32_t tempreg_u32 = SPL_INST(_CANx)->CANFD_CR;
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_BRS_TXM0 == ENABLE, 1 << 1);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_BRS_TXM1 == ENABLE, 1 << 2);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_BRS_TXM2 == ENABLE, 1 << 3);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_ESI_Auto_TXM0 == ENABLE, 1 << 4);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_ESI_Auto_TXM1 == ENABLE, 1 << 5);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_ESI_Auto_TXM2 == ENABLE, 1 << 6);
        tempreg_u32 = set_or_reset_bit(tempreg_u32, CAN_InitStruct->CAN_RES_Error == ENABLE, 1 << 7);
        SPL_INST(_CANx)->CANFD_CR = tempreg_u32;
    }

    CANx->CANFD_BTR &= ~(0x009F1FFF);
    CANx->CANFD_TDCT &= ~(0x00003F3F);

    CANx->CANFD_TDCT = (uint32_t)((uint32_t)CANFD_InitStruct->CANFD_TDC_FILTER << 8) | \
                                    ((uint32_t)CANFD_InitStruct->CANFD_TDC0);

    CANx->CANFD_BTR = (uint32_t)((uint32_t)CANFD_InitStruct->CANFD_TDCE << 23) | \
                                ((uint32_t)CANFD_InitStruct->CANFD_Prescaler-1 << 16) | \
                                ((uint32_t)CANFD_InitStruct->CANFD_BS1 << 8) | \
                                ((uint32_t)CANFD_InitStruct->CANFD_BS2 << 4) | \
                                ((uint32_t)CANFD_InitStruct->CANFD_SJW);

    CANx->CTLR &= ~(uint32_t)CAN_CTLR_INRQ;
    wait_ack = 0;

    for(volatile size_t wait_times = 0;; wait_times++){
        //进入初始化模式
        if(is_init_mode()) break;
        if(wait_times >= INAK_TIMEOUT) return Err();
    }

    return Ok();
}


/*********************************************************************
 * @fn      CANFD_Transmit
 *
 * @brief   Initiates the transmission of a message for CANFD.
 *
 * @param   CANx - where x can be 1 to select the CAN peripheral.
 *          TxMessage - pointer to a structure which contains CAN Id, CAN
 *        DLC and CAN data.
 *
 * @return  transmit_mailbox - The number of the mailbox that is used for
 *        transmission or CAN_TxStatus_NoMailBox if there is no empty mailbox.
 */
uint8_t CANFD_Transmit(CAN_TypeDef* CANx, CanFDTxMsg* TxMessage)
{
    uint8_t transmit_mailbox = 0;

    CANx->CANFD_CR |= (1);

    if ((CANx->TSTATR&CAN_TSTATR_TME0) == CAN_TSTATR_TME0)
    {
        transmit_mailbox = 0;
    }
    else if ((CANx->TSTATR&CAN_TSTATR_TME1) == CAN_TSTATR_TME1)
    {
        transmit_mailbox = 1;
    }
    else if ((CANx->TSTATR&CAN_TSTATR_TME2) == CAN_TSTATR_TME2)
    {
        transmit_mailbox = 2;
    }
    else
    {
        transmit_mailbox = CAN_TxStatus_NoMailBox;
    }

    if (transmit_mailbox != CAN_TxStatus_NoMailBox)
    {
        CANx->sTxMailBox[transmit_mailbox].TXMIR &= TMIDxR_TXRQ;
        if (TxMessage->IDE == CAN_Id_Standard)
        {
            CANx->sTxMailBox[transmit_mailbox].TXMIR |= ((TxMessage->StdId << 21) | \
                                                        TxMessage->RTR);
        }
        else
        {
            CANx->sTxMailBox[transmit_mailbox].TXMIR |= ((TxMessage->ExtId << 3) | \
                                                        TxMessage->IDE | \
                                                        TxMessage->RTR);
        }

        TxMessage->DLC &= (uint8_t)0x0000000F;
        CANx->sTxMailBox[transmit_mailbox].TXMDTR &= (uint32_t)0xFFFFFFF0;
        CANx->sTxMailBox[transmit_mailbox].TXMDTR |= TxMessage->DLC;
        CANx->CANFD_DMA_T[transmit_mailbox] = (uint32_t)TxMessage->Data;
        CANx->sTxMailBox[transmit_mailbox].TXMIR |= TMIDxR_TXRQ;
    }

    return transmit_mailbox;
}

/*********************************************************************
 * @fn      CANFD_Receive
 *
 * @brief   Receives a message.
 *
 * @param   CANx - where x can be 1 to select the CAN peripheral.
 *          FIFONumber - Receive FIFO number.
 *            CAN_FIFO0.
 *            CAN_FIFO1.
 *          RxMessage -  pointer to a structure receive message which contains
 *        CAN Id, CAN DLC, CAN datas and FMI number.
 *
 * @return  ErrorStatus - NoREADY or READY.
 */
ErrorStatus CANFD_Receive(CAN_TypeDef* CANx, uint8_t FIFONumber, CanFDRxMsg* RxMessage)
{
    ErrorStatus sta = NoREADY;
    uint8_t len, i;
    uint32_t adr;

    if((CANx->sFIFOMailBox[FIFONumber].RXMIR & 1) == 0) return sta;

    RxMessage->IDE = (uint8_t)0x04 & CANx->sFIFOMailBox[FIFONumber].RXMIR;

    if (RxMessage->IDE == CAN_Id_Standard)
    {
        RxMessage->StdId = (uint32_t)0x000007FF & (CANx->sFIFOMailBox[FIFONumber].RXMIR >> 21);
    }
    else
    {
        RxMessage->ExtId = (uint32_t)0x1FFFFFFF & (CANx->sFIFOMailBox[FIFONumber].RXMIR >> 3);
    }

    RxMessage->RTR = (uint8_t)0x02 & CANx->sFIFOMailBox[FIFONumber].RXMIR;

    len = (uint8_t)0x0F & CANx->sFIFOMailBox[FIFONumber].RXMDTR;

    if(len <= 8)
    {
        RxMessage->DLC = len;
    }
    else if(len <= 12)
    {
        RxMessage->DLC = (len - 6) * 4;
    }
    else if(len <= 15)
    {
        RxMessage->DLC = (len - 11) * 16;
    }

    RxMessage->FMI = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RXMDTR >> 8);
    RxMessage->BRS = (uint8_t)0x01 & (CANx->sFIFOMailBox[FIFONumber].RXMDTR >> 4);
    RxMessage->ESI = (uint8_t)0x01 & (CANx->sFIFOMailBox[FIFONumber].RXMDTR >> 5);
    RxMessage->RES = (uint8_t)0x01 & (CANx->sFIFOMailBox[FIFONumber].RXMDTR >> 6);

    adr = CANx->CANFD_DMA_R[FIFONumber] + 0x20000000;

    for(i=0; i<RxMessage->DLC; i++)
    {
        RxMessage->Data[i] = *((uint8_t*)adr++);
    }

    if (FIFONumber == CAN_FIFO0)
    {
        CANx->RFIFO0 |= CAN_RFIFO0_RFOM0;
    }
    else
    {
        CANx->RFIFO1 |= CAN_RFIFO1_RFOM1;
    }

    sta = READY;

    return sta;
}


#endif

#ifdef YMD_CANFD_SPECIFIC_CH32L103
[[maybe_unused]] static void fdcan_set_receive_addr(
    void * p_inst, 
    hal::CanFifoIndex fifo_idx,
    uintptr_t addr
){
    if(addr & 0b11) __builtin_trap();
    auto & reg_u32 = RAL_INST(p_inst)->CANFD_DMA_R[static_cast<uint8_t>(fifo_idx)];
    reg_u32 = static_cast<uint32_t>(addr);
}

[[maybe_unused]] static void fdcan_set_transmit_addr(
    void * p_inst, 
    hal::CanMailboxIndex mbox_idx,
    uintptr_t addr
){
    if(addr & 0b11) __builtin_trap();
    auto & reg_u32 = RAL_INST(p_inst)->CANFD_DMA_T[static_cast<uint8_t>(mbox_idx)];
    reg_u32 = static_cast<uint32_t>(addr);
}

static constexpr uint32_t FDCAN_DMA_BASE_ADDR = 0x2000'0000;
[[maybe_unused]] static uintptr_t fdcan_get_receive_addr(
    void * p_inst, 
    hal::CanFifoIndex fifo_idx
){
    auto & reg_u32 = RAL_INST(p_inst)->CANFD_DMA_R[static_cast<uint8_t>(fifo_idx)];
    return static_cast<uintptr_t>(reg_u32 + FDCAN_DMA_BASE_ADDR);
}

[[maybe_unused]] static uintptr_t fdcan_get_transmit_addr(
    void * p_inst, 
    hal::CanFifoIndex fifo_idx 
){
    auto & reg_u32 = RAL_INST(p_inst)->CANFD_DMA_T[static_cast<uint8_t>(fifo_idx)];
    return static_cast<uintptr_t>(reg_u32 + FDCAN_DMA_BASE_ADDR);
}

[[maybe_unused]] static uint8_t fdcan_get_delay_offset(
    void * p_inst
){
    return static_cast<uint8_t>(RAL_INST(p_inst)->CANFD_PSR.TDCV);
}

[[maybe_unused]] static void fdcan_enable_restrict_mode(
    void * p_inst,
    const Enable en
){
    RAL_INST(p_inst)->CANFD_CR.RESTRICT_MODE = (en == EN);

}
[[maybe_unused]] static void fdcan_set_buserr_cnt(
    void * p_inst,
    const uint8_t cnt
){
    RAL_INST(p_inst)->TERR_CNT.TX_ERR_CNT = cnt;
}

template<typename T>
static void store_volatile_reg(volatile T * p_reg, const uint32_t x){
    *reinterpret_cast<volatile uint32_t *>(p_reg) = x;
}

[[maybe_unused]] static void fdcan_set_bs1_tq(
    void * p_inst,
    const bool is_6bit,
    const uint8_t bs1_bits
){
    // volatile uint32_t & reg = *reinterpret_cast<volatile uint32_t *>(&RAL_INST(p_inst)->BTIMR);
    auto tempreg_u32 = intrinsics::load_volatile(&RAL_INST(p_inst)->BTIMR);
    uint32_t tempreg_u32 = std::bit_cast<uint32_t>(tempreg_u32) & (~0xff00);
    if(is_6bit){
        RAL_INST(p_inst)->CANFD_CR.CLAS_LONG_TS1 = 1;
        tempreg_u32 |= static_cast<uint32_t>(bs1_bits << 16);
    }else{
        RAL_INST(p_inst)->CANFD_CR.CLAS_LONG_TS1 = 0;
        tempreg_u32 |= static_cast<uint32_t>(bs1_bits << 16);
    }
    store_volatile_reg(&RAL_INST(p_inst)->BTIMR, tempreg_u32);
}
#endif