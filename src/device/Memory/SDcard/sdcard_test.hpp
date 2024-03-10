// #include "bsp_spi_sdcard.hpp"

// using namespace MOS::User::Global;

// /* Private typedef -----------------------------------------------------------*/
// typedef enum
// {
// 	FAILED = 0,
// 	PASSED = !FAILED
// } TestStatus;

// /* Private define ------------------------------------------------------------*/
// #define BLOCK_SIZE 512 /* Block Size in Bytes */

// #define NUMBER_OF_BLOCKS  10 /* For Multi Blocks operation (Read/Write) */
// #define MULTI_BUFFER_SIZE (BLOCK_SIZE * NUMBER_OF_BLOCKS)

// /* Private macro -------------------------------------------------------------*/
// /* Private variables ---------------------------------------------------------*/
// uint8_t Buffer_Block_Tx[BLOCK_SIZE], Buffer_Block_Rx[BLOCK_SIZE];
// uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
// volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
// SD_Error Status = SD_RESPONSE_NO_ERROR;

// /* Private function prototypes -----------------------------------------------*/
// static void SD_SingleBlockTest(void);
// void SD_MultiBlockTest(void);
// static void Fill_Buffer(uint8_t* pBuffer, uint32_t BufferLength, uint32_t Offset);
// static TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);

// /* Private functions ---------------------------------------------------------*/

// void SD_Test(void)
// {
// 	__disable_irq();
// 	if ((Status = SD_Init()) != SD_RESPONSE_NO_ERROR) {
// 		leds[0].on();
// 		kprintf("dead sd\n");
// 	}
// 	else {
// 		kprintf("good\n");
// 	}
// 	__enable_irq();

// 	if (Status == SD_RESPONSE_NO_ERROR) {

// 		/*single block ∂¡–¥≤‚ ‘*/
// 		SD_SingleBlockTest();

// 		/*muti block ∂¡–¥≤‚ ‘*/
// 		SD_MultiBlockTest();
// 	}
// }

// /**
//   * @brief  Tests the SD card Single Blocks operations.
//   * @param  None
//   * @retval None
//   */
// void SD_SingleBlockTest(void)
// {
// 	/*------------------- Block Read/Write --------------------------*/
// 	/* Fill the buffer to send */
// 	Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE, 0x320F);

// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		/* Write block of 512 bytes on address 0 */
// 		Status = SD_WriteBlock(Buffer_Block_Tx, 0x00, BLOCK_SIZE);
// 		kprintf("Good\n");
// 		/* Check if the Transfer is finished */
// 	}

// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		/* Read block of 512 bytes from address 0 */
// 		Status = SD_ReadBlock(Buffer_Block_Rx, 0x00, BLOCK_SIZE);
// 		kprintf("Good\n");
// 	}

// 	/* Check the correctness of written data */
// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE);
// 		kprintf("Good\n");
// 	}

// 	if (TransferStatus1 == PASSED)
// 	{
// 		leds[2].on();
// 	}
// 	else
// 	{
// 		leds[1].on();
// 	}
// }

// /**
//   * @brief  Tests the SD card Multiple Blocks operations.
//   * @param  None
//   * @retval None
//   */
// void SD_MultiBlockTest(void)
// {
// 	/*--------------- Multiple Block Read/Write ---------------------*/
// 	/* Fill the buffer to send */
// 	Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		/* Write multiple block of many bytes on address 0 */
// 		Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
// 		kprintf("Good\n");

// 		/* Check if the Transfer is finished */
// 	}

// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		/* Read block of many bytes from address 0 */
// 		Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
// 		kprintf("Good\n");

// 		/* Check if the Transfer is finished */
// 	}

// 	/* Check the correctness of written data */
// 	if (Status == SD_RESPONSE_NO_ERROR)
// 	{
// 		TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
// 		kprintf("Good\n");
// 	}

// 	if (TransferStatus2 == PASSED)
// 	{
// 		leds[2].on();
// 	}
// 	else
// 	{
// 		leds[1].on();
// 	}
// }

// /**
//   * @brief  Compares two buffers.
//   * @param  pBuffer1, pBuffer2: buffers to be compared.
//   * @param  BufferLength: buffer's length
//   * @retval PASSED: pBuffer1 identical to pBuffer2
//   *         FAILED: pBuffer1 differs from pBuffer2
//   */
// TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
// {
// 	while (BufferLength--)
// 	{
// 		if (*pBuffer1 != *pBuffer2)
// 		{
// 			return FAILED;
// 		}

// 		pBuffer1++;
// 		pBuffer2++;
// 	}

// 	return PASSED;
// }

// /**
//   * @brief  Fills buffer with user predefined data.
//   * @param  pBuffer: pointer on the Buffer to fill
//   * @param  BufferLength: size of the buffer to fill
//   * @param  Offset: first value to fill on the Buffer
//   * @retval None
//   */
// void Fill_Buffer(uint8_t* pBuffer, uint32_t BufferLength, uint32_t Offset)
// {
// 	uint16_t index = 0;

// 	/* Put in global buffer same values */
// 	for (index = 0; index < BufferLength; index++)
// 	{
// 		pBuffer[index] = index + Offset;
// 	}
// }

// /*********************************************END OF FILE**********************/
