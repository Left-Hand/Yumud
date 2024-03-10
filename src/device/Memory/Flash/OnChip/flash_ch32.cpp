#include "flash.hpp"

#define PAGE_SIZE     256

//EEPROM_PART0 0x0800F000 - 0x0800F800 size 2048
//EEPROM_PART1 0x0800F800 - 0x08010000 size 2048

/* Erase from PART_BASE_ADDRESS, size EEPROM_PART_SIZE */
int EE_ErasePart(int part)
{
    uint32_t time_use=HAL_GetTick();
    int ret;
	uint32_t erase_address = part == 0 ? PART0_BASE_ADDRESS : PART1_BASE_ADDRESS;
	uint32_t erase_size = part == 0 ? EEPROM_PART0_SIZE : EEPROM_PART1_SIZE;

	if (erase_address < PART0_BASE_ADDRESS || erase_address >= PART1_END_ADDRESS)
		return HAL_ERROR;

	log_printf(LOG_Debug, "FLASH Erase", erase_address, "size", erase_size);

    FLASH_Unlock();

	ret = FLASH_ErasePage(erase_address);
    if(ret != FLASH_COMPLETE)
    {
        FLASH_Lock();
        log_printf(LOG_Error, "FLASH Erase error", ret);
        return HAL_ERROR;
    }
	ret = FLASH_ErasePage(erase_address + PAGE_SIZE);
    if(ret != FLASH_COMPLETE)
    {
        FLASH_Lock();
        log_printf(LOG_Error, "FLASH Erase error", ret);
        return HAL_ERROR;
    }

    FLASH_Lock();
    log_printf(LOG_Verbose, "FLASH Erase over, use", HAL_GetTick()-time_use, "ms");
    return HAL_OK;
}

/* If the write operation is not a 32-bit atomic operation,
   Write the low 16-bit first, then write the high 16-bit */
int EE_ProgramWord(uint32_t Address, uint32_t Data)
{
    int ret;
	
    if(Address<PART0_BASE_ADDRESS || Address>=PART1_END_ADDRESS)
        return HAL_ERROR;

    log_printf(LOG_Debug, "FLASH Program:", Address, Data);

    FLASH_Unlock();
	
	ret=FLASH_ProgramHalfWord(Address, Data);
    if(ret != FLASH_COMPLETE)
    {
        FLASH_Lock();
        log_printf(LOG_Error, "FLASH Program error", ret);
        return HAL_ERROR;
    }
	ret=FLASH_ProgramHalfWord(Address+2, Data>>16);
    if(ret != FLASH_COMPLETE)
    {
        FLASH_Lock();
        log_printf(LOG_Error, "FLASH Program error", ret);
        return HAL_ERROR;
    }

    FLASH_Lock();
    return HAL_OK;
}
