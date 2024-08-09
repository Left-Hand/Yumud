#include "can.hpp"
#include "../hal/nvic/nvic.hpp"

using Callback = Can::Callback;

static bool pending_tx_msg_exist[3] = {};
static RingBuf_t<CanMsg, 8> pending_rx_msgs;
static Callback cb_txok;
static Callback cb_txfail;
static Callback cb_rx;

#ifdef HAVE_CAN1
Can can1{CAN1};
#endif


static void CAN_IT_Init(CAN_TypeDef * instance){
    //tx interrupt
    /****************************************/
    CAN_ClearITPendingBit(instance, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1);
    CAN_ITConfig(instance, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);

    NvicRequest{1, 6, USB_HP_CAN1_TX_IRQn}.enable();
    //rx0 interrupt

    NvicRequest{1, 4, USB_LP_CAN1_RX0_IRQn}.enable();

    //rx1 interrupt
    NvicRequest{1, 5, CAN1_RX1_IRQn}.enable();

    CAN_ITConfig(instance, CAN_IT_ERR | CAN_IT_WKU
            | CAN_IT_SLK | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF
            | CAN_IT_LEC | CAN_IT_ERR, ENABLE);

    // CAN_ITConfig(instance, CAN_IT_ERR, DISABLE);
    //sce interrupt
    NvicRequest{1,2, CAN1_SCE_IRQn}.enable();
}

[[maybe_unused]] static void Save_CAN_Msg(CAN_TypeDef * instance, const uint8_t fifo_index){
    CanMsg rx_msg;

    if(CAN_MessagePending(instance, fifo_index) == 0) return;

    CAN_Receive(instance, fifo_index, &rx_msg);
    pending_rx_msgs.addData(rx_msg);
}

__fast_inline constexpr static uint32_t Mailbox_Index_To_TSTATR(const uint8_t mbox){
    switch(mbox){
    case 0:
        return CAN_TSTATR_RQCP0;
    case 1:
        return CAN_TSTATR_RQCP1;
    case 2:
        return CAN_TSTATR_RQCP2;
    default:
        static_assert(true, "Invalid value for switch!");
        return 0;
    }
}


__fast_inline static bool CAN_Mailbox_Done(const CAN_TypeDef* CANx, const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    return ((CANx->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
}

__fast_inline static void CAN_Mailbox_Clear(CAN_TypeDef* CANx, const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    CANx->TSTATR = TSTATR_FLAG;
}


Gpio & Can::getTxGpio(){
    #ifdef HAVE_CAN1
    return CAN1_TX_GPIO;
    #endif
    return GpioNull;
}

Gpio & Can::getRxGpio(){
    #ifdef HAVE_CAN1
    return CAN1_RX_GPIO;
    #endif
    return GpioNull;
}

void Can::installGpio(){
    auto & txGpio = getTxGpio();
    auto & rxGpio = getRxGpio();

    txGpio.afpp();
    rxGpio.afpp();

}
void Can::enableRcc(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    #ifdef HAVE_CAN1
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
    #endif
}

void Can::bindCbTxOk(Callback && _cb){cb_txok = _cb;}
void Can::bindCbTxFail(Callback && _cb){cb_txfail = _cb;}
void Can::bindCbRx(Callback && _cb){cb_rx = _cb;}

void Can::init(const uint baudRate, const Mode _mode, const CanFilter & filter){
    BaudRate baud;
    switch(baudRate){
        case 125_KHz:
            baud = BaudRate::Kbps125;
            break;
        case 250_KHz:
            baud = BaudRate::Kbps250;
            break;
        case 500_KHz:
            baud = BaudRate::Kbps500;
            break;
        case 1_MHz:
            baud = BaudRate::Mbps1;
            break;
        default:
            baud = BaudRate::Kbps250;
    }
    init(baud, _mode, filter);
}


void Can::init(const BaudRate baudRate, const Mode _mode, const CanFilter & filter){
    installGpio();
    enableRcc();

    uint8_t swj, bs1, bs2;
    uint8_t prescale = 0;

    swj = CAN_SJW_2tq;
    bs1 = CAN_BS1_6tq;
    bs2 = CAN_BS2_5tq;

    switch(baudRate){
    case BaudRate::Kbps125:
        prescale = 96;
        break;
    case BaudRate::Kbps250:
        prescale = 48;
        break;
    case BaudRate::Kbps500:
        prescale = 24;
        break;
    case BaudRate::Mbps1:
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

    CanFilter::init(filter);
    CAN_IT_Init(instance);
}

size_t Can::pending(){
    size_t cnt = 0;
    for(uint8_t i = 0; i < 3; i++) cnt += bool(pending_tx_msg_exist[i]);
    return cnt;
}

void Can::enableHwReTransmit(const bool en){
    if(en)  instance->CTLR &= ~CAN_CTLR_NART;
    else    instance->CTLR |=  CAN_CTLR_NART;
}

bool Can::write(const CanMsg & msg){

    uint8_t mbox = CAN_Transmit(instance, (const CanTxMsg *)&msg);
    if(mbox == CAN_TxStatus_NoMailBox) return false;

    pending_tx_msg_exist[mbox] = true;
    return true;
}

const CanMsg & Can::read(){
    return pending_rx_msgs.getData();
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

void Can::configBaudRate(const uint32_t baudRate){
    //TODO
    // static_assert(false,"configBaudRate is not supported currently");
}

#ifdef HAVE_CAN1
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void){
    for(uint8_t mbox = 0; mbox < 3; mbox++){
        if(pending_tx_msg_exist[mbox] && CAN_Mailbox_Done(CAN1, mbox)){ // if existing message done
            uint8_t tx_status = CAN_TransmitStatus(CAN1, mbox);

            switch (tx_status){
            case(CAN_TxStatus_Failed):
                //process failed message
                EXECUTE(cb_txfail);
                pending_tx_msg_exist[mbox] = false;
                break;
            case(CAN_TxStatus_Ok):
                //process success message
                EXECUTE(cb_txok);
                pending_tx_msg_exist[mbox] = false;
                break;
            }
            CAN_Mailbox_Clear(CAN1, mbox);
        }
    }
}

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void) {
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0)){
        //process rx pending
        Save_CAN_Msg(CAN1, CAN_FIFO0);
        EXECUTE(cb_rx);
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }else if(CAN_GetITStatus(CAN1, CAN_IT_FF0)){
        //process rx full
        CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);
    }else if(CAN_GetITStatus(CAN1, CAN_IT_FOV0)){
        //process rx overrun
        CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);
    }
}

__interrupt
void CAN1_RX1_IRQHandler(void){
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP1)){
        //process rx pending
        Save_CAN_Msg(CAN1, CAN_FIFO1);
        EXECUTE(cb_rx);
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
    }else if(CAN_GetITStatus(CAN1, CAN_IT_FF1)){
        //process rx full
        CAN_ClearITPendingBit(CAN1, CAN_IT_FF1);
    }else if(CAN_GetITStatus(CAN1, CAN_IT_FOV1)){
        //process rx overrun
        CAN_ClearITPendingBit(CAN1, CAN_IT_FOV1);
    }
}

__interrupt
void CAN1_SCE_IRQHandler(void){
    if (CAN_GetITStatus(CAN1, CAN_IT_WKU)) {
        // Handle Wake-up interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_SLK)) {
        // Handle Sleep acknowledge interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_ERR)) {
        // Handle Error interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_EWG)) {
        // Handle Error warning interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_EPV)) {
        // Handle Error passive interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_BOF)) {
        // Handle Bus-off interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);
    } else if (CAN_GetITStatus(CAN1, CAN_IT_LEC)) {
        // Handle Last error code interrupt
        CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);
    } else {
        // Handle other interrupts or add more cases as needed
    }
}
#endif