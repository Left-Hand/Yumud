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
#include "flash.hpp"

static uint16_t EEPROM_data[EEPROM_NUM_MAX] = {0};
static int EEPROM_PART_USE = -1;

static int EE_Init(void);
static int EE_ReadVariable(uint32_t VirtAddress, uint16_t *Data);
static int EE_WriteVariable(uint32_t VirtAddress, uint16_t Data);
static int EE_Format(int part);

int EEPROM_Init(void *default_data)
{
	uint32_t time_use = HAL_GetTick();

	if (default_data){
		memcpy(EEPROM_data, default_data, sizeof(EEPROM_data));
        log_printf("use default data");
    }
	if (EE_Init() != HAL_OK)
	{
		log_printf(LOG_Error, "EEPROM Init error\r\n");
		return HAL_ERROR;
	}

    log_printf("eeprom init done");
	for (int i = 0; i < EEPROM_NUM_MAX; i++){
		EE_ReadVariable(i, &EEPROM_data[i]);
    }
	log_printf(LOG_Verbose, "EEPROM_Init use ", HAL_GetTick() - time_use, "ms");
	return HAL_OK;
}

uint16_t EEPROM_Read(uint16_t Address)
{
	if (Address >= EEPROM_NUM_MAX)
		return 0;

	return EEPROM_data[Address];
}

int EEPROM_Write(uint16_t Address, uint16_t Data)
{
	if (EEPROM_PART_USE == -1 || Address >= EEPROM_NUM_MAX)
		return HAL_ERROR;

	if (EEPROM_data[Address] == Data)
		return HAL_OK;

	if (EE_WriteVariable(Address, Data) != HAL_OK)
	{
		// EEPROM_data[Address] = Data;
		log_printf(LOG_Error, "EEPROM write error\r\n");
		return HAL_ERROR;
	}

	return HAL_OK;
}

int EEPROM_Read_Buf(uint16_t Address, uint16_t *buf, uint16_t length)
{
	if (Address + length > EEPROM_NUM_MAX)
		return HAL_ERROR;

	memcpy(buf, EEPROM_data + Address, length << 1);
	return HAL_OK;
}

int EEPROM_Write_Buf(uint16_t Address, uint16_t *buf, uint16_t length)
{
	if (EEPROM_PART_USE == -1 || Address + length > EEPROM_NUM_MAX)
		return HAL_ERROR;

	while (length--)
	{
		if (EEPROM_data[Address] != *buf)
		{
			if (EE_WriteVariable(Address, *buf) != HAL_OK)
			{
				// EEPROM_data[Address] = *buf;
				log_printf(LOG_Error, "EEPROM write buf error\r\n");
				return HAL_ERROR;
			}
		}

		buf++;
		Address++;
	}
	return HAL_OK;
}

int EEPROM_Format(void *default_data)
{
	if (default_data)
		memcpy(EEPROM_data, default_data, sizeof(EEPROM_data));
	else
		memset(EEPROM_data, 0, sizeof(EEPROM_data));

	if (EE_Format(0) != HAL_OK)
	{
		log_printf(LOG_Error, "EEPROM Format error\r\n");
		return HAL_ERROR;
	}

	EEPROM_PART_USE = 0;

	if (EE_ErasePart(1) != HAL_OK)
		return HAL_ERROR;
	return HAL_OK;
}

/* -------------- EE Private function -------------- */
static int EE_Init(void)
{
    log_printf("part0 mask", EE_ReadWord(PART0_BASE_ADDRESS));
    log_printf("part1_mask", EE_ReadWord(PART1_BASE_ADDRESS));

	if (EE_ReadWord(PART0_BASE_ADDRESS) == PART_USED_MARK)
	{
		EEPROM_PART_USE = 0;
	}
	else if (EE_ReadWord(PART1_BASE_ADDRESS) == PART_USED_MARK)
	{
		EEPROM_PART_USE = 1;
	}
	else
	{
		if (EE_Format(0) != HAL_OK)
			return HAL_ERROR;
		EEPROM_PART_USE = 0;
	}

	return HAL_OK;
}

static int EE_ReadVariable(uint32_t VirtAddress, uint16_t *Data)
{
	uint32_t time_use = HAL_GetTick();
	uint32_t PartStartAddress = (EEPROM_PART_USE == 0 ? PART0_BASE_ADDRESS : PART1_BASE_ADDRESS);
	uint32_t Address = (EEPROM_PART_USE == 0 ? PART0_END_ADDRESS : PART1_END_ADDRESS) - 4;
	uint32_t temp;

	VirtAddress <<= 16;
	for (; Address > PartStartAddress; Address -= 4)
	{
		temp = EE_ReadWord(Address);
		if ((temp & 0xffff0000) == VirtAddress)
		{
			*Data = temp & 0xffff;
			log_printf(LOG_Verbose, "EE_ReadVariable use", HAL_GetTick() - time_use, "ms");
			return HAL_OK;
		}
	}
	log_printf(LOG_Warn, "EE_ReadVariable failed, use", HAL_GetTick() - time_use, "ms");
	return HAL_ERROR;
}

static int EE_WriteVariable(uint32_t VirtAddress, uint16_t Data)
{
	uint32_t time_use = HAL_GetTick();
	uint32_t PartStartAddress = (EEPROM_PART_USE == 0 ? PART0_BASE_ADDRESS : PART1_BASE_ADDRESS);
	uint32_t Address = (EEPROM_PART_USE == 0 ? PART0_END_ADDRESS : PART1_END_ADDRESS) - 4;

	if (EE_ReadWord(Address) != 0xffffffff)
	{
		EEPROM_data[VirtAddress] = Data;
		if (EE_Format(EEPROM_PART_USE == 0 ? 1 : 0) != HAL_OK)
			return HAL_ERROR;

		EEPROM_PART_USE = EEPROM_PART_USE == 0 ? 1 : 0;

#if PART0_BASE_ADDRESS != PART1_BASE_ADDRESS
		if (EE_ErasePart(EEPROM_PART_USE == 0 ? 1 : 0) != HAL_OK)
			return HAL_ERROR;
#endif

		return HAL_OK;
	}
	else
	{
		for (Address -= 4; Address > PartStartAddress; Address -= 4)
		{
			if (EE_ReadWord(Address) != 0xffffffff)
				break;
		}

		if (EE_ProgramWord(Address + 4, (VirtAddress << 16) | Data) != HAL_OK)
			return HAL_ERROR;

		EEPROM_data[VirtAddress] = Data;
		log_printf(LOG_Verbose, "EE_WriteVariable use ", HAL_GetTick() - time_use, "ms");

		return HAL_OK;
	}
}

static int EE_Format(int part)
{
	uint32_t i;
	uint32_t Part_Addr = (part == 0 ? PART0_BASE_ADDRESS : PART1_BASE_ADDRESS);
	uint32_t Address;

	if (EE_ErasePart(part) != HAL_OK)
		return HAL_ERROR;

	log_printf(LOG_Info, "EEPROM move data to partition", part);
	Address = Part_Addr + 4;
	for (i = 0; i < EEPROM_NUM_MAX; i++)
	{
		if (EE_ProgramWord(Address, (i << 16) | EEPROM_data[i]) != HAL_OK)
			return HAL_ERROR;

		Address += 4;
	}

	if (EE_ProgramWord(Part_Addr, PART_USED_MARK) != HAL_OK)
		return HAL_ERROR;

	return HAL_OK;
}
