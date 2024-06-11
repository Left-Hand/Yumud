// /************************************************************************************
						
// *************************************************************************************/
// #include "HX711.h"

// volatile static uint32_t base_weight = 0;
// volatile static int32_t result_weight = 0;
// volatile static uint8_t progress = IDLE;

// void HX711_GPIO_Init(void){
//     CHECK_INIT

// 	GPIO_InitTypeDef GPIO_InitStructure;
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 
// 	GPIO_InitStructure.GPIO_Pin = HX711_SCK_Pin;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(HX711_SCK_Port, &GPIO_InitStructure);	

//     GPIO_InitStructure.GPIO_Pin = HX711_SDO_Pin;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(HX711_SDO_Port, &GPIO_InitStructure);  
	
// 	HX711_SCK_ON;
// }

// static void HX711_Start(){
//   	HX711_SDO_ON;
//     __nopn(2);
//   	HX711_SCK_OFF;
// }

// static int32_t HX711_Get(){
//   	uint32_t data=0; 

//   	for(uint8_t i=0;i<24;i++)
// 	{ 
// 	  	HX711_SCK_ON;
// 	  	data=data<<1; 
//         __nopn(2);
// 		HX711_SCK_OFF;
// 	  	if(HX711_SDO_READ) data++; 
// 	} 

//  	HX711_SCK_ON;
//     __nopn(2);
// 	HX711_SCK_OFF; 

//     data ^= 0x800000;
// 	return(data);
// }

// static uint32_t HX711_Read(void)
// {
//     switch(progress){
    
//     case IDLE:
//         HX711_Start();
//         progress = WAIT;
//         return HX711_ERR_DATA;
//     case WAIT:
//         if(!HX711_SDO_READ){
//             uint32_t temp = HX711_Get();
//             progress = IDLE;
//             return temp;
//         }
//     }
//     return HX711_ERR_DATA;
// }
 
// void HX711_Cali(void){
//     uint32_t temp = HX711_ERR_DATA;
//     while(temp == HX711_ERR_DATA) temp = HX711_Read();
// 	base_weight = temp;	
// } 

// int32_t HX711_Get_Weight(void){
// 	uint32_t temp = HX711_Read();
//     if(temp != HX711_ERR_DATA){
//         result_weight = temp - base_weight;
//         return result_weight;
//     }else{
//         result_weight = HX711_ERR_DATA;
//         return HX711_ERR_DATA;
//     }
// }

// uint8_t HX711_Valid(){
//     return (result_weight != HX711_ERR_DATA);
// }