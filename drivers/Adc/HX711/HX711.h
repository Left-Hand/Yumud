
// #ifndef __HX711_H
// #define __HX711_H
 
// #include "drivers/device_defs.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

// #define HX711_SCK_Port GPIOB
// #define HX711_SDO_Port GPIOB
// #define HX711_SCK_Pin GPIO_Pin_0
// #define HX711_SDO_Pin GPIO_Pin_1
 
// #define HX711_SCK_ON (HX711_SCK_Port -> BSHR = HX711_SCK_Pin)
// #define HX711_SCK_OFF (HX711_SCK_Port -> BCR = HX711_SCK_Pin)
// #define HX711_SDO_ON (HX711_SDO_Port -> BSHR = HX711_SDO_Pin)
// #define HX711_SDO_OFF (HX711_SDO_Port -> BCR = HX711_SDO_Pin)
// #define HX711_SDO_READ ((HX711_SDO_Port -> INDR) & HX711_SDO_Pin)

// #define HX711_ERR_DATA 0xFFFFFFFF

// enum{
//     IDLE,
//     WAIT
// };

// void HX711_GPIO_Init(void);
// void HX711_Cali(void);
// int32_t HX711_Get_Weight(void);
// uint8_t HX711_Valid(void);

// #ifdef __cplusplus
// }
// #endif

// #endif