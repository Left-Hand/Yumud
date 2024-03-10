/* eeprom in flash
 * Copyright (c) 2021-2022 epoko
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _EEPROM_IN_FLASH_H_
#define _EEPROM_IN_FLASH_H_

#include "../clock/clock.h"
#include "stdint.h"
#include "string.h"


/* eeprom storage data num max, 1-65535, unit halfword.
Must be smaller than (EEPROM_PART_SIZE/4)-2 */
#define EEPROM_NUM_MAX          12		//*2byte

/* EEPROM Use two partitions, each partition size is
an integer multiple of the erased page */
#define EEPROM_PART0_SIZE       (2048)  //eeprom part size 2K, total size 4K
#define EEPROM_PART1_SIZE       (EEPROM_PART0_SIZE)

/* EEPROM start address in Flash */
#define EEPROM_START_ADDRESS    (0x0800F000U)  //use flash 0x0800F000 - 0x08010000, for 64k flash mcu
#define EEPROM_END_ADDRESS      (EEPROM_START_ADDRESS + EEPROM_PART0_SIZE + EEPROM_PART1_SIZE)


/* Pages 0 and 1 base and end addresses */
#define PART0_BASE_ADDRESS      (EEPROM_START_ADDRESS)
#define PART0_END_ADDRESS       ((PART0_BASE_ADDRESS + EEPROM_PART0_SIZE))

#define PART1_BASE_ADDRESS      (PART0_END_ADDRESS)
#define PART1_END_ADDRESS       ((PART1_BASE_ADDRESS + EEPROM_PART1_SIZE))

/* PAGE is marked to record data */
// #define PART_USED_MARK          (0xEAE5D135 + EEPROM_NUM_MAX)	//Different number, use new data
#define PART_USED_MARK          (0xEAE5D135)  //Different number, use old data

#ifndef log_printf
    #include "bus/uart/uart1.hpp"
	#define LOG_Assert "[A] "
	#define LOG_Error "[E] "
	#define LOG_Warn "[W] "
	#define LOG_Info "[I] "
	#define LOG_Debug "[D] "
	#define LOG_Verbose "[V] "
	#define log_printf(...) uart1.println(__VA_ARGS__)
#endif

#ifndef USE_HAL_DRIVER
	#define HAL_GetTick() millis()
	#define HAL_Delay(ms) delay(ms)
typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;
#endif

int EEPROM_Init(void *default_data);
int EEPROM_Format(void *default_data);
uint16_t EEPROM_Read(uint16_t Address);
int EEPROM_Write(uint16_t Address, uint16_t Data);
int EEPROM_Read_Buf(uint16_t Address, uint16_t *buf, uint16_t length);
int EEPROM_Write_Buf(uint16_t Address, uint16_t *buf, uint16_t length);
#define Config_Read_Buf(addr, buf, length) EEPROM_Read_Buf((addr)/2, (uint16_t *)(buf), (length)/2)
#define Config_Write_Buf(addr, buf, length) EEPROM_Write_Buf((addr)/2, (uint16_t *)(buf), (length)/2)

int EE_ErasePart(int part);
int EE_ProgramWord(uint32_t Address, uint32_t Data);
#define EE_ReadWord(Addr) (*(volatile uint32_t*)Addr)

#endif
