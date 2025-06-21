#include "can.hpp"
#include "can_filter.hpp"

#include "hal/nvic/nvic.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/debug/debug.hpp"

//#region switches

// #define SCE_ENABLED

//#endregion switches

using namespace ymd;
using namespace ymd::hal;

using Callback = Can::Callback;

static constexpr auto Mailbox_Index_To_TSTATR(auto x) {
    return CAN_TSTATR_RQCP0 << (x << 3);
}


template<uint32_t IT>
ITStatus MY_CAN_GetITStatus(CAN_TypeDef* CANx){
    if((CANx->INTENR & IT) != RESET){
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
        CANx->TSTATR = CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2;  
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
void Can::init_it(){
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

    CAN_ClearITPendingBit(instance_, it_mask);
    CAN_ITConfig(instance_, it_mask, ENABLE);

    switch(uint32_t(instance_)){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            //tx interrupt
            NvicRequest{{1, 6}, USB_HP_CAN1_TX_IRQn}.enable();
            //rx0 interrupt
            NvicRequest{{1, 4}, USB_LP_CAN1_RX0_IRQn}.enable();
            //rx1 interrupt
            NvicRequest{{1, 5}, CAN1_RX1_IRQn}.enable();
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



bool Can::is_mail_box_done(const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    return ((instance_->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
}

void Can::clear_mailbox(const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    instance_->TSTATR = TSTATR_FLAG;
}


Gpio & Can::get_tx_gpio(){
    switch((uint32_t)instance_){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            return CAN1_TX_GPIO;
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_TX_GPIO;
        #endif


    }
}

Gpio & Can::get_rx_gpio(){
    switch((uint32_t)instance_){
        default:
            __builtin_unreachable();
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            return CAN1_RX_GPIO;
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_RX_GPIO;
        #endif
    }
}

void Can::install_gpio(){
    get_tx_gpio().afpp();
    get_rx_gpio().afpp();
}


void Can::enable_rcc(){
    switch((uint32_t)instance_){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
            uint8_t remap = CAN1_REMAP;
            switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
                    break;
                case 1:
                case 2:
                    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);//for TEST
                    GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);
                    break;
            }
        }
        break;
        #endif


        #ifdef ENABLE_CAN2
        case CAN2_BASE:{
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
            uint8_t remap = CAN2_REMAP;
                switch(remap){
                case 0:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, DISABLE);
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
                    break;
            }
        }
        break;
        #endif
    }
}

void Can::init(const Config & cfg){
    enable_rcc();
    install_gpio();

    const auto setting = cfg.baudrate.dump();

    const CAN_InitTypeDef CAN_InitConf = {
        .CAN_Prescaler = setting.prescale,
        .CAN_Mode = std::bit_cast<uint8_t>(cfg.mode),
        .CAN_SJW = std::bit_cast<uint8_t>(setting.swj),
        .CAN_BS1 = std::bit_cast<uint8_t>(setting.bs1),
        .CAN_BS2 = std::bit_cast<uint8_t>(setting.bs2),

        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = DISABLE,
        .CAN_NART = ENABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = DISABLE,
    };

    // DEBUG_PRINTLN(
    //     std::bit_cast<uint8_t>(cfg.mode),
    //     setting.prescale,
    //     std::bit_cast<uint8_t>(setting.swj),
    //     std::bit_cast<uint8_t>(setting.bs1),
    //     std::bit_cast<uint8_t>(setting.bs2)
    // );



    CAN_Init(instance_, &CAN_InitConf);
    init_it();
}

size_t Can::pending(){
    const uint32_t tempreg = instance_->TSTATR;
    if((tempreg & CAN_TSTATR_TME0)) return 0;
    else if((tempreg & CAN_TSTATR_TME1)) return 1;
    else if((tempreg & CAN_TSTATR_TME2)) return 2;
    else return 3;
}

std::optional<uint8_t> Can::transmit(const CanMsg & msg){
    const auto transmit_mailbox = [this] -> int{
        const uint32_t tempreg = instance_->TSTATR;
        if((tempreg & CAN_TSTATR_TME0)) return 0;
        else if((tempreg & CAN_TSTATR_TME1)) return 1;
        else if((tempreg & CAN_TSTATR_TME2)) return 2;
        else return -1;
    }();

    if(transmit_mailbox < 0) return std::nullopt;

    const uint32_t tempmir = msg.identifier_as_u32();
    const uint64_t data = msg.payload_as_u64();
    instance_->sTxMailBox[uint32_t(transmit_mailbox)].TXMDLR = data & UINT32_MAX;
    instance_->sTxMailBox[uint32_t(transmit_mailbox)].TXMDHR = data >> 32;

    instance_->sTxMailBox[uint32_t(transmit_mailbox)].TXMDTR = uint32_t(0xFFFF0000 | msg.size());
    instance_->sTxMailBox[uint32_t(transmit_mailbox)].TXMIR = tempmir;

    return transmit_mailbox;
}


void Can::enable_hw_retransmit(const Enable en){
    if(en == EN)  instance_->CTLR &= ~CAN_CTLR_NART;
    else    instance_->CTLR |=  CAN_CTLR_NART;
}

bool Can::write(const CanMsg & msg){
    if(this->sync_){
        const auto begin_ms = clock::millis();
        while(clock::millis() - begin_ms < 10ms){
            if(transmit(msg)) return true;
        }
        return false;
    }else{
        if(pending() < 3){
            return transmit(msg).has_value();
        }else{
            if(tx_fifo_.available() < tx_fifo_.size()){
                tx_fifo_.push(msg);
                return true;
            }
            return false;
        }
    }
}

CanMsg Can::read(){
    return std::move(rx_fifo_.pop());
}


size_t Can::available(){
    return rx_fifo_.available();
}

uint8_t Can::get_rx_errcnt(){
    return instance_->ERRSR >> 24;
}

uint8_t Can::get_tx_errcnt(){
    return instance_->ERRSR >> 16;
}
Can::ErrCode Can::get_last_error(){
    return (ErrCode)CAN_GetLastErrorCode(instance_);
}

bool Can::is_tranmitting(){
    return bool(instance_->STATR & CAN_STATR_TXM);
}

bool Can::is_receiving(){
    return bool(instance_->STATR & CAN_STATR_RXM);
}

bool Can::is_busoff(){
    return instance_->ERRSR & CAN_ERRSR_BOFF;
}

void Can::cancel_transmit(const uint8_t mbox){
    CAN_CancelTransmit(instance_, mbox);
}

void Can::cancel_all_transmits(){
    for(uint8_t i = 0; i < 3; i++)    CAN_CancelTransmit(instance_, i);
    instance_->TSTATR = instance_->TSTATR | (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can::enable_fifo_lock(const Enable en){
    if(en == EN) instance_->CTLR |= CAN_CTLR_RFLM;
    else instance_->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enable_index_priority(const Enable en){
    if(en == EN) instance_->CTLR |= CAN_CTLR_TXFP;
    else instance_->CTLR &= ~CAN_CTLR_TXFP;
}

void Can::set_baudrate(const uint32_t baudrate){
    //TODO
}

CanMsg Can::receive(const uint8_t fifo_num){
    const uint32_t rxmir = instance_->sFIFOMailBox[fifo_num].RXMIR;
    const uint32_t rxmdtr = instance_->sFIFOMailBox[fifo_num].RXMDTR;

    const uint8_t dlc = (uint8_t)0x0F & rxmdtr;

    const uint64_t data = instance_->sFIFOMailBox[fifo_num].RXMDLR | (uint64_t(instance_->sFIFOMailBox[fifo_num].RXMDHR) << 32);

    if(fifo_num == CAN_FIFO0){
        instance_->RFIFO0 = CAN_RFIFO0_RFOM0 | instance_->RFIFO0;
    }else{
        instance_->RFIFO1 = CAN_RFIFO1_RFOM1 | instance_->RFIFO1;
    }

    return CanMsg::from_regs(rxmir, data, dlc);
}


CanFilter Can::operator[](const size_t idx) const {
    if(idx > 13) HALT;
    return CanFilter(this->instance_, idx);
}

void Can::on_tx_interrupt(){

    for(uint8_t mbox = 0; mbox < 3; mbox++){
        if(not is_mail_box_done(mbox)) continue; // if existing message done

        const uint8_t tx_status = CAN_TransmitStatus(instance_, mbox);

        switch (tx_status){
            case(CAN_TxStatus_Failed):
                //process failed message
                EXECUTE(cb_txfail_);
                break;
            case(CAN_TxStatus_Ok):
                //process success message
                EXECUTE(cb_txok_);
                break;
        }

        clear_mailbox(mbox);

        if(tx_fifo_.available()){
            (void)transmit(tx_fifo_.pop());
        }
    
    }
}

void Can::on_rx_msg_interrupt(const uint8_t fifo_num){
    //process rx pending
    //如果没有接收到 直接返回
    if(CAN_MessagePending(instance_, fifo_num) == 0) return;
    
    rx_fifo_.push(receive(fifo_num));
    EXECUTE(cb_rx_);
}

void Can::on_sce_interrupt(){
    #if 1
    // #ifdef SCE_ENABLED
    if (MY_CAN_GetITStatus<CAN_IT_WKU>(instance_)) {
        // Handle Wake-up interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_WKU>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_SLK>(instance_)) {
        // Handle Sleep acknowledge interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_SLK>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_ERR>(instance_)) {
        // Handle Error interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_ERR>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_EWG>(instance_)) {
        // Handle Error warning interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_EWG>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_EPV>(instance_)) {
        // Handle Error passive interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_EPV>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_BOF>(instance_)) {
        // Handle Bus-off interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_BOF>(instance_);
    } else if (MY_CAN_GetITStatus<CAN_IT_LEC>(instance_)) {
        // Handle Last error code interrupt
        MY_CAN_ClearITPendingBit<CAN_IT_LEC>(instance_);
    } else {
        // Handle other interrupts or add more cases as needed
    }
    #endif
}


#ifdef ENABLE_CAN1
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void){
    can1.on_tx_interrupt();
}


#define CAN_RX_HANDLER(inst, uinst)\
if (MY_CAN_GetITStatus<FMP_MASK>(uinst)){\
    inst.on_rx_msg_interrupt(FIFO_NUM);\
    MY_CAN_ClearITPendingBit<FMP_MASK>(uinst);\
}else if(MY_CAN_GetITStatus<FF_MASK>(uinst)){\
    inst.on_rx_full_interrupt();\
    MY_CAN_ClearITPendingBit<FF_MASK>(uinst);\
}else if(MY_CAN_GetITStatus<FOV_MASK>(uinst)){\
    inst.on_rx_overrun_interrupt();\
    MY_CAN_ClearITPendingBit<FOV_MASK>(uinst);\
}\


__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void) {
    static constexpr auto FMP_MASK = CAN_IT_FMP0;
    static constexpr auto FF_MASK = CAN_IT_FF0;
    static constexpr auto FOV_MASK = CAN_IT_FOV0;
    static constexpr auto FIFO_NUM = CAN_FIFO0;
    CAN_RX_HANDLER(can1, CAN1)
}

__interrupt
void CAN1_RX1_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP1;
    static constexpr auto FF_MASK = CAN_IT_FF1;
    static constexpr auto FOV_MASK = CAN_IT_FOV1;
    static constexpr auto FIFO_NUM = CAN_FIFO1;
    CAN_RX_HANDLER(can1, CAN1)
}

#ifdef SCE_ENABLED
__interrupt
void CAN1_SCE_IRQHandler(void){
    can1.onSceInterrupt();
}
#endif
#endif

#ifdef ENABLE_CAN2
__interrupt
void CAN2_TX_IRQHandler(void){
    can2.on_tx_interrupt();
}

__interrupt
void CAN2_RX0_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP0;
    static constexpr auto FF_MASK = CAN_IT_FF0;
    static constexpr auto FOV_MASK = CAN_IT_FOV0;
    static constexpr auto FIFO_NUM = CAN_FIFO0;
    CAN_RX_HANDLER(can2, CAN2, CAN_FIFO0)
}

__interrupt
void CAN2_RX1_IRQHandler(void){
    static constexpr auto FMP_MASK = CAN_IT_FMP1;
    static constexpr auto FF_MASK = CAN_IT_FF1;
    static constexpr auto FOV_MASK = CAN_IT_FOV1;
    static constexpr auto FIFO_NUM = CAN_FIFO1;
    CAN_RX_HANDLER(can2, CAN2, CAN_FIFO1)
}

#ifdef SCE_ENABLED
__interrupt
void CAN2_SCE_IRQHandler(void){
    can2.onSceInterrupt();
}

#endif
#endif

namespace ymd::hal{

#ifdef ENABLE_CAN1
Can can1 = {CAN1};
#endif

#ifdef ENABLE_CAN2
Can can2 = {CAN2};
#endif
}
