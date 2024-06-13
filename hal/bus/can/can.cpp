#include "can.hpp"


using Callback = Can::Callback;

static std::unique_ptr<CanMsg> pending_tx_msg_ptrs[3] = {nullptr};
static RingBuf_t<std::shared_ptr<CanMsg>, 8> pending_rx_msgs;
static Callback cb_txok;
static Callback cb_txfail;
static Callback cb_rx;

#ifdef HAVE_CAN1

Can can1{CAN1};

#endif






void CAN_IT_Init(CAN_TypeDef * instance){
    //tx interrupt
    /****************************************/
    CAN_ClearITPendingBit(instance, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1);
    CAN_ITConfig(instance, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);

    NvicRequest{USB_HP_CAN1_TX_IRQn,1,1}.enable();
    //rx0 interrupt

    NvicRequest{USB_LP_CAN1_RX0_IRQn,1,0}.enable();

    //rx1 interrupt
    NvicRequest{CAN1_RX1_IRQn,1,0}.enable();

    CAN_ITConfig(instance, CAN_IT_ERR | CAN_IT_WKU
            | CAN_IT_SLK | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF
            | CAN_IT_LEC | CAN_IT_ERR, ENABLE);

    // CAN_ITConfig(instance, CAN_IT_ERR, DISABLE);
    //sce interrupt
    NvicRequest{CAN1_SCE_IRQn,1,2}.enable();
}

void Save_CAN_Msg(CAN_TypeDef * instance, const uint8_t fifo_index){
    CanMsg rx_msg;

    if(CAN_MessagePending(instance, fifo_index) == 0) return;

    CAN_Receive(instance, fifo_index, &rx_msg);
    pending_rx_msgs.addData(std::make_shared<CanMsg>(rx_msg));
}

void CAN_Init_Filter(uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2){
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

    CAN_FilterInitSturcture.CAN_FilterNumber = 1;

    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

    // CAN_FilterInitSturcture.CAN_FilterIdLow  = id1 << 5;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdLow = mask2 << 5;
    // CAN_FilterInitSturcture.CAN_FilterIdHigh = id2 << 5;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = mask2 << 5;
    CAN_FilterInitSturcture.CAN_FilterIdLow  = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0;

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

    CAN_FilterInit(&CAN_FilterInitSturcture);

    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    CAN_FilterInit(&CAN_FilterInitSturcture);
}

__fast_inline constexpr uint32_t Mailbox_Index_To_TSTATR(const uint8_t mbox){
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


__fast_inline bool CAN_Mailbox_Done(CAN_TypeDef* CANx, const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    return ((CANx->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
}

__fast_inline void CAN_Mailbox_Clear(CAN_TypeDef* CANx, const uint8_t mbox){
    const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
    CANx->TSTATR = TSTATR_FLAG;
}

void Can::settleTxPin(const uint8_t & remap){

    #ifdef HAVE_CAN1
    switch(remap){
        case 0:
        {
            Gpio & gpio = portA[(Pin)CAN1_TX_RM0_Pin];
            gpio.OutAfPP();
            break;
        }
        case 1:
        {
            Gpio & gpio = portB[(Pin)CAN1_TX_RM1_Pin];
            gpio.OutAfPP();
            break;
        }
    }
    #endif
}

void Can::settleRxPin(const uint8_t & remap){
    #ifdef HAVE_CAN1
    switch(remap){
        case 0:
        {
            Gpio & gpio = portA[(Pin)CAN1_RX_RM0_Pin];
            gpio.OutAfPP();
            break;
        }
        case 1:
        {
            Gpio & gpio = portB[(Pin)CAN1_RX_RM1_Pin];
            gpio.OutAfPP();
            break;
        }
    }
    #endif
}

void Can::bindCbTxOk(Callback && _cb){cb_txok = _cb;}
void Can::bindCbTxFail(Callback && _cb){cb_txfail = _cb;}
void Can::bindCbRx(Callback && _cb){cb_rx = _cb;}
void Can::init(const BaudRate baudRate, const CanFilter & filter){
    uint8_t remap = 1;

    settleTxPin(remap);
    settleRxPin(remap);

    switch(remap){
    case 0:
        GPIO_PinRemapConfig(GPIO_Remap1_CAN1, DISABLE);
        break;
    case 1:
        GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
        break;
    case 2:
        GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);
        break;
    }
    uint8_t swj, bs1, bs2;
    uint16_t prescale = 0;

    switch(baudRate){
    case BaudRate::Kbps125:
        swj = CAN_SJW_2tq;
        bs1 = CAN_BS1_6tq;
        bs2 = CAN_BS2_5tq;
        prescale = 96;
        break;
    case BaudRate::Mbps1:
        swj = CAN_SJW_2tq;
        bs1 = CAN_BS1_6tq;
        bs2 = CAN_BS2_5tq;
        prescale = 12;
        break;
    };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    CAN_InitTypeDef config;
    config.CAN_Prescaler = prescale;
    config.CAN_Mode = CAN_Mode_Normal;
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
    CAN_IT_Init(CAN1);
}

size_t Can::pending(){
    size_t cnt = 0;
    for(uint8_t i = 0; i < 3; i++) cnt += bool(pending_tx_msg_ptrs[i]);
    return cnt;
}

void Can::enableHwReTransmit(const bool en){
    // config.CAN_NART = en ? DISABLE : ENABLE;
    if(en)  CAN1->CTLR &= ~CAN_CTLR_NART;
    else    CAN1->CTLR |=  CAN_CTLR_NART;
}

bool Can::write(const CanMsg & msg){
    uint8_t mbox = CAN_Transmit(instance, (const CanTxMsg *)&msg);
    if(mbox == CAN_TxStatus_NoMailBox) return false;

    pending_tx_msg_ptrs[mbox] = std::make_unique<CanMsg>(msg);

    return true;
}

const CanMsg & Can::read(){
    return *(pending_rx_msgs.getData());
}

size_t Can::available(){
    return pending_rx_msgs.available();
}

uint8_t Can::getRxErrCnt(){
    return CAN1->ERRSR >> 24;
}

uint8_t Can::getTxErrCnt(){
    return CAN1->ERRSR >> 16;
}
uint8_t Can::getErrCode(){
    return CAN_GetLastErrorCode(CAN1);
}

bool Can::isTranmitting(){
    return bool(CAN1->STATR & CAN_STATR_TXM);
}

bool Can::isReceiving(){
    return bool(CAN1->STATR & CAN_STATR_RXM);
}
bool Can::isBusOff(){
    return CAN1->ERRSR & CAN_ERRSR_BOFF;
}

void Can::cancelTransmit(const uint8_t mbox){
    CAN_CancelTransmit(instance, mbox);
}

void Can::cancelAllTransmit(){
    CAN1->TSTATR |= (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can::enableFifoLock(const bool en){
    if(en) CAN1->CTLR |= CAN_CTLR_RFLM;
    else CAN1->CTLR &= ~CAN_CTLR_RFLM;
}

void Can::enableIndexPriority(const bool en){
    if(en) CAN1->CTLR |= CAN_CTLR_TXFP;
    else CAN1->CTLR &= ~CAN_CTLR_TXFP;
}



#ifdef HAVE_CAN1
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void){
    for(uint8_t mbox = 0; mbox < 3; mbox++){
        if(pending_tx_msg_ptrs[mbox] && CAN_Mailbox_Done(CAN1, mbox)){ // if existing message done
            uint8_t tx_status = CAN_TransmitStatus(CAN1, mbox);

            switch (tx_status){
            case(CAN_TxStatus_Failed):
                //process failed message
                EXECUTE(cb_txfail);
                pending_tx_msg_ptrs[mbox].reset();
                break;
            case(CAN_TxStatus_Ok):
                //process success message
                EXECUTE(cb_txok);
                pending_tx_msg_ptrs[mbox].reset();
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