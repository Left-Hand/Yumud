// /********************************** (C) COPYRIGHT *******************************
//  * File Name          : main.c
//  * Author             : WCH
//  * Version            : V1.0.0
//  * Date               : 2021/06/06
//  * Description        : Main program body.
//  * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
//  * SPDX-License-Identifier: Apache-2.0
//  *******************************************************************************/

// /*
//  *@Note
//  CAN正常模式，标准帧和扩展帧数据收发：
//  CAN_Tx(PB9),CAN_Rx(PB8)
//  该例程需外接CAN收发器，可演示 Standard_Frame 和 Extended_Frame。
//  Standard_Frame：包括 1个32bit过滤器屏蔽位模式、2个16bit过滤器屏蔽位模式、
//  1个32bit标识符列表模式、2个16bit标识符列表模式；
//  Extended_Frame：1个32bit过滤器屏蔽位模式。

// */

// #include "debug.h"

// /* CAN Mode Definition */
// #define TX_MODE           1
// #define RX_MODE           0

// /* Frame Format Definition */
// #define Standard_Frame    1
// #define Extended_Frame    0

// /* CAN Communication Mode Selection */
// // #define CAN_MODE          TX_MODE
// // #define CAN_MODE   RX_MODE
// volatile uint8_t CAN_MODE = 0;
// volatile uint8_t send_done = 0;
// volatile uint8_t send_mbox = 0;

// /* Frame Formate Selection */
// #define Frame_Format      Standard_Frame
// //#define Frame_Format   Extended_Frame

// void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));


// void CAN_Mode_Init(uint8_t tsjw, uint8_t tbs2, uint8_t tbs1, uint16_t brp, uint8_t mode, uint8_t isrtr){
//     GPIO_InitTypeDef      GPIO_InitSturcture = {0};
//     CAN_InitTypeDef       CAN_InitSturcture = {0};
//     CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};

//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

//     GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

//     GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_9;
//     GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
//     GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOB, &GPIO_InitSturcture);

//     GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_8;
//     GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(GPIOB, &GPIO_InitSturcture);

//     GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;
//     GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPD;
//     GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOB, &GPIO_InitSturcture);
    
//     CAN_MODE = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);

//     CAN_InitSturcture.CAN_TTCM = DISABLE;
//     CAN_InitSturcture.CAN_ABOM = ENABLE;
//     CAN_InitSturcture.CAN_AWUM = DISABLE;
//     CAN_InitSturcture.CAN_NART = ENABLE;
//     CAN_InitSturcture.CAN_RFLM = DISABLE;
//     CAN_InitSturcture.CAN_TXFP = DISABLE;
//     CAN_InitSturcture.CAN_Mode = mode;
//     CAN_InitSturcture.CAN_SJW = tsjw;
//     CAN_InitSturcture.CAN_BS1 = tbs1;
//     CAN_InitSturcture.CAN_BS2 = tbs2;
//     CAN_InitSturcture.CAN_Prescaler = brp;
//     CAN_Init(CAN1, &CAN_InitSturcture);

//     CAN_FilterInitSturcture.CAN_FilterNumber = 0;

//     CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
//     CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_16bit;

//     CAN_FilterInitSturcture.CAN_FilterIdLow = 0xffff << 5;
//     CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0xffff << 5;

//     if (isrtr){
//         CAN_FilterInitSturcture.CAN_FilterIdHigh = (0xffff << 5) | CAN_RTR_Remote;
//         CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = (0 << 5) | (CAN_Id_Extended | CAN_RTR_Remote);
//     }else{
//         CAN_FilterInitSturcture.CAN_FilterIdHigh = (0xffff << 5);
//         CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = (0 << 5) | (CAN_Id_Extended);
//     }

//     CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
//     CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    
//     CAN_FilterInit(&CAN_FilterInitSturcture);
// }

// void CAN_TX_IT_Init(){
//     CAN_ClearITPendingBit(CAN1, CAN_IT_TME);
//     CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);

//     NVIC_InitTypeDef NVIC_InitStructure;
//     NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
// }

// void CAN_RX_IT_Init(){
//     CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
//     CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

//     NVIC_InitTypeDef NVIC_InitStructure;
//     NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);

// }
// void CAN_Send_Msg(uint8_t *msg, uint8_t len)
// {
//     uint16_t i = 0;

//     CanTxMsg CanTxStructure;

//     CanTxStructure.StdId = 0;
//     CanTxStructure.IDE = CAN_Id_Standard;
//     CanTxStructure.RTR = CAN_RTR_Data;
//     CanTxStructure.DLC = len;

//     for(i = 0; i < len; i++) {
//         CanTxStructure.Data[i] = msg[i];
//     }

//     uint8_t mbox = CAN_Transmit(CAN1, &CanTxStructure);
//     send_mbox = mbox;

//     printf("%d->%d\r\n", mbox, CanTxStructure.Data[0]);

//     // for(uint8_t i = 0; i < 8; i++) {
//     //     printf("%02x\r\n", CanTxStructure.Data[i]);
//     // }
// }

// uint8_t CAN_Send_Msg_Sync(uint8_t *msg, uint8_t len)
// {
//     uint16_t i = 0;

//     CanTxMsg CanTxStructure;

//     CanTxStructure.StdId = 0;
//     CanTxStructure.IDE = CAN_Id_Standard;
//     CanTxStructure.RTR = CAN_RTR_Data;
//     CanTxStructure.DLC = len;

//     for(i = 0; i < len; i++) {
//         CanTxStructure.Data[i] = msg[i];
//     }

//     uint8_t mbox = CAN_Transmit(CAN1, &CanTxStructure);

//     while((CAN_TransmitStatus(CAN1, mbox) != CAN_TxStatus_Ok) && (i < 0xFFF))
//     {
//         i++;
//     }

//     return (i);
// }

// uint8_t CAN_Receive_Msg(uint8_t *buf)
// {
//     uint8_t i;

//     CanRxMsg CanRxStructure;

//     if(CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
//     {
//         return 0;
//     }

//     CAN_Receive(CAN1, CAN_FIFO0, &CanRxStructure);

//     printf("Recv Msg:%d\r\n", CanRxStructure.DLC);

//     for(i = 0; i < CanRxStructure.DLC; i++) {
//         buf[i] = CanRxStructure.Data[i];
//         printf("%02x\r\n", CanRxStructure.Data[i]);
//     }

//     return CanRxStructure.DLC;
// }

// void Recv_Msg(){
//     uint8_t pxbuf[8];
//     CAN_Receive_Msg(pxbuf);
// }

// void Send_Msg(){
//     uint8_t pxbuf[8];
//     static uint8_t cnt = 0;
//     cnt++;
//     for(uint8_t i = 0; i < 8; i++) {
//         pxbuf[i] = cnt + i;
//     }

//     CAN_Send_Msg(pxbuf, 8);
// }

// void Clear_Mailbox(){
//     CAN1->TSTATR |= (CAN_TSTATR_ABRQ0 | CAN_TSTATR_ABRQ1 | CAN_TSTATR_ABRQ2);
// }

// void Send_Done_CB(){
//     Send_Msg();
//     send_done = 0;
// }
// int main(void)
// {


//     Delay_Init();
//     USART_Printf_Init(921600);

//     RCC_PCLK1Config(RCC_HCLK_Div1);
//     RCC_PCLK2Config(RCC_HCLK_Div1);
//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

//     CAN_Mode_Init(CAN_SJW_2tq, CAN_BS2_5tq, CAN_BS1_6tq, 12, CAN_Mode_Normal,CAN_MODE);
//     CAN_DBGFreeze(CAN1, DISABLE);

//     CAN_TX_IT_Init();
//     CAN_RX_IT_Init();

//     if(CAN_MODE == TX_MODE){
//         // Clear_Mailbox();
//         // Send_Msg();
//         printf("Tx Mode\r\n");
//         // Clear_Mailbox();
//         // uint8_t buf[8] = {1,0};

//         // CAN_Send_Msg_Sync(buf, 8);
//         // CAN_TX_IT_Init();

//         // CAN_Send_Msg();
//         Send_Msg();



//         while(1){
//             // printf("%d\r\n", (int)(CAN_TransmitStatus(CAN1, mbox)));
//             // printf("Send_Done: %d\r\n", send_done);
//             // Send_Msg();
//             if(send_done){
//                 Send_Done_CB();
//             }
//             Delay_Ms(20);
//         }
//     }else{
//         // CAN_RX_IT_Init();
//         printf("Rx Mode\r\n");


//         while(1){
//         }
//     }
//     /* Bps = 250Kbps */

// }



// void USB_HP_CAN1_TX_IRQHandler(void){
//     if (CAN_GetITStatus(CAN1, CAN_IT_TME) != RESET)
//     {
//         if(CAN_TransmitStatus(CAN1, send_mbox) == CAN_TxStatus_Ok){
//             send_done = 1;

//         }
        
//         printf("T!\r\n");
//         CAN_ClearITPendingBit(CAN1, CAN_IT_TME);  // 清除发送中断标志位
//     }
// }

// void USB_LP_CAN1_RX0_IRQHandler(void) {
//     if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
//     {
//         printf("R!\r\n");
//         Recv_Msg();
//         CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
//     }
// }
