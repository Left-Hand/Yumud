#include "can.hpp"
#include "CanFilter.hpp"
#include "hal/nvic/nvic.hpp"

#include "sys/debug/debug_inc.h"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

using Callback = Can::Callback;

#define Mailbox_Index_To_TSTATR(x) (CAN_TSTATR_RQCP0 << (x * 8))

void Can::initIt(){
    uint32_t it_mask = 
        CAN_IT_TME      //tx done
        | CAN_IT_FMP0   //rx fifo0
        | CAN_IT_FMP1   //rx fifo1
        | CAN_IT_ERR 
        | CAN_IT_WKU
        | CAN_IT_SLK 
        | CAN_IT_EWG 
        | CAN_IT_EPV 
        | CAN_IT_BOF
        | CAN_IT_LEC
    ;

    CAN_ClearITPendingBit(instance, it_mask);
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
            NvicRequest{{1, 2}, CAN1_SCE_IRQn}.enable();
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
            NvicRequest{{1, 2}, CAN2_SCE_IRQn}.enable();
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
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            return CAN1_TX_GPIO;
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_TX_GPIO;
        #endif

        default:
            return GpioNull;
    }
}

Gpio & Can::getRxGpio(){
    switch((uint32_t)instance){
        #ifdef ENABLE_CAN1
        case CAN1_BASE:
            return CAN1_RX_GPIO;
        #endif

        #ifdef ENABLE_CAN2
        case CAN2_BASE:
            return CAN2_RX_GPIO;
        #endif

        default:
            return GpioNull;
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

void Can::bindCbTxOk(Callback && _cb){cb_txok = _cb;}
void Can::bindCbTxFail(Callback && _cb){cb_txfail = _cb;}
void Can::bindCbRx(Callback && _cb){cb_rx = _cb;}

void Can::init(const uint baudRate, const Mode _mode){
    BaudRate baud;
    switch(baudRate){
        case 125_KHz:
            baud = BaudRate::_125K;
            break;
        case 250_KHz:
            baud = BaudRate::_250K;
            break;
        case 500_KHz:
            baud = BaudRate::_500K;
            break;

        default:
        case 1_MHz:
            baud = BaudRate::_1M;
            break;
    }

    init(baud, _mode);
}

void Can::init(const BaudRate baudRate, const Mode _mode){
    installGpio();
    enableRcc();

    uint8_t swj, bs1, bs2;
    uint8_t prescale = 0;

    swj = CAN_SJW_2tq;
    bs1 = CAN_BS1_6tq;
    bs2 = CAN_BS2_5tq;

    switch(baudRate){
    case BaudRate::_125K:
        prescale = 96;
        break;
    case BaudRate::_250K:
        prescale = 48;
        break;
    case BaudRate::_500K:
        prescale = 24;
        break;
    case BaudRate::_1M:
        prescale = 12;
        break;
    };

    CAN_InitTypeDef config;
    config.CAN_Prescaler = prescale;
    config.CAN_Mode = (uint8_t)_mode;
    config.CAN_SJW = swj;
    config.CAN_BS1 = bs1;
    config.CAN_BS2 = bs2;

    config.CAN_TTCM = DISABLE;
    config.CAN_ABOM = ENABLE;
    config.CAN_AWUM = DISABLE;
    config.CAN_NART = ENABLE;
    config.CAN_RFLM = DISABLE;
    config.CAN_TXFP = DISABLE;
    CAN_Init(instance, &config);
    initIt();
}

size_t Can::pending(){
    if((instance->TSTATR & CAN_TSTATR_TME0) == CAN_TSTATR_TME0){
        return 0;
    }else if((instance->TSTATR & CAN_TSTATR_TME1) == CAN_TSTATR_TME1){
        return 1;
    }else if((instance->TSTATR & CAN_TSTATR_TME2) == CAN_TSTATR_TME2){
        return 2;
    }else{
        return 3;
    }
}

void Can::enableHwReTransmit(const bool en){
    if(en)  instance->CTLR &= ~CAN_CTLR_NART;
    else    instance->CTLR |=  CAN_CTLR_NART;
}

bool Can::write(const CanMsg & msg){
    // DEBUGGER_INST.println(msg);
    if(this->sync_){
        uint8_t mbox;
        do{
            mbox = CAN_Transmit(instance, msg.cptx());
        }while(mbox == CAN_TxStatus_NoMailBox);

        return true;
    }else{
        uint8_t mbox = CAN_Transmit(instance, msg.cptx());
        return (mbox != CAN_TxStatus_NoMailBox);
    }
}

CanMsg Can::read(){
    return pending_rx_msgs.pop();
}

const CanMsg & Can::front(){
    return pending_rx_msgs.front();
}

size_t Can::available(){
    return pending_rx_msgs.available();
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

void Can::handleTx(){
    for(uint8_t mbox = 0; mbox < 3; mbox++){
        if(isMailBoxDone(mbox)){ // if existing message done
            uint8_t tx_status = CAN_TransmitStatus(instance, mbox);

            switch (tx_status){
            case(CAN_TxStatus_Failed):
                //process failed message
                EXECUTE(cb_txfail);
                break;
            case(CAN_TxStatus_Ok):
                //process success message
                EXECUTE(cb_txok);
                break;
            }
            clearMailbox(mbox);
        }
    }
}

void Can::handleRx(const uint8_t fifo_num){
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

    if (CAN_GetITStatus(instance, fmp_mask)){
        //process rx pending
        do{
            CanMsg rx_msg;

            //如果没有接收到 直接返回
            if(CAN_MessagePending(instance, fifo_num) == 0) return;

            //从外设读入报文到变量
            CAN_Receive(instance, fifo_num, rx_msg.prx());

            pending_rx_msgs.push(rx_msg);
        }while(false);
        EXECUTE(cb_rx);
        CAN_ClearITPendingBit(instance, fmp_mask);
    }else if(CAN_GetITStatus(instance, ff_mask)){
        //process rx full
        CAN_ClearITPendingBit(instance, ff_mask);
    }else if(CAN_GetITStatus(instance, fov_mask)){
        //process rx overrun
        CAN_ClearITPendingBit(instance, fov_mask);
    }
}

void Can::handleSce(){
    if (CAN_GetITStatus(instance, CAN_IT_WKU)) {
        // Handle Wake-up interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_WKU);
    } else if (CAN_GetITStatus(instance, CAN_IT_SLK)) {
        // Handle Sleep acknowledge interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_SLK);
    } else if (CAN_GetITStatus(instance, CAN_IT_ERR)) {
        // Handle Error interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_ERR);
    } else if (CAN_GetITStatus(instance, CAN_IT_EWG)) {
        // Handle Error warning interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_EWG);
    } else if (CAN_GetITStatus(instance, CAN_IT_EPV)) {
        // Handle Error passive interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_EPV);
    } else if (CAN_GetITStatus(instance, CAN_IT_BOF)) {
        // Handle Bus-off interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_BOF);
    } else if (CAN_GetITStatus(instance, CAN_IT_LEC)) {
        // Handle Last error code interrupt
        CAN_ClearITPendingBit(instance, CAN_IT_LEC);
    } else {
        // Handle other interrupts or add more cases as needed
    }
}



// CanFilter & Can::operator[](const size_t idx){
//     return CanFilter{this->instance, 0};
// }

#ifdef ENABLE_CAN1
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void){
    can1.handleTx();
}

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void) {
    can1.handleRx(CAN_FIFO0);
}

__interrupt
void CAN1_RX1_IRQHandler(void){
    can1.handleRx(CAN_FIFO1);
}

__interrupt
void CAN1_SCE_IRQHandler(void){
    can1.handleSce();
}
#endif

#ifdef ENABLE_CAN2
__interrupt
void CAN2_TX_IRQHandler(void){
    can2.handleTx();
}

__interrupt
void CAN2_RX0_IRQHandler(void){
    can2.handleRx(CAN_FIFO0);
}

__interrupt
void CAN2_RX1_IRQHandler(void){
    can2.handleRx(CAN_FIFO1);
}

__interrupt
void CAN2_SCE_IRQHandler(void){
    can2.handleSce();
}
#endif