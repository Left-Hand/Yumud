// // /*
// //  * @Author: error: git config user.name && git config user.email & please set dead value or install git
// //  * @Date: 2023-01-05 19:48:21
// //  * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
// //  * @LastEditTime: 2023-01-11 12:35:59
// //  * @FilePath: \testch32v203\src\main.cpp
// //  * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
// //  */
// // #include "debug.h"
// // // #include "stdbool.h"
// // // #include <map>
// // // #include <vector>
// // #pragma GCC optimize(0)
// // #define BUILTIN_LED_PORT GPIOC
// // #define BUILTIN_LED_PIN GPIO_Pin_13

// // class Led{
// //     private:
// //         GPIO_TypeDef* port = nullptr;
// //         uint16_t pin = 0;
// //         bool inverse = false;

// //     public:
// //         Led(){};
// //         Led(GPIO_TypeDef* _port,uint16_t _pin, bool _inv):port(_port), pin(_pin), inverse(_inv){};
// //         ~Led(){};
        
// //         void on(){GPIO_WriteBit(port, pin, (BitAction)!inverse);};
// //         void off(){GPIO_WriteBit(port, pin, (BitAction)inverse);};
// // };


// // Led buitinLed(GPIOC, GPIO_Pin_13, false);

// // void GPIO_Toggle_INIT()
// // {

// //     RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE );


// //     do{ //disable tamper func and enable pc 13,14,15
// //         PWR_BackupAccessCmd( ENABLE );
// //         RCC_LSEConfig( RCC_LSE_OFF );
// //         BKP_TamperPinCmd(DISABLE);
// //         PWR_BackupAccessCmd(DISABLE);

// //         GPIO_InitTypeDef  GPIO_InitStructure = {0};
// //         GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
// //         GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
// //         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          
// //         GPIO_Init(GPIOC, &GPIO_InitStructure);
// //     }while(false);
    


// // }


// // void HW_Init(){
// //     SystemInit();
// //     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
// //     Delay_Init();
// //     GPIO_Toggle_INIT();
// //     USART_Printf_Init(115200);
// // }
// // void setup(){
// //     HW_Init();
// //     // printf("SystemClk:%d\r\n", SystemCoreClock);
// // }

// // using namespace std;


// // int get_sum(int x){
// //     int sum = x;
// //     while(x--) sum += x;
// //     return sum;
// // }


// // void loop(){
// //     Delay_Ms(20);        
// //     // GPIO_SetBits(BUILTIN_LED_PORT, BUILTIN_LED_PIN);
// //     buitinLed.on();
// //     Delay_Ms(20);
// //     buitinLed.off();
// //     // Delay_Ms(get_sum(20));
// //     // GPIO_ResetBits(BUILTIN_LED_PORT, BUILTIN_LED_PIN);
// //     // GPIO_WriteBit(BUILTIN_LED_PORT, BUILTIN_LED_PIN, (BitAction)true);
// //     // GPIO_WriteBit(BUILTIN_LED_PORT, BUILTIN_LED_PIN, (BitAction)false);
// //     // std::vector<int> testvec;
// //     // testvec.push_back(1);
// //     // printf("%d", testvec[0]);
// //     // map < int, int > m1;
// //     // map < int, int >::iterator m1_Iter;
// //     // m1.insert ( pair < int, int >  ( 1, 20 ) );
// //     // m1.insert ( pair < int, int >  ( 4, 40 ) );
// //     // m1.insert ( pair < int, int >  ( 3, 60 ) );
// //     // m1.insert ( pair < int, int >  ( 2, 50 ) );
// //     // m1.insert ( pair < int, int >  ( 6, 40 ) );
// //     // m1.insert ( pair < int, int >  ( 7, 30 ) );
// //     // printf("The original map m1 is:");
// //     // for ( m1_Iter = m1.begin( ); m1_Iter != m1.end( ); m1_Iter++ )
// //     //     printf("%d, %d", m1_Iter->first, m1_Iter->second);
// // }


// // extern "C" int main(){
// //     setup();
// //     while(1)
// //     {
// //         loop();
// //     }
// // }
// /********************************** (C) COPYRIGHT *******************************
//  * File Name          : main.c
//  * Author             : WCH
//  * Version            : V1.0.0
//  * Date               : 2022/08/08
//  * Description        : Main program body.
//  * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
//  * SPDX-License-Identifier: Apache-2.0
//  *******************************************************************************/

// /*
//  *@Note
//  GPIOÀý³Ì£º
//  PD0ÍÆÍìÊä³ö¡£

// */

// #include "debug.h"

// /* Global define */

// /* Global Variable */

// /*********************************************************************
//  * @fn      GPIO_Toggle_INIT
//  *
//  * @brief   Initializes GPIOA.0
//  *
//  * @return  none
//  */
// void GPIO_Toggle_INIT(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure = {0};

//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOD, &GPIO_InitStructure);
// }

// /*********************************************************************
//  * @fn      main
//  *
//  * @brief   Main program.
//  *
//  * @return  none
//  */
// int main(void)
// {
//     u8 i = 0;

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//     Delay_Init();
//     USART_Printf_Init(115200);
//     printf("SystemClk:%d\r\n", SystemCoreClock);

//     printf("GPIO Toggle TEST\r\n");
//     GPIO_Toggle_INIT();

//     while(1)
//     {
//         Delay_Ms(250);
//         GPIO_WriteBit(GPIOD, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
//     }
// }
