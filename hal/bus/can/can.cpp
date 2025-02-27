#include "can.hpp"
#include "CanFilter.hpp"
#include "hal/nvic/nvic.hpp"

//#region switches

// #define SCE_ENABLED

//#endregion switches

using namespace ymd;
using namespace ymd::hal;

using Callback = Can::Callback;

#define Mailbox_Index_To_TSTATR(x) (CAN_TSTATR_RQCP0 << (x << 3))

ITStatus MY_CAN_GetITStatus(CAN_TypeDef* CANx, uint32_t CAN_IT){
  
    if((CANx->INTENR & CAN_IT) != RESET){
        switch (CAN_IT){
        case CAN_IT_TME:
            return(CANx->TSTATR & (CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2));  
            break;
            
        case CAN_IT_FMP0:
            return (CANx->RFIFO0 & CAN_RFIFO0_FMP0);  
            
        case CAN_IT_FF0:
            return (CANx->RFIFO0 & CAN_RFIFO0_FULL0);  
            
        case CAN_IT_FOV0:
            return (CANx->RFIFO0 & CAN_RFIFO0_FOVR0);  
            
        case CAN_IT_FMP1:
            return (CANx->RFIFO0 & CAN_RFIFO1_FMP1);  
            
        case CAN_IT_FF1:
            return (CANx->RFIFO0 & CAN_RFIFO1_FULL1);  
            
        case CAN_IT_FOV1:
            return (CANx->RFIFO0 & CAN_RFIFO1_FOVR1);  
            
        case CAN_IT_WKU:
            return (CANx->STATR& CAN_STATR_WKUI);  
            
        case CAN_IT_SLK:
            return (CANx->STATR& CAN_STATR_SLAKI);  
            
        case CAN_IT_EWG:
            return (CANx->ERRSR& CAN_ERRSR_EWGF);  
            
        case CAN_IT_EPV:
            return (CANx->ERRSR& CAN_ERRSR_EPVF);  
            
        case CAN_IT_BOF:
            return (CANx->ERRSR& CAN_ERRSR_BOFF);  
            
        case CAN_IT_LEC:
            return (CANx->ERRSR& CAN_ERRSR_LEC);  
            
        case CAN_IT_ERR:
            return (CANx->STATR& CAN_STATR_ERRI); 
        }
    }
    return RESET;
}


void MY_CAN_ClearITPendingBit(CAN_TypeDef* CANx, uint32_t CAN_IT)
{
	switch (CAN_IT)
	{
		case CAN_IT_TME:
			CANx->TSTATR = CAN_TSTATR_RQCP0|CAN_TSTATR_RQCP1|CAN_TSTATR_RQCP2;  
			break;

		case CAN_IT_FF0:
			CANx->RFIFO0 = CAN_RFIFO0_FULL0; 
			break;

		case CAN_IT_FOV0:
			CANx->RFIFO0 = CAN_RFIFO0_FOVR0; 
			break;

		case CAN_IT_FF1:
			CANx->RFIFO1 = CAN_RFIFO1_FULL1;  
			break;

		case CAN_IT_FOV1:
			CANx->RFIFO1 = CAN_RFIFO1_FOVR1; 
			break;

		case CAN_IT_WKU:
			CANx->STATR = CAN_STATR_WKUI;  
			break;

		case CAN_IT_SLK:
			CANx->STATR = CAN_STATR_SLAKI;   
			break;

		case CAN_IT_EWG:
			CANx->STATR = CAN_STATR_ERRI;
			break;

		case CAN_IT_EPV:
			CANx->STATR = CAN_STATR_ERRI; 
			break;

		case CAN_IT_BOF:
			CANx->STATR = CAN_STATR_ERRI; 
			break;

		case CAN_IT_LEC:
			CANx->ERRSR = RESET; 
			CANx->STATR = CAN_STATR_ERRI; 
			break;

		case CAN_IT_ERR:
			CANx->ERRSR = RESET; 
			CANx->STATR = CAN_STATR_ERRI; 
			break;

		default :
			break;
	}
}
void Can::initIt(){
    uint32_t it_mask = 
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

    MY_CAN_ClearITPendingBit(instance, it_mask);
    CAN_ITConfig(instance, it_mask, ENABLE);

    switch(uint32_t(instance)){
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



bool Can::isMailBoxDone(const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    return ((instance->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
}

void Can::clearMailbox(const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    instance->TSTATR = TSTATR_FLAG;
}


Gpio & Can::getTxGpio(){
    switch((uint32_t)instance){
        default:
            HALT;
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

Gpio & Can::getRxGpio(){
    switch((uint32_t)instance){
        default:
            HALT
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

void Can::installGpio(){
    auto & txGpio = getTxGpio();
    auto & rxGpio = getRxGpio();

    txGpio.afpp();
    rxGpio.afpp();

}
void Can::enableRcc(){
    switch((uint32_t)instance){
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

void Can::init(const uint baudRate, const Mode _mode){
    BaudRate baud;
    switch(baudRate){
        default:
            HALT
        case 125_KHz:
            baud = BaudRate::_125K;
            break;
        case 250_KHz:
            baud = BaudRate::_250K;
            break;
        case 500_KHz:
            baud = BaudRate::_500K;
            break;
        case 1_MHz:
            baud = BaudRate::_1M;
            break;
    }

    init(baud, _mode);
}

void Can::init(const BaudRate baudRate, const Mode _mode){
    installGpio();
    enableRcc();


    scexpr uint8_t swj = CAN_SJW_2tq;
    scexpr uint8_t bs1 = CAN_BS1_6tq;
    scexpr uint8_t bs2 = CAN_BS2_5tq;

    const uint8_t prescale = [baudRate]()->uint8_t{
        switch(baudRate){
            default:
            case BaudRate::_125K:
                return 96;
            case BaudRate::_250K:
                return 48;
            case BaudRate::_500K:
                return 24;
            case BaudRate::_1M:
                return 12;
        };
    }();

    const CAN_InitTypeDef config = {
        .CAN_Prescaler = prescale,
        .CAN_Mode = (uint8_t)_mode,
        .CAN_SJW = swj,
        .CAN_BS1 = bs1,
        .CAN_BS2 = bs2,

        .CAN_TTCM = DISABLE,
        .CAN_ABOM = ENABLE,
        .CAN_AWUM = DISABLE,
        .CAN_NART = ENABLE,
        .CAN_RFLM = DISABLE,
        .CAN_TXFP = DISABLE,
    };

    CAN_Init(instance, &config);
    initIt();
}

size_t Can::pending(){
    uint32_t tempreg = instance->TSTATR;
    if((tempreg & CAN_TSTATR_TME0) == CAN_TSTATR_TME0){
        return 0;
    }else if((tempreg & CAN_TSTATR_TME1) == CAN_TSTATR_TME1){
        return 1;
    }else if((tempreg & CAN_TSTATR_TME2) == CAN_TSTATR_TME2){
        return 2;
    }else{
        return 3;
    }
}

uint8_t Can::transmit(const CanMsg & msg){
    uint8_t transmit_mailbox = 0;

    {
        uint32_t tempreg = instance->TSTATR;

        // scexpr auto mask = CAN_TSTATR_TME0 | CAN_TSTATR_TME1 | CAN_TSTATR_TME2;

        if((tempreg & CAN_TSTATR_TME0)){
            transmit_mailbox = 0;
        }else if((tempreg & CAN_TSTATR_TME1)){
            transmit_mailbox = 1;
        }else if((tempreg & CAN_TSTATR_TME2)){
            transmit_mailbox = 2;
        }else{
            return CAN_TxStatus_NoMailBox;
        }


    }

    if(transmit_mailbox != CAN_TxStatus_NoMailBox){

        uint32_t tempmir;
        uint32_t tempdtr = instance->sTxMailBox[transmit_mailbox].TXMDTR;

        if(msg.isStd()){
            tempmir = ((msg.id() << 21) | (msg.isRemote() << 1) | 0x01);
        }else{
            tempmir = ((msg.id() << 3)  | (1 << 2) | (msg.isRemote() << 1) | 0x01);
        }

        tempdtr &= (uint32_t)0xFFFFFFF0;
        tempdtr |= msg.size() & 0x0F;

        if(msg.size() && (!msg.isRemote())){

            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wuninitialized"

            uint64_t data = msg.data64();
            instance->sTxMailBox[transmit_mailbox].TXMDLR = data & UINT32_MAX;
            instance->sTxMailBox[transmit_mailbox].TXMDHR = data >> 32;

            #pragma GCC diagnostic pop
        }

        instance->sTxMailBox[transmit_mailbox].TXMDTR = tempdtr;
        instance->sTxMailBox[transmit_mailbox].TXMIR = tempmir;
    }

    return transmit_mailbox;
}


void Can::enableHwReTransmit(const bool en){
    if(en)  instance->CTLR &= ~CAN_CTLR_NART;
    else    instance->CTLR |=  CAN_CTLR_NART;
}

bool Can::write(const CanMsg & msg){
    if(this->sync_){
        uint8_t mbox;
        do{
            mbox = transmit(msg);
        }while(mbox == CAN_TxStatus_NoMailBox);

        return true;
    }else{
        if(pending() < 3){
            uint8_t mbox = transmit(msg);
            return (mbox != CAN_TxStatus_NoMailBox);
        }else{
            // if(tx_fifo_.available() < tx_fifo_.size()){
                tx_fifo_.push(msg);
                return true;
            // }
            // return false;
        }
    }
}

const CanMsg && Can::read(){
    return std::move(rx_fifo_.pop());
}

const CanMsg & Can::front(){
    return rx_fifo_.front();
}

size_t Can::available(){
    return rx_fifo_.available();
}

uint8_t Can::getRxErrCnt(){
    return instance->ERRSR >> 24;
}

uint8_t Can::getTxErrCnt(){
    return instance->ERRSR >> 16;
}
Can::ErrCode Can::getErrCode(){
    return (ErrCode)CAN_GetLastErrorCode(instance);
}

bool Can::isTranmitting(){
    return bool(instance->STATR & CAN_STATR_TXM);
}

bool Can::isReceiving(){
    return bool(instance->STATR & CAN_STATR_RXM);
}

bool Can::isBusOff(){
    return instance->ERRSR & CAN_ERRSR_BOFF;
}

void Can::cancelTransmit(const uint8_t mbox){
    CAN_CancelTransmit(instance, mbox);
}

void Can::cancelAllTransmit(){
    for(uint8_t i = 0; i < 3; i++)    CAN_CancelTransmit(instance, i);
    instance->TSTATR |= (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can::enableFifoLock(const bool en){
    if(en) instance->CTLR |= CAN_CTLR_RFLM;
    else instance->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enableIndexPriority(const bool en){
    if(en) instance->CTLR |= CAN_CTLR_TXFP;
    else instance->CTLR &= ~CAN_CTLR_TXFP;
}

void Can::setBaudRate(const uint32_t baudRate){
    //TODO
}

CanMsg Can::receive(const uint8_t fifo_num){
    uint32_t rxmir = instance->sFIFOMailBox[fifo_num].RXMIR;
    uint32_t rxmdtr = instance->sFIFOMailBox[fifo_num].RXMDTR;

    bool ext = (uint8_t)0x04 & rxmir;
    uint32_t id = ext ? ((rxmir >> 3)) : (((1 << 11) - 1) & (rxmir >> 21));
    // uint32_t id = (rxmir >> 21);
    bool remote = (uint8_t)0x02 & rxmir;
    uint8_t dlc = (uint8_t)0x0F & rxmdtr;
    // uint8_t fmi = (uint8_t)0xFF & (rxmdtr >> 8);

    uint64_t data = instance->sFIFOMailBox[fifo_num].RXMDLR | (uint64_t(instance->sFIFOMailBox[fifo_num].RXMDHR) << 32);

    if(fifo_num == CAN_FIFO0){
        instance->RFIFO0 = CAN_RFIFO0_RFOM0 | instance->RFIFO0;
    }else{
        instance->RFIFO1 = CAN_RFIFO1_RFOM1 | instance->RFIFO1;
    }

    if(remote){
        return CanMsg(id, true);
    }else{
        return CanMsg(id, data, dlc);
    }
}


CanFilter Can::operator[](const size_t idx) const {
    if(idx > 13) HALT;
    return CanFilter(this->instance, idx);
}

void Can::onTxInterrupt(){
    for(uint8_t mbox = 0; mbox < 3; mbox++){
        if(isMailBoxDone(mbox)){ // if existing message done
            uint8_t tx_status = CAN_TransmitStatus(instance, mbox);

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

            clearMailbox(mbox);

            if(tx_fifo_.available()){
                transmit(tx_fifo_.pop());
            }
        }
    }
}

void Can::onRxInterrupt(const uint8_t fifo_num){
    uint32_t fmp_mask;
    uint32_t ff_mask;
    uint32_t fov_mask;

    switch(fifo_num){
        default:
        case CAN_FIFO0:
            fmp_mask = CAN_IT_FMP0;
            ff_mask = CAN_IT_FF0;
            fov_mask = CAN_IT_FOV0;
            break;
        case CAN_FIFO1:
            fmp_mask = CAN_IT_FMP1;
            ff_mask = CAN_IT_FF1;
            fov_mask = CAN_IT_FOV1;
            break;
    }

    if (MY_CAN_GetITStatus(instance, fmp_mask)){
        //process rx pending
        do{
            // CanMsg rx_msg;

            //如果没有接收到 直接返回
            if(CAN_MessagePending(instance, fifo_num) == 0) return;

            //从外设读入报文到变量
            // CAN_Receive(instance, fifo_num, rx_msg.prx());
            
            rx_fifo_.push(receive(fifo_num));
        }while(false);
        EXECUTE(cb_rx_);
        MY_CAN_ClearITPendingBit(instance, fmp_mask);
    }else if(MY_CAN_GetITStatus(instance, ff_mask)){
        //process rx full
        MY_CAN_ClearITPendingBit(instance, ff_mask);
    }else if(MY_CAN_GetITStatus(instance, fov_mask)){
        //process rx overrun
        MY_CAN_ClearITPendingBit(instance, fov_mask);
    }
}

void Can::onSceInterrupt(){
    if (MY_CAN_GetITStatus(instance, CAN_IT_WKU)) {
        // Handle Wake-up interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_WKU);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_SLK)) {
        // Handle Sleep acknowledge interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_SLK);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_ERR)) {
        // Handle Error interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_ERR);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_EWG)) {
        // Handle Error warning interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_EWG);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_EPV)) {
        // Handle Error passive interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_EPV);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_BOF)) {
        // Handle Bus-off interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_BOF);
    } else if (MY_CAN_GetITStatus(instance, CAN_IT_LEC)) {
        // Handle Last error code interrupt
        MY_CAN_ClearITPendingBit(instance, CAN_IT_LEC);
    } else {
        // Handle other interrupts or add more cases as needed
    }
}


#ifdef ENABLE_CAN1
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void){
    can1.onTxInterrupt();
}

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void) {
    can1.onRxInterrupt(CAN_FIFO0);
}

__interrupt
void CAN1_RX1_IRQHandler(void){
    can1.onRxInterrupt(CAN_FIFO1);
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
    can2.onTxInterrupt();
}

__interrupt
void CAN2_RX0_IRQHandler(void){
    can2.onRxInterrupt(CAN_FIFO0);
}

__interrupt
void CAN2_RX1_IRQHandler(void){
    can2.onRxInterrupt(CAN_FIFO1);
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