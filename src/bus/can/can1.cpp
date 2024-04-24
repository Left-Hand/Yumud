#include "can1.hpp"


using Callback = std::function<void(void)>;
// #define EXECUTE(x) if(x != nullptr) x();
#define EXECUTE(x)
static std::unique_ptr<CanMsg> pending_tx_msg_ptrs[3] = {nullptr};
static RingBuf_t<std::shared_ptr<CanMsg>, 8> pending_rx_msgs;
static CAN_InitTypeDef config;
static Callback cb_txok;
static Callback cb_txfail;
static Callback cb_rx;


void Can1::settleTxPin(const uint8_t & remap){
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
}

void Can1::settleRxPin(const uint8_t & remap){
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
}

void Can1::bindCbTxOk(const Callback & _cb){cb_txok = _cb;}
void Can1::bindCbTxFail(const Callback & _cb){cb_txfail = _cb;}
void Can1::bindCbRx(const Callback & _cb){cb_rx = _cb;}

void CAN1_IT_Init(){


    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    //tx interrupt
    /****************************************/
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1);
    CAN_ITConfig(CAN1, CAN_IT_TME | CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    //rx0 interrupt

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    //rx1 interrupt

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    CAN_ITConfig(CAN1, CAN_IT_ERR | CAN_IT_WKU
            | CAN_IT_SLK | CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF
            | CAN_IT_LEC | CAN_IT_ERR, ENABLE);

    // CAN_ITConfig(CAN1, CAN_IT_ERR, DISABLE);
    //sce interrupt
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);
}

void CAN1_Init_Filter(uint16_t id1, uint16_t mask1, uint16_t id2, uint16_t mask2){
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

void Can1::init(const BaudRate & baudRate, const uint8_t & remap, const CanFilter & filter){
    settleTxPin(remap);
    settleRxPin(remap);
    // CAN1_Init_Filter(filter.id, filter.mask, 0, 0x0f);

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
        prescale = 12 * 8;
        break;
    case BaudRate::Mbps1:
        swj = CAN_SJW_2tq;
        bs1 = CAN_BS1_6tq;
        bs2 = CAN_BS2_5tq;
        prescale = 12;
        break;
    };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

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
    CAN_Init(CAN1, &config);
    CAN1_Init_Filter(filter.id, filter.mask, 0, 0xf);
    CAN1_IT_Init();
}

size_t Can1::pending(){
    size_t cnt = 0;
    for(uint8_t i = 0; i < 3; i++) cnt += bool(pending_tx_msg_ptrs[i]);
    return cnt;
}

void Can1::enableHwReTransmit(const bool en){
    config.CAN_NART = en ? DISABLE : ENABLE;
    if(en)  CAN1->CTLR &= ~CAN_CTLR_NART;
    else    CAN1->CTLR |=  CAN_CTLR_NART;
}

bool Can1::write(const CanMsg & msg){
    uint8_t mbox = CAN_Transmit(CAN1, msg.toTxMessagePtr());
    if(mbox == CAN_TxStatus_NoMailBox) return false;

    pending_tx_msg_ptrs[mbox] = std::make_unique<CanMsg>(msg);

    return true;
}

const CanMsg & Can1::read(){
    return *(pending_rx_msgs.getData());
}

size_t Can1::available(){
    return pending_rx_msgs.available();
}

uint8_t Can1::getRxErrCnt(){
    return CAN1->ERRSR >> 24;
}

uint8_t Can1::getTxErrCnt(){
    return CAN1->ERRSR >> 16;
}
uint8_t Can1::getErrCode(){
    return CAN_GetLastErrorCode(CAN1);
}

bool Can1::isTranmitting(){
    return bool(CAN1->STATR & CAN_STATR_TXM);
}

bool Can1::isReceiving(){
    return bool(CAN1->STATR & CAN_STATR_RXM);
}
bool Can1::isBusOff(){
    return CAN1->ERRSR & CAN_ERRSR_BOFF;
}

void Can1::cancelTransmit(const uint8_t mbox){
    CAN_CancelTransmit(CAN1, mbox);
}

void Can1::cancelAllTransmit(){
    CAN1->TSTATR |= (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
}

void Can1::enableFifoLock(const bool en){
    if(en) CAN1->CTLR |= CAN_CTLR_RFLM;
    else CAN1->CTLR &= ~CAN_CTLR_RFLM;
}

void Can1::enableIndexPriority(const bool en){
    if(en) CAN1->CTLR |= CAN_CTLR_TXFP;
    else CAN1->CTLR &= ~CAN_CTLR_TXFP;
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

#define HAVE_CAN1
#ifdef HAVE_CAN1
Can1 can1;
#endif



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

void Save_CAN1_Msg(const uint8_t fifo_index){
    // if(CAN_MessagePending(CAN1, fifo_index) == 0) return;

    // std::unique_ptr<CanMsg> rx_msg = std::make_unique<CanMsg>();
    // CAN_Receive(CAN1, fifo_index, rx_msg->toRxMessagePtr());
    // pending_rx_msgs.addData(std::move(rx_msg));
    CanMsg rx_msg;

    if(CAN_MessagePending(CAN1, fifo_index) == 0) return;

    CAN_Receive(CAN1, fifo_index, rx_msg.toRxMessagePtr());
    pending_rx_msgs.addData(std::make_shared<CanMsg>(rx_msg));
}

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void) {
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0)){
        //process rx pending
        Save_CAN1_Msg(CAN_FIFO0);
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
        Save_CAN1_Msg(CAN_FIFO1);
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

// #include "can1.hpp"

// static std::unique_ptr<CanMsg> pending_tx_msg_ptrs[3] = {nullptr};
// static RingBuf_t<std::shared_ptr<CanMsg>, 8> pending_rx_msgs;
// static CAN_InitTypeDef config;
// void CAN1_GPIO_Init(){
//     CHECK_INIT

//     GPIO_InitTypeDef      GPIO_InitSturcture = {0};
//     GPIO_PinRemapConfig(CAN1_REMAP, CAN1_REMAP_ENABLE);

//     GPIO_InitSturcture.GPIO_Pin = CAN1_TX_Pin;
//     GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
//     GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(CAN1_Port, &GPIO_InitSturcture);

//     GPIO_InitSturcture.GPIO_Pin = CAN1_RX_Pin;
//     GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(CAN1_Port, &GPIO_InitSturcture);
// }

// void CAN1_IT_Init(){


//     NVIC_InitTypeDef NVIC_InitStructure;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

//     //tx interrupt
//     /****************************************/
//     CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
//     CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);

//     NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//     NVIC_Init(&NVIC_InitStructure);

//     //rx0 interrupt
//     /***************************************/
//     CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0);
//     CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);

//     NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//     NVIC_Init(&NVIC_InitStructure);

//     //rx1 interrupt
//     /***************************************/
//     // CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
//     // CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);

//     // NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX1_IRQn;
//     // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//     // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//     // NVIC_Init(&NVIC_InitStructure);

//     // CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_WKU, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_SLK, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_EWG, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_EPV, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_BOF, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_LEC, ENABLE);
//     // CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);

//     // // CAN_ITConfig(CAN1, CAN_IT_ERR, DISABLE);
//     // //sce interrupt
//     // NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
//     // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//     // NVIC_Init(&NVIC_InitStructure);
// }

// void CAN1_Init_Filter(){
//     CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

//     CAN_FilterInitSturcture.CAN_FilterNumber = 0;

//     CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
//     CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

//     CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
//     CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0;
//     CAN_FilterInitSturcture.CAN_FilterIdHigh = 0;
//     CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0;


//     CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
//     CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;

//     CAN_FilterInit(&CAN_FilterInitSturcture);
// }

// void Can1::init(const BaudRate & baudRate, const uint8_t & remap, const uint16_t & mask){
//     CAN1_GPIO_Init();

//     uint8_t swj, bs1, bs2;
//     uint16_t prescale = 0;

//     switch(baudRate){
//     case BaudRate::Kbps125:
//         swj = CAN_SJW_2tq;
//         bs1 = CAN_BS1_6tq;
//         bs2 = CAN_BS2_5tq;
//         prescale = 12 * 8;
//         break;
//     case BaudRate::Mbps1:
//         swj = CAN_SJW_2tq;
//         bs1 = CAN_BS1_6tq;
//         bs2 = CAN_BS2_5tq;
//         prescale = 12;
//         break;
//     };

//     // RCC_ClocksTypeDef rcc_clocks;
//     // RCC_GetClocksFreq(&rcc_clocks);
//     // uint32_t pclk1 = rcc_clocks.PCLK1_Frequency;
//     // prescale = prescale * pclk1 / 72000000;
//     // prescale *= 2;
//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

//     config.CAN_Prescaler = prescale;
//     config.CAN_Mode = CAN_Mode_Normal;
//     config.CAN_SJW = swj;
//     config.CAN_BS1 = bs1;
//     config.CAN_BS2 = bs2;

//     config.CAN_TTCM = DISABLE;
//     config.CAN_ABOM = ENABLE;
//     config.CAN_AWUM = DISABLE;
//     config.CAN_NART = ENABLE;
//     config.CAN_RFLM = DISABLE;
//     config.CAN_TXFP = DISABLE;
//     CAN_Init(CAN1, &config);

//     CAN1_Init_Filter();
//     CAN1_IT_Init();
// }

// size_t Can1::pending(){
//     size_t cnt = 0;
//     for(uint8_t i = 0; i < 3; i++) cnt += bool(pending_tx_msg_ptrs[i]);
//     return cnt;
// }

// void Can1::enableHwReTransmit(const bool en){
//     config.CAN_NART = en ? DISABLE : ENABLE;
//     if(en)  CAN1->CTLR &= ~CAN_CTLR_NART;
//     else    CAN1->CTLR |=  CAN_CTLR_NART;
// }

// bool Can1::write(const CanMsg & msg){
//     uint8_t mbox = CAN_Transmit(CAN1, msg.toTxMessagePtr());
//     if(mbox == CAN_TxStatus_NoMailBox) return false;

//     pending_tx_msg_ptrs[mbox] = std::make_unique<CanMsg>(msg);

//     return true;
// }

// const CanMsg & Can1::read(){
//     return *(pending_rx_msgs.getData());
// }

// size_t Can1::available(){
//     return pending_rx_msgs.available();
// }

// uint8_t Can1::getRxErrCnt(){
//     return CAN1->ERRSR >> 24;
// }

// uint8_t Can1::getTxErrCnt(){
//     return CAN1->ERRSR >> 16;
// }
// uint8_t Can1::getErrCode(){
//     return CAN_GetLastErrorCode(CAN1);
// }

// bool Can1::isTranmitting(){
//     return bool(CAN1->STATR & CAN_STATR_TXM);
// }

// bool Can1::isReceiving(){
//     return bool(CAN1->STATR & CAN_STATR_RXM);
// }
// bool Can1::isBusOff(){
//     return CAN1->ERRSR & CAN_ERRSR_BOFF;
// }

// void Can1::cancelTransmit(const uint8_t mbox){
//     CAN_CancelTransmit(CAN1, mbox);
// }

// void Can1::cancelAllTransmit(){
//     CAN1->TSTATR |= (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
// }

// void Can1::enableFifoLock(const bool en){
//     if(en) CAN1->CTLR |= CAN_CTLR_RFLM;
//     else CAN1->CTLR &= ~CAN_CTLR_RFLM;
// }

// void Can1::enableIndexPriority(const bool en){
//     if(en) CAN1->CTLR |= CAN_CTLR_TXFP;
//     else CAN1->CTLR &= ~CAN_CTLR_TXFP;
// }
// __fast_inline constexpr uint32_t Mailbox_Index_To_TSTATR(const uint8_t mbox){
//     switch(mbox){
//     case 0:
//         return CAN_TSTATR_RQCP0;
//     case 1:
//         return CAN_TSTATR_RQCP1;
//     case 2:
//         return CAN_TSTATR_RQCP2;
//     default:
//         static_assert(true, "Invalid value for switch!");
//         return 0;
//     }
// }

// __fast_inline bool CAN_Mailbox_Done(CAN_TypeDef* CANx, const uint8_t mbox){
//     const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
//     // static_assert(TSTATR_FLAG == CAN_TSTATR_RQCP0 || TSTATR_FLAG == CAN_TSTATR_RQCP1 || TSTATR_FLAG == CAN_TSTATR_RQCP2);
//     return ((CANx->TSTATR & TSTATR_FLAG) == TSTATR_FLAG);
// }

// __fast_inline void CAN_Mailbox_Clear(CAN_TypeDef* CANx, const uint8_t mbox){
//     const uint32_t TSTATR_FLAG = Mailbox_Index_To_TSTATR(mbox);
//     // static_assert(TSTATR_FLAG == CAN_TSTATR_RQCP0 || TSTATR_FLAG == CAN_TSTATR_RQCP1 || TSTATR_FLAG == CAN_TSTATR_RQCP2);
//     CANx->TSTATR = TSTATR_FLAG;
// }

// #define HAVE_CAN1
// #ifdef HAVE_CAN1
// Can1 can1;
// #endif



// __interrupt
// void USB_HP_CAN1_TX_IRQHandler(void){
//     for(uint8_t mbox = 0; mbox < 3; mbox++){

//         if(pending_tx_msg_ptrs[mbox] && CAN_Mailbox_Done(CAN1, mbox)){ // if existing message done
//             uint8_t tx_status = CAN_TransmitStatus(CAN1, mbox);
//             switch (tx_status){
//             case(CAN_TxStatus_Failed):
//                 //process failed message
//                 pending_tx_msg_ptrs[mbox].reset();
//                 break;
//             case(CAN_TxStatus_Ok):
//                     // uart2.println("tx doen");
//                 //process success message
//                 pending_tx_msg_ptrs[mbox].reset();
//                 break;
//             }
//             CAN_Mailbox_Clear(CAN1, mbox);
//         }
//     }
// }

// void Save_CAN1_Msg(const uint8_t fifo_index){
//     CanMsg rx_msg;

//     if(CAN_MessagePending(CAN1, fifo_index) == 0) return;

//     CAN_Receive(CAN1, fifo_index, rx_msg.toRxMessagePtr());
//     pending_rx_msgs.addData(std::make_shared<CanMsg>(rx_msg));
// }

// __interrupt
// void USB_LP_CAN1_RX0_IRQHandler(void) {
//     if (CAN_GetITStatus(CAN1, CAN_IT_FMP0)){
//         //process rx pending
//         Save_CAN1_Msg(CAN_FIFO0);
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
//     }else if(CAN_GetITStatus(CAN1, CAN_IT_FF0)){
//         //process rx full
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);
//     }else if(CAN_GetITStatus(CAN1, CAN_IT_FOV0)){
//         //process rx overrun
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);
//     }
// }

// __interrupt
// void CAN1_RX1_IRQHandler(void){
//     if (CAN_GetITStatus(CAN1, CAN_IT_FMP1)){
//         //process rx pending
//         Save_CAN1_Msg(CAN_FIFO1);
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
//     }else if(CAN_GetITStatus(CAN1, CAN_IT_FF1)){
//         //process rx full
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FF1);
//     }else if(CAN_GetITStatus(CAN1, CAN_IT_FOV1)){
//         //process rx overrun
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FOV1);
//     }
// }

// __interrupt
// void CAN1_SCE_IRQHandler(void){
//     if (CAN_GetITStatus(CAN1, CAN_IT_WKU)) {
//         // Handle Wake-up interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_SLK)) {
//         // Handle Sleep acknowledge interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_ERR)) {
//         // Handle Error interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_EWG)) {
//         // Handle Error warning interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_EPV)) {
//         // Handle Error passive interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_BOF)) {
//         // Handle Bus-off interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);
//     } else if (CAN_GetITStatus(CAN1, CAN_IT_LEC)) {
//         // Handle Last error code interrupt
//         CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);
//     } else {
//         // Handle other interrupts or add more cases as needed
//     }
// }