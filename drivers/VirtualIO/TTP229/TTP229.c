// #include "TTP229.h"

// volatile static int8_t num = -1;
// volatile static uint16_t map = 0;

// void TTP229_GPIO_Init(void){
//     CHECK_INIT

// 	GPIO_InitTypeDef GPIO_InitStructure;
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 
// 	GPIO_InitStructure.GPIO_Pin = TTP229_SCK_Pin;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(TTP229_SCK_Port, &GPIO_InitStructure);	

//     GPIO_InitStructure.GPIO_Pin = TTP229_SDO_Pin;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(TTP229_SDO_Port, &GPIO_InitStructure);  
	
//     TTP229_SDO_ON;
//     TTP229_SCK_ON;
// }


// void TTP229_Scan(){
//     if(TTP229_SDO_READ) return;
//     TTP229_SCK_OFF;
//     uint16_t new_map = 0;
//     for (int i = 0; i < 16; i++)
//     {
//         TTP229_SCK_ON;
//         // __nopn(80);
//         delay(1);
//         TTP229_SCK_OFF;
//         // __nopn(80);
//         delay(1);
//         if(TTP229_SDO_READ) new_map |= 1;
//         new_map <<= 1;
//     }

//     map = new_map;
//     uint8_t new_num = 0;
//     while(new_map != (1 << new_num)){
//         new_num++;
//         if(new_num >= 16){
//             num = -1;
//             return;
//         }
//     }
//     num = new_num;
// }

// int8_t TTP229_Get_Key(){return num + 1;}
// uint16_t TTP229_Get_Map(){return map;}

