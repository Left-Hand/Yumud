#include "can.hpp"
#include "can_filter.hpp"

#include "hal/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;

using Callback = Can::Callback;


static constexpr auto Mailbox_Index_To_TSTATR(CanMailboxNth x) {
    return CAN_TSTATR_RQCP0 << (std::bit_cast<uint8_t>(x) << 3);
}


template<uint32_t IT>
ITStatus MY_CAN_GetITStatus(const uint32_t reg, CAN_TypeDef* CANx){
    if((reg & IT) != RESET){
        if constexpr(IT == CAN_IT_TME){
            return(CANx->TSTATR & (CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2));  
        }else if constexpr(IT == CAN_IT_FMP0){
            return (CANx->RFIFO0 & CAN_RFIFO0_FMP0);  
        }else if constexpr(IT == CAN_IT_FF0){
            return (CANx->RFIFO0 & CAN_RFIFO0_FULL0);  
        }else if constexpr(IT == CAN_IT_FOV0){
            return (CANx->RFIFO0 & CAN_RFIFO0_FOVR0);  
        }else if constexpr(IT == CAN_IT_FMP1){
            return (CANx->RFIFO1 & CAN_RFIFO1_FMP1);  
        }else if constexpr(IT == CAN_IT_FF1){
            return (CANx->RFIFO1 & CAN_RFIFO1_FULL1);  
        }else if constexpr(IT == CAN_IT_FOV1){
            return (CANx->RFIFO1 & CAN_RFIFO1_FOVR1);  
        }else if constexpr (IT == CAN_IT_WKU)
            return (CANx->STATR& CAN_STATR_WKUI);  
        else if constexpr (IT == CAN_IT_SLK)
            return (CANx->STATR& CAN_STATR_SLAKI);  
        else if constexpr (IT == CAN_IT_EWG)
            return (CANx->ERRSR& CAN_ERRSR_EWGF);  
        else if constexpr (IT == CAN_IT_EPV)
            return (CANx->ERRSR& CAN_ERRSR_EPVF); 
        else if constexpr (IT == CAN_IT_BOF)
            return (CANx->ERRSR& CAN_ERRSR_BOFF);  
        else if constexpr (IT == CAN_IT_LEC)
            return (CANx->ERRSR& CAN_ERRSR_LEC);  
        else if constexpr (IT == CAN_IT_ERR)
            return (CANx->STATR& CAN_STATR_ERRI); 
    }
    return RESET;
}


template<uint32_t IT>
void MY_CAN_ClearITPendingBit(CAN_TypeDef* CANx)
{
    if constexpr(IT == CAN_IT_TME){
        CANx->TSTATR = CAN_TSTATR_RQCP0 | CAN_TSTATR_RQCP1 | CAN_TSTATR_RQCP2;  
    }else if constexpr(IT == CAN_IT_FF0){
        CANx->RFIFO0 = CAN_RFIFO0_FULL0;
    }else if constexpr(IT == CAN_IT_FOV0){
        CANx->RFIFO0 = CAN_RFIFO0_FOVR0;
    }else if constexpr(IT == CAN_IT_FF1){
        CANx->RFIFO1 = CAN_RFIFO1_FULL1;
    }else if constexpr(IT == CAN_IT_FOV1){
        CANx->RFIFO1 = CAN_RFIFO1_FOVR1;
    }else if constexpr(IT == CAN_IT_WKU){
        CANx->STATR = CAN_STATR_WKUI;
    }else if constexpr(IT == CAN_IT_SLK){
        CANx->STATR = CAN_STATR_SLAKI;
    }else if constexpr(IT == CAN_IT_EWG){
        CANx->STATR = CAN_STATR_ERRI;
    }else if constexpr(IT == CAN_IT_EPV){
        CANx->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_BOF){
        CANx->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_LEC){
        CANx->ERRSR = RESET; 
        CANx->STATR = CAN_STATR_ERRI; 
    }else if constexpr (IT == CAN_IT_ERR){
        CANx->ERRSR = RESET; 
        CANx->STATR = CAN_STATR_ERRI; 
	}
}


void Can::init_interrupts(){
    const uint32_t it_mask = 
        CAN_IT_TME      //tx done
        | CAN_IT_FMP0   //rx fifo0
        | CAN_IT_FMP1   //rx fifo1

        #ifdef SCE_ENABLED
        | CAN_IT_ERR 
        | CAN_IT_WKU
        | CAN_IT_SLK 
        | CAN_IT_EWG 
        | CAN_IT_EPV 
        | CAN_IT_BOF
        | CAN_IT_LEC
        #endif
    ;

    CAN_ClearITPendingBit(inst_, it_mask);
    CAN_ITConfig(inst_, it_mask, ENABLE);

    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            //tx interrupt
            NvicPriority{1, 7}.with_irqn(USB_HP_CAN1_TX_IRQn).enable(EN);
            //rx0 interrupt
            NvicPriority{1, 5}.with_irqn(USB_LP_CAN1_RX0_IRQn).enable(EN);
            //rx1 interrupt
            NvicPriority{1, 6}.with_irqn(CAN1_RX1_IRQn).enable(EN);
            //sce interrupt

            #ifdef SCE_ENABLED
            NvicRequest{{1, 2}, CAN1_SCE_IRQn}.enable();
            #endif
            break;
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            //tx interrupt
            NvicRequest{{1, 6}, CAN2_TX_IRQn}.enable();
            //rx0 interrupt
            NvicRequest{{1, 4}, CAN2_RX0_IRQn}.enable();
            //rx1 interrupt
            NvicRequest{{1, 5}, CAN2_RX1_IRQn}.enable();
            //sce interrupt
            #ifdef SCE_ENABLED
            NvicRequest{{1, 2}, CAN2_SCE_IRQn}.enable();
            #endif
            break;
        #endif
        
        default:
            break;
    }
}



bool Can::is_mail_box_done(const CanMailboxNth mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    return ((inst_->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
}


void Can::clear_mailbox(const CanMailboxNth mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    inst_->TSTATR = TSTATR_FLAG;
}


Gpio map_inst_to_tx_gpio(const void * inst, const uint8_t remap){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            switch(remap){
                case 0:
                    return CAN1_RM0_TX_GPIO;
                case 1:
                    if(sys::chip::get_flash_size() < 64_KB){
                        return CAN1_RM0_TX_GPIO;
                    }
                    return CAN1_RM1_TX_GPIO;
                case 3:
                    return CAN1_RM3_TX_GPIO;
                default:
                    __builtin_unreachable();
            }
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_TX_GPIO;
        #endif
    }
}


Gpio map_inst_to_rx_gpio(const void * inst, const uint8_t remap){
    switch(reinterpret_cast<size_t>(inst)){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            // return CAN1_RX_GPIO;
            switch(remap){
                case 0:
                    return CAN1_RM0_RX_GPIO;
                case 1:
                    if(sys::chip::get_flash_size() < 64_KB){
                        return CAN1_RM0_RX_GPIO;
                    }
                    return CAN1_RM1_RX_GPIO;
                case 3:
                    return CAN1_RM3_RX_GPIO;
                default:
                    __builtin_unreachable();
            }
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_RX_GPIO;
        #endif
    }
}


Gpio Can::get_tx_gpio(const uint8_t remap){
    return map_inst_to_tx_gpio(inst_, remap);
}


Gpio Can::get_rx_gpio(const uint8_t remap){
    return map_inst_to_rx_gpio(inst_, remap);
}


void Can::plant_gpio(const uint8_t remap){
    get_tx_gpio(remap).afpp();
    get_rx_gpio(remap).afpp();
}


void Can::enable_rcc(const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
            return;
        }
        #endif


        #ifdef ENABLE_CAN2
        case CAN2_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
            return;
        }
        #endif
    }

    __builtin_trap();
}

void Can::set_remap(const uint8_t remap){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:{
            switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
                    return;
                case 1:
                case 2:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
                    return;
                case 3:
                    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);//for TEST
                    GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);
                    return;
            }
        }
        break;
        #endif


        #ifdef ENABLE_CAN2
        case CAN2_BASE:{
            switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, DISABLE);
                    return;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
                    return;
            }
        }
        break;
        #endif
    }
    __builtin_trap();
}


void Can::init(const Config & cfg){
    enable_rcc(EN);
    set_remap(cfg.remap);
    plant_gpio(cfg.remap);


    const CAN_InitTypeDef CAN_InitConf = {
        .CAN_Prescaler = cfg.timming_coeffs.prescale,
        .CAN_Mode = std::bit_cast<uint8_t>(cfg.mode),
        .CAN_SJW = std::bit_cast<uint8_t>(cfg.timming_coeffs.swj),
        .CAN_BS1 = std::bit_cast<uint8_t>(cfg.timming_coeffs.bs1),
        .CAN_BS2 = std::bit_cast<uint8_t>(cfg.timming_coeffs.bs2),

        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = DISABLE,
        .CAN_NART = ENABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = DISABLE,
    };

    CAN_Init(inst_, &CAN_InitConf);
    init_interrupts();
}

size_t Can::pending(){
    const uint32_t tempreg = inst_->TSTATR;
    if((tempreg & CAN_TSTATR_TME0)) return 0;
    else if((tempreg & CAN_TSTATR_TME1)) return 1;
    else if((tempreg & CAN_TSTATR_TME2)) return 2;
    else return 3;
}

Option<CanMailboxNth> Can::transmit(const CanMsg & msg){
    const auto transmit_mailbox = [this] -> int32_t{
        const uint32_t tempreg = inst_->TSTATR;
        if((tempreg & CAN_TSTATR_TME0)) return 0;
        else if((tempreg & CAN_TSTATR_TME1)) return 1;
        else if((tempreg & CAN_TSTATR_TME2)) return 2;
        else return -1;
    }();

    if(transmit_mailbox < 0) return None;

    const uint32_t tempmir = msg.sxx32_identifier_as_u32();
    const uint64_t data = msg.payload_as_u64();
    auto & mailbox_setting = inst_->sTxMailBox[
        static_cast<size_t>(transmit_mailbox)];

    mailbox_setting.TXMDLR = data & UINT32_MAX;
    mailbox_setting.TXMDHR = data >> 32;

    mailbox_setting.TXMDTR = uint32_t(0xFFFF0000 | msg.size());
    mailbox_setting.TXMIR = tempmir;

    return Some(
        std::bit_cast<CanMailboxNth>(uint8_t(transmit_mailbox))
    );
}


void Can::enable_hw_retransmit(const Enable en){
    if(en == EN)  inst_->CTLR &= ~CAN_CTLR_NART;
    else    inst_->CTLR |=  CAN_CTLR_NART;
}

Result<void, CanError> Can::write(const CanMsg & msg){
    auto push_buf = [this, &msg]() -> Result<void, CanError>{ 
        if(tx_fifo_.writable_capacity() > 0){
            tx_fifo_.push(msg);
            return Ok();
        }
        return Err(CanError::SoftFifoOverflow);
    };

    if(pending() >= 3)
        return push_buf();
    
    if(transmit(msg).is_none())
        return push_buf();

    return Ok();
}

CanMsg Can::read(){
    return std::move(rx_fifo_.pop());
}

size_t Can::available(){
    return rx_fifo_.available();
}

uint8_t Can::get_rx_errcnt(){
    return inst_->ERRSR >> 24;
}

uint8_t Can::get_tx_errcnt(){
    return inst_->ERRSR >> 16;
}

Option<Can::Fault> Can::last_fault(){
    const auto code = CAN_GetLastErrorCode(inst_);
    if(code == 0) return None;
    return Some(std::bit_cast<Can::Fault>(code));
}

bool Can::is_tranmitting(){
    return bool(inst_->STATR & CAN_STATR_TXM);
}

bool Can::is_receiving(){
    return bool(inst_->STATR & CAN_STATR_RXM);
}

bool Can::is_busoff(){
    return inst_->ERRSR & CAN_ERRSR_BOFF;
}

void Can::cancel_transmit(const CanMailboxNth mbox){
    CAN_CancelTransmit(inst_, std::bit_cast<uint8_t>(mbox));
}

void Can::cancel_all_transmits(){
    for(uint8_t i = 0; i < 3; i++) 
        CAN_CancelTransmit(inst_, i);
    inst_->TSTATR = inst_->TSTATR | 
        (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can::enable_fifo_lock(const Enable en){
    if(en == EN) inst_->CTLR |= CAN_CTLR_RFLM;
    else inst_->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enable_index_priority(const Enable en){
    if(en == EN) inst_->CTLR |= CAN_CTLR_TXFP;
    else inst_->CTLR &= ~CAN_CTLR_TXFP;
}

void Can::set_baudrate(const uint32_t baudrate){
    //TODO
}

CanMsg Can::receive(const CanFifoNth fifo_num){
    const size_t index = std::bit_cast<uint8_t>(fifo_num);
    auto & mailbox = inst_->sFIFOMailBox[index];
    const uint32_t rxmir = mailbox.RXMIR;
    const uint32_t rxmdtr = mailbox.RXMDTR;

    const uint8_t dlc = rxmdtr & (0x0F);

    const uint64_t data = 
        static_cast<uint64_t>(mailbox.RXMDLR) 
        | (static_cast<uint64_t>(mailbox.RXMDHR) << 32);

    switch(fifo_num){
        case CanFifoNth::_0:
            inst_->RFIFO0 = CAN_RFIFO0_RFOM0 | inst_->RFIFO0;
            break;
        case CanFifoNth::_1:
            inst_->RFIFO1 = CAN_RFIFO1_RFOM1 | inst_->RFIFO1;
            break;
    }

    return CanMsg::from_sxx32_regs(rxmir, data, dlc);
}




void Can::accept_tx_interrupt(){

    //遍历每个邮箱
    for(uint8_t mbox = 0; mbox < 3; mbox++){
        //如果还没完成 那么略过
        if(not is_mail_box_done(std::bit_cast<CanMailboxNth>(mbox))) continue; 

        const auto tx_status = CAN_TransmitStatus(inst_, mbox);


        switch (tx_status){
            case(CAN_TxStatus_Failed):
                //process failed message
                if(callback_ != nullptr)
                    callback_(CanEvent(CanTransmitEvent::Failed));
                break;
            case(CAN_TxStatus_Ok):
                if(callback_ != nullptr)
                    callback_(CanEvent(CanTransmitEvent::Success));
                break;
            default:
                __builtin_unreachable();
        }

        clear_mailbox(std::bit_cast<CanMailboxNth>(mbox));
    }
    //poll next
    if(tx_fifo_.available()){
        (void)transmit(tx_fifo_.pop());
    }
}

void Can::accept_rx_msg_interrupt(const CanFifoNth fifo_num){
    //process rx pending
    //如果没有接收到 直接返回
    if(CAN_MessagePending(inst_, std::bit_cast<uint8_t>(fifo_num)) == 0) return;
    
    rx_fifo_.push(receive(fifo_num));
}

void Can::accept_sce_interrupt(){
    #if 1
    const auto reg = inst_->INTENR;
    // #ifdef SCE_ENABLED
    if (MY_CAN_GetITStatus<CAN_IT_WKU>(reg, inst_)) {
        // Handle Wake-up interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_WKU>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_SLK>(reg, inst_)) {
        // Handle Sleep acknowledge interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_SLK>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_ERR>(reg, inst_)) {
        // Handle Error interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_ERR>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_EWG>(reg, inst_)) {
        // Handle Error warning interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_EWG>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_EPV>(reg, inst_)) {
        // Handle Error passive interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_EPV>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_BOF>(reg, inst_)) {
        // Handle Bus-off interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_BOF>(inst_);
    } else if (MY_CAN_GetITStatus<CAN_IT_LEC>(reg, inst_)) {
        // Handle Last error code interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_LEC>(inst_);
    } else {
        // Handle other interrupts or add more cases as needed
    }
    #endif
}

void Can::accept_rx_full_interrupt(const CanFifoNth fifo_num){

}

void Can::accept_rx_overrun_interrupt(const CanFifoNth fifo_num){

}


#ifdef ENABLE_CAN1
void USB_HP_CAN1_TX_IRQHandler(void){
    can1.accept_tx_interrupt();
}


#define CAN_RX_HANDLER(inst, uinst)\
if (MY_CAN_GetITStatus<FMP_MASK>(reg, uinst)){\
    inst.accept_rx_msg_interrupt(FIFO_NUM);\
    MY_CAN_ClearITPendingBit<FMP_MASK>(uinst);\
}else if(MY_CAN_GetITStatus<FF_MASK>(reg, uinst)){\
    inst.accept_rx_full_interrupt(FIFO_NUM);\
    MY_CAN_ClearITPendingBit<FF_MASK>(uinst);\
}else if(MY_CAN_GetITStatus<FOV_MASK>(reg, uinst)){\
    inst.accept_rx_overrun_interrupt(FIFO_NUM);\
    MY_CAN_ClearITPendingBit<FOV_MASK>(uinst);\
}\

void USB_LP_CAN1_RX0_IRQHandler(void) {
    static constexpr auto FMP_MASK = CAN_IT_FMP0;
    static constexpr auto FF_MASK = CAN_IT_FF0;
    static constexpr auto FOV_MASK = CAN_IT_FOV0;
    static constexpr auto FIFO_NUM = CanFifoNth::_0;
    const auto reg = CAN1->INTENR;
    CAN_RX_HANDLER(can1, CAN1)
}

void CAN1_RX1_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP1;
    static constexpr auto FF_MASK = CAN_IT_FF1;
    static constexpr auto FOV_MASK = CAN_IT_FOV1;
    static constexpr auto FIFO_NUM = CanFifoNth::_1;
    const auto reg = CAN1->INTENR;
    CAN_RX_HANDLER(can1, CAN1)
}

#ifdef SCE_ENABLED
void CAN1_SCE_IRQHandler(void){
    can1.onSceInterrupt();
}
#endif
#endif

#ifdef ENABLE_CAN2
void CAN2_TX_IRQHandler(void){
    can2.on_tx_interrupt();
}

void CAN2_RX0_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP0;
    static constexpr auto FF_MASK = CAN_IT_FF0;
    static constexpr auto FOV_MASK = CAN_IT_FOV0;
    static constexpr auto FIFO_NUM = CAN_FIFO0;
    CAN_RX_HANDLER(can2, CAN2, CAN_FIFO0)
}

void CAN2_RX1_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP1;
    static constexpr auto FF_MASK = CAN_IT_FF1;
    static constexpr auto FOV_MASK = CAN_IT_FOV1;
    static constexpr auto FIFO_NUM = CAN_FIFO1;
    CAN_RX_HANDLER(can2, CAN2, CAN_FIFO1)
}

#ifdef SCE_ENABLED
void CAN2_SCE_IRQHandler(void){
    can2.onSceInterrupt();
}

#endif
#endif

namespace ymd::hal{

#ifdef ENABLE_CAN1
Can can1 = Can{CAN1};
#endif

#ifdef ENABLE_CAN2
Can can2 = Can{CAN2};
#endif
}
