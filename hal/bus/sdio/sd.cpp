/********************************** (C) COPYRIGHT *******************************
* File Name          : sdio.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2023/11/24
* Description        : This file contains the headers of the SDIO.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "sd.hpp"
#include "sys/stream/ostream.hpp"
#include "hal/gpio/port.hpp"

#ifdef ENABLE_SDIO

using namespace ymd;
using namespace ymd::drivers;
using SD_Error = SD::SD_Error;
using SDCardState = SD::SDCardState;


using enum SD_Error;

/*SDIO flag*/
#define SDIO_FLAG_CCRCFAIL                  ((uint32_t)0x00000001)
#define SDIO_FLAG_DCRCFAIL                  ((uint32_t)0x00000002)
#define SDIO_FLAG_CTIMEOUT                  ((uint32_t)0x00000004)
#define SDIO_FLAG_DTIMEOUT                  ((uint32_t)0x00000008)
#define SDIO_FLAG_TXUNDERR                  ((uint32_t)0x00000010)
#define SDIO_FLAG_RXOVERR                   ((uint32_t)0x00000020)
#define SDIO_FLAG_CMDREND                   ((uint32_t)0x00000040)
#define SDIO_FLAG_CMDSENT                   ((uint32_t)0x00000080)
#define SDIO_FLAG_DATAEND                   ((uint32_t)0x00000100)
#define SDIO_FLAG_STBITERR                  ((uint32_t)0x00000200)
#define SDIO_FLAG_DBCKEND                   ((uint32_t)0x00000400)
#define SDIO_FLAG_CMDACT                    ((uint32_t)0x00000800)
#define SDIO_FLAG_TXACT                     ((uint32_t)0x00001000)
#define SDIO_FLAG_RXACT                     ((uint32_t)0x00002000)
#define SDIO_FLAG_TXFIFOHE                  ((uint32_t)0x00004000)
#define SDIO_FLAG_RXFIFOHF                  ((uint32_t)0x00008000)
#define SDIO_FLAG_TXFIFOF                   ((uint32_t)0x00010000)
#define SDIO_FLAG_RXFIFOF                   ((uint32_t)0x00020000)
#define SDIO_FLAG_TXFIFOE                   ((uint32_t)0x00040000)
#define SDIO_FLAG_RXFIFOE                   ((uint32_t)0x00080000)
#define SDIO_FLAG_TXDAVL                    ((uint32_t)0x00100000)
#define SDIO_FLAG_RXDAVL                    ((uint32_t)0x00200000)
#define SDIO_FLAG_SDIOIT                    ((uint32_t)0x00400000)
#define SDIO_FLAG_CEATAEND                  ((uint32_t)0x00800000)

/* SDIO clock */
#define SDIO_INIT_CLK_DIV        0xB2
#define SDIO_TRANSFER_CLK_DIV    0x00  //If communication is abnormal, it is recommended to increase this value.



/* SDIO instruction*/
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)
#define SD_CMD_SET_REL_ADDR                        ((uint8_t)3)
#define SD_CMD_SET_DSR                             ((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define SD_CMD_SEND_CSD                            ((uint8_t)9)
#define SD_CMD_SEND_CID                            ((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11)
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK                     ((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20) 
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23) 
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define SD_CMD_PROG_CID                            ((uint8_t)26)
#define SD_CMD_PROG_CSD                            ((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((uint8_t)32)
#define SD_CMD_SD_ERASE_GRP_END                    ((uint8_t)33)
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35)
#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36)
#define SD_CMD_ERASE                               ((uint8_t)38)
#define SD_CMD_FAST_IO                             ((uint8_t)39)
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40)
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)
#define SD_CMD_APP_CMD                             ((uint8_t)55)
#define SD_CMD_GEN_CMD                             ((uint8_t)56)
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/*Following commands are SD Card Specific commands.
  SDIO_APP_CMD ：CMD55 should be sent before sending these commands.
*/
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)
#define SD_CMD_SD_APP_STAUS                        ((uint8_t)13)
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22)
#define SD_CMD_SD_APP_OP_COND                      ((uint8_t)41)
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42)
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51)
#define SD_CMD_SDIO_RW_DIRECT                      ((uint8_t)52)
#define SD_CMD_SDIO_RW_EXTENDED                    ((uint8_t)53)

/*Following commands are SD Card Specific security commands.
  SDIO_APP_CMD should be sent before sending these commands.
*/
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43)
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44)
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45)
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46)
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47)
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48)
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18)
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25)
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38)
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49)
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48)


#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

/* SDIO parameter definition */
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)
#define SDIO_DATATIMEOUT                ((uint32_t)0xFFFFFFFF)
#define SDIO_FIFO_Address               ((uint32_t)0x40018080)

/* Mask for errors Card Status R1 (OCR Register) */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/* Masks for R6 Response */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)
#define SD_VOLTAGE_WINDOW_MMC           ((uint32_t)0x80FF8000)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)
#define SD_CARD_PROGRAMMING             ((uint32_t)0x00000007)
#define SD_CARD_RECEIVING               ((uint32_t)0x00000006)
#define SD_DATATIMEOUT                  ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/*Command Class Supported */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/*CMD8*/
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;

SD_Error CmdError( void );
SD_Error CmdResp7Error( void );
SD_Error CmdResp1Error( uint8_t cmd );
SD_Error CmdResp3Error( void );
SD_Error CmdResp2Error( void );
SD_Error CmdResp6Error( uint8_t cmd, uint16_t *prca );

void SDIO_Clock_Set( uint8_t clkdiv )
{
    uint32_t tmpreg = SDIO->CLKCR;

    tmpreg &= 0XFFFFFF00;
    tmpreg |= clkdiv;
    SDIO->CLKCR = tmpreg;
}


constexpr uint8_t convert_from_bytes_to_power_of_two( uint16_t NumberOfBytes )
{
    uint8_t count = 0;
    while( NumberOfBytes != 1 )
    {
        NumberOfBytes >>= 1;
        count++;
    }
    return count;
}





__attribute__( ( aligned( 4 ) ) ) uint8_t SDIO_DATA_BUFFER[512];


void SD::enableRcc(const bool en){
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, en);
}

SD_Error SD::Init( void ){
    enableRcc(true);

    portC[8].afpp();
    portC[9].afpp();
    portC[10].afpp();
    portC[11].afpp();
    portC[12].afpp();

    portD[2].afpp();

    portD[7].inpu();

    // GPIO_InitTypeDef  GPIO_InitStructure;
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init( GPIOC, &GPIO_InitStructure );

    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init( GPIOD, &GPIO_InitStructure );

    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    // GPIO_Init( GPIOD, &GPIO_InitStructure );

    SDIO_DeInit();

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init( &NVIC_InitStructure );


    uint8_t clkdiv = 0;
    SD_Error err = SD_OK;

    err = PowerON();

    if( err == SD_OK ){
        err = InitializeCards();
    }

    if( err == SD_OK ){
        err = GetCardInfo(cardinfo);
    }

    if( err == SD_OK ){
        err = SelectDeselect( ( uint32_t )( cardinfo.RCA << 16 ) );
    }

    if( err == SD_OK ){
        err = EnableWideBusOperation( 1 );
    }

    if( ( err == SD_OK ) || ( SDIO_MULTIMEDIA_CARD == CardType ) ){
        if( cardinfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 || cardinfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0 ){
            clkdiv = SDIO_TRANSFER_CLK_DIV + 6;
        }else{
            clkdiv = SDIO_TRANSFER_CLK_DIV + 1;
        }
        SDIO_Clock_Set( clkdiv );

        err = SetDeviceMode( SD_POLLING_MODE );
    }
    return err;
}




SD_Error SD::PowerON( void )
{
    uint8_t i = 0;
    SD_Error err = SD_OK;
    uint32_t response = 0, count = 0, validvoltage = 0;
    uint32_t SDType = SD_STD_CAPACITY;

    SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
    SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
    SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
    SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
    SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
    SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
    SDIO_Init( &SDIO_InitStructure );

    SDIO_SetPowerState( SDIO_PowerState_ON );

    SDIO_ClockCmd( ENABLE );

    for( i = 0; i < 74; i++ ){
        SDIO_CmdInitStructure.SDIO_Argument = 0x0;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdError();
        if( err == SD_OK )
        {
            break;
        }
    }
    if( err != SD_OK )
    {
        return err;
    }

    SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp7Error();
    if( err == SD_OK )
    {
        CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;
        SDType = SD_HIGH_CAPACITY;
    }
    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_APP_CMD );

    if( err == SD_OK )
    {
        while( ( !validvoltage ) && ( count < SD_MAX_VOLT_TRIAL ) )
        {
            SDIO_CmdInitStructure.SDIO_Argument = 0x00;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            err = CmdResp1Error( SD_CMD_APP_CMD );
            if( err != SD_OK )
            {
                return err;
            }

            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r3
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            err = CmdResp3Error();

            if( err != SD_OK )
            {
                return err;
            }
            response = SDIO->RESP1;;
            validvoltage = ( ( ( response >> 31 ) == 1 ) ? 1 : 0 );
            count++;
        }
        if( count >= SD_MAX_VOLT_TRIAL )
        {
            err = SD_INVALID_VOLTRANGE;
            return err;
        }
        if( response &= SD_HIGH_CAPACITY )
        {
            CardType = SDIO_HIGH_CAPACITY_SD_CARD;
        }
    }
    else
    {
        while( ( !validvoltage ) && ( count < SD_MAX_VOLT_TRIAL ) )
        {
            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r3
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            err = CmdResp3Error();
            if( err != SD_OK )
            {
                return err;
            }
            response = SDIO->RESP1;;
            validvoltage = ( ( ( response >> 31 ) == 1 ) ? 1 : 0 );
            count++;
        }
        if( count >= SD_MAX_VOLT_TRIAL )
        {
            err = SD_INVALID_VOLTRANGE;
            return err;
        }
        CardType = SDIO_MULTIMEDIA_CARD;
    }
    return( err );
}


SD_Error SD::PowerOFF( void )
{
    SDIO_SetPowerState( SDIO_PowerState_OFF );
    return SD_OK;
}


SD_Error SD::InitializeCards( void )
{
    SD_Error err = SD_OK;
    uint16_t rca = 0x01;
    if( SDIO_GetPowerState() == 0 )
    {
        return SD_REQUEST_NOT_APPLICABLE;
    }
    if( SDIO_SECURE_DIGITAL_IO_CARD != CardType )
    {
        SDIO_CmdInitStructure.SDIO_Argument = 0x0;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp2Error();
        if( err != SD_OK )
        {
            return err;
        }
        CID_Tab[0] = SDIO->RESP1;
        CID_Tab[1] = SDIO->RESP2;
        CID_Tab[2] = SDIO->RESP3;
        CID_Tab[3] = SDIO->RESP4;
    }
    if( ( SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType ) || ( SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType ) || ( SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType ) || ( SDIO_HIGH_CAPACITY_SD_CARD == CardType ) )
    {
        SDIO_CmdInitStructure.SDIO_Argument = 0x00;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp6Error( SD_CMD_SET_REL_ADDR, &rca );
        if( err != SD_OK )
        {
            return err;
        }
    }
    if( SDIO_MULTIMEDIA_CARD == CardType )
    {
        SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )( rca << 16 );
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; //r6
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp2Error();
        if( err != SD_OK )
        {
            return err;
        }
    }
    if( SDIO_SECURE_DIGITAL_IO_CARD != CardType )
    {
        RCA = rca;
        SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )( rca << 16 );
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp2Error();
        if( err != SD_OK )
        {
            return err;
        }
        CSD_Tab[0] = SDIO->RESP1;
        CSD_Tab[1] = SDIO->RESP2;
        CSD_Tab[2] = SDIO->RESP3;
        CSD_Tab[3] = SDIO->RESP4;
    }
    return SD_OK;
}


SD_Error SD::GetCardInfo(SD_CardInfo  & info)
{
    SD_Error err = SD_OK;
    uint8_t tmp = 0;
    info.CardType = ( uint8_t )CardType;
    info.RCA = ( uint16_t )RCA;
    tmp = ( uint8_t )( ( CSD_Tab[0] & 0xFF000000 ) >> 24 );
    info.SD_csd.CSDStruct = ( tmp & 0xC0 ) >> 6;
    info.SD_csd.SysSpecVersion = ( tmp & 0x3C ) >> 2;
    info.SD_csd.Reserved1 = tmp & 0x03;
    tmp = ( uint8_t )( ( CSD_Tab[0] & 0x00FF0000 ) >> 16 );
    info.SD_csd.TAAC = tmp;
    tmp = ( uint8_t )( ( CSD_Tab[0] & 0x0000FF00 ) >> 8 );
    info.SD_csd.NSAC = tmp;
    tmp = ( uint8_t )( CSD_Tab[0] & 0x000000FF );
    info.SD_csd.MaxBusClkFrec = tmp;
    tmp = ( uint8_t )( ( CSD_Tab[1] & 0xFF000000 ) >> 24 );
    info.SD_csd.CardComdClasses = tmp << 4;
    tmp = ( uint8_t )( ( CSD_Tab[1] & 0x00FF0000 ) >> 16 );
    info.SD_csd.CardComdClasses |= ( tmp & 0xF0 ) >> 4;
    info.SD_csd.RdBlockLen = tmp & 0x0F;
    tmp = ( uint8_t )( ( CSD_Tab[1] & 0x0000FF00 ) >> 8 );
    info.SD_csd.PartBlockRead = ( tmp & 0x80 ) >> 7;
    info.SD_csd.WrBlockMisalign = ( tmp & 0x40 ) >> 6;
    info.SD_csd.RdBlockMisalign = ( tmp & 0x20 ) >> 5;
    info.SD_csd.DSRImpl = ( tmp & 0x10 ) >> 4;
    info.SD_csd.Reserved2 = 0;
    if( ( CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 ) || ( CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0 ) || ( SDIO_MULTIMEDIA_CARD == CardType ) ) 
    {
        info.SD_csd.DeviceSize = ( tmp & 0x03 ) << 10;
        tmp = ( uint8_t )( CSD_Tab[1] & 0x000000FF );
        info.SD_csd.DeviceSize |= ( tmp ) << 2;
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0xFF000000 ) >> 24 );
        info.SD_csd.DeviceSize |= ( tmp & 0xC0 ) >> 6;
        info.SD_csd.MaxRdCurrentVDDMin = ( tmp & 0x38 ) >> 3;
        info.SD_csd.MaxRdCurrentVDDMax = ( tmp & 0x07 );
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0x00FF0000 ) >> 16 );
        info.SD_csd.MaxWrCurrentVDDMin = ( tmp & 0xE0 ) >> 5;
        info.SD_csd.MaxWrCurrentVDDMax = ( tmp & 0x1C ) >> 2;
        info.SD_csd.DeviceSizeMul = ( tmp & 0x03 ) << 1;
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0x0000FF00 ) >> 8 );
        info.SD_csd.DeviceSizeMul |= ( tmp & 0x80 ) >> 7;
        info.CardCapacity = ( info.SD_csd.DeviceSize + 1 );
        info.CardCapacity *= ( 1 << ( info.SD_csd.DeviceSizeMul + 2 ) );
        info.CardBlockSize = 1 << ( info.SD_csd.RdBlockLen );
        info.CardCapacity *= info.CardBlockSize;
    }
    else if( CardType == SDIO_HIGH_CAPACITY_SD_CARD )
    {
        tmp = ( uint8_t )( CSD_Tab[1] & 0x000000FF );
        info.SD_csd.DeviceSize = ( tmp & 0x3F ) << 16;
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0xFF000000 ) >> 24 );
        info.SD_csd.DeviceSize |= ( tmp << 8 );
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0x00FF0000 ) >> 16 );
        info.SD_csd.DeviceSize |= ( tmp );
        tmp = ( uint8_t )( ( CSD_Tab[2] & 0x0000FF00 ) >> 8 );
        info.CardCapacity = ( uint64_t )( info.SD_csd.DeviceSize + 1 ) * 512 * 1024;
        info.CardBlockSize = 512;
    }
    info.SD_csd.EraseGrSize = ( tmp & 0x40 ) >> 6;
    info.SD_csd.EraseGrMul = ( tmp & 0x3F ) << 1;
    tmp = ( uint8_t )( CSD_Tab[2] & 0x000000FF );
    info.SD_csd.EraseGrMul |= ( tmp & 0x80 ) >> 7;
    info.SD_csd.WrProtectGrSize = ( tmp & 0x7F );
    tmp = ( uint8_t )( ( CSD_Tab[3] & 0xFF000000 ) >> 24 );
    info.SD_csd.WrProtectGrEnable = ( tmp & 0x80 ) >> 7;
    info.SD_csd.ManDeflECC = ( tmp & 0x60 ) >> 5;
    info.SD_csd.WrSpeedFact = ( tmp & 0x1C ) >> 2;
    info.SD_csd.MaxWrBlockLen = ( tmp & 0x03 ) << 2;
    tmp = ( uint8_t )( ( CSD_Tab[3] & 0x00FF0000 ) >> 16 );
    info.SD_csd.MaxWrBlockLen |= ( tmp & 0xC0 ) >> 6;
    info.SD_csd.WriteBlockPaPartial = ( tmp & 0x20 ) >> 5;
    info.SD_csd.Reserved3 = 0;
    info.SD_csd.ContentProtectAppli = ( tmp & 0x01 );
    tmp = ( uint8_t )( ( CSD_Tab[3] & 0x0000FF00 ) >> 8 );
    info.SD_csd.FileFormatGrouop = ( tmp & 0x80 ) >> 7;
    info.SD_csd.CopyFlag = ( tmp & 0x40 ) >> 6;
    info.SD_csd.PermWrProtect = ( tmp & 0x20 ) >> 5;
    info.SD_csd.TempWrProtect = ( tmp & 0x10 ) >> 4;
    info.SD_csd.FileFormat = ( tmp & 0x0C ) >> 2;
    info.SD_csd.ECC = ( tmp & 0x03 );
    tmp = ( uint8_t )( CSD_Tab[3] & 0x000000FF );
    info.SD_csd.CSD_CRC = ( tmp & 0xFE ) >> 1;
    info.SD_csd.Reserved4 = 1;
    tmp = ( uint8_t )( ( CID_Tab[0] & 0xFF000000 ) >> 24 );
    info.SD_cid.ManufacturerID = tmp;
    tmp = ( uint8_t )( ( CID_Tab[0] & 0x00FF0000 ) >> 16 );
    info.SD_cid.OEM_AppliID = tmp << 8;
    tmp = ( uint8_t )( ( CID_Tab[0] & 0x000000FF00 ) >> 8 );
    info.SD_cid.OEM_AppliID |= tmp;
    tmp = ( uint8_t )( CID_Tab[0] & 0x000000FF );
    info.SD_cid.ProdName1 = tmp << 24;
    tmp = ( uint8_t )( ( CID_Tab[1] & 0xFF000000 ) >> 24 );
    info.SD_cid.ProdName1 |= tmp << 16;
    tmp = ( uint8_t )( ( CID_Tab[1] & 0x00FF0000 ) >> 16 );
    info.SD_cid.ProdName1 |= tmp << 8;
    tmp = ( uint8_t )( ( CID_Tab[1] & 0x0000FF00 ) >> 8 );
    info.SD_cid.ProdName1 |= tmp;
    tmp = ( uint8_t )( CID_Tab[1] & 0x000000FF );
    info.SD_cid.ProdName2 = tmp;
    tmp = ( uint8_t )( ( CID_Tab[2] & 0xFF000000 ) >> 24 );
    info.SD_cid.ProdRev = tmp;
    tmp = ( uint8_t )( ( CID_Tab[2] & 0x00FF0000 ) >> 16 );
    info.SD_cid.ProdSN = tmp << 24;
    tmp = ( uint8_t )( ( CID_Tab[2] & 0x0000FF00 ) >> 8 );
    info.SD_cid.ProdSN |= tmp << 16;
    tmp = ( uint8_t )( CID_Tab[2] & 0x000000FF );
    info.SD_cid.ProdSN |= tmp << 8;
    tmp = ( uint8_t )( ( CID_Tab[3] & 0xFF000000 ) >> 24 );
    info.SD_cid.ProdSN |= tmp;
    tmp = ( uint8_t )( ( CID_Tab[3] & 0x00FF0000 ) >> 16 );
    info.SD_cid.Reserved1 |= ( tmp & 0xF0 ) >> 4;
    info.SD_cid.ManufactDate = ( tmp & 0x0F ) << 8;
    tmp = ( uint8_t )( ( CID_Tab[3] & 0x0000FF00 ) >> 8 );
    info.SD_cid.ManufactDate |= tmp;
    tmp = ( uint8_t )( CID_Tab[3] & 0x000000FF );
    info.SD_cid.CID_CRC = ( tmp & 0xFE ) >> 1;
    info.SD_cid.Reserved2 = 1;
    return err;
}


SD_Error SD::EnableWideBusOperation( uint32_t wmode )
{
    SD_Error err = SD_OK;
    if( SDIO_MULTIMEDIA_CARD == CardType )
    {
        return SD_UNSUPPORTED_FEATURE;
    }
    else if( ( SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType ) || ( SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType ) || ( SDIO_HIGH_CAPACITY_SD_CARD == CardType ) )
    {
        if( wmode >= 2 )
        {
            return SD_UNSUPPORTED_FEATURE;
        }
        else
        {
            err = SDEnWideBus( wmode );
            if( SD_OK == err )
            {
                SDIO->CLKCR &= ~( 3 << 11 );
                SDIO->CLKCR |= ( uint16_t )wmode << 11;
                SDIO->CLKCR |= 0 << 14;
            }
        }
    }
    return err;
}


SD_Error SD::SetDeviceMode( uint32_t Mode )
{
    SD_Error err = SD_OK;
    if( ( Mode == SD_DMA_MODE ) || ( Mode == SD_POLLING_MODE ) )
    {
        DeviceMode = Mode;
    }
    else
    {
        err = SD_INVALID_PARAMETER;
    }
    return err;
}


SD_Error SD::SelectDeselect( uint32_t addr )
{
    SDIO_CmdInitStructure.SDIO_Argument =  addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    return CmdResp1Error( SD_CMD_SEL_DESEL_CARD );
}


SD_Error SD::ReadBlock( uint8_t *buf, uint64_t addr, uint16_t blksize )
{
    SD_Error err = SD_OK;
    uint8_t power;
    uint32_t count = 0, *tempbuff = ( uint32_t * )buf;
    uint32_t timeout = SDIO_DATATIMEOUT;
    if( NULL == buf )
    {
        return SD_INVALID_PARAMETER;
    }
    SDIO->DCTRL = 0x0;
    if( CardType == SDIO_HIGH_CAPACITY_SD_CARD )
    {
        blksize = 512;
        addr >>= 9;
    }

    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b ;
    SDIO_DataInitStructure.SDIO_DataLength = 0 ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    if( SDIO->RESP1 & SD_CARD_LOCKED )
    {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if( ( blksize > 0 ) && ( blksize <= 2048 ) && ( ( blksize & ( blksize - 1 ) ) == 0 ) )
    {
        power = convert_from_bytes_to_power_of_two( blksize );

        SDIO_CmdInitStructure.SDIO_Argument =  blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_SET_BLOCKLEN );

        if( err != SD_OK )
        {
            return err;
        }
    }
    else
    {
        return SD_INVALID_PARAMETER;
    }
    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4 ;
    SDIO_DataInitStructure.SDIO_DataLength = blksize ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    SDIO_CmdInitStructure.SDIO_Argument =  addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_READ_SINGLE_BLOCK );
    if( err != SD_OK )
    {
        return err;
    }
    if( DeviceMode == SD_POLLING_MODE )
    {
        while( !( SDIO->STA & ( ( 1 << 5 ) | ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 10 ) | ( 1 << 9 ) ) ) )
        {
            if( SDIO_GetFlagStatus( SDIO_FLAG_RXFIFOHF ) != RESET )
            {
                for( count = 0; count < 8; count++ )
                {
                    *( tempbuff + count ) = SDIO->FIFO;
                }
                tempbuff += 8;
                timeout = 0X7FFFFF;
            }
            else
            {
                if( timeout == 0 )
                {
                    return SD_DATA_TIMEOUT;
                }
                timeout--;
            }
        }
        if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
            return SD_DATA_TIMEOUT;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
            return SD_DATA_CRC_FAIL;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_RXOVERR ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_RXOVERR );
            return SD_RX_OVERRUN;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_STBITERR );
            return SD_START_BIT_ERR;
        }
        while( SDIO_GetFlagStatus( SDIO_FLAG_RXDAVL ) != RESET )
        {
            *tempbuff = SDIO_ReadData();
            tempbuff++;
        }
        SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    }
    else if( DeviceMode == SD_DMA_MODE )
    {
        DMA_Config( ( uint32_t * )buf, blksize, DMA_DIR_PeripheralSRC );
        TransferError = SD_OK;
        StopCondition = 0;
        TransferEnd = 0;
        SDIO->MASK |= ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 5 ) | ( 1 << 9 );
        SDIO_DMACmd( ENABLE );
        DMA_Cmd( DMA2_Channel4, ENABLE );
        while( ( ( DMA2->INTFR & 0X2000 ) == RESET ) && ( TransferEnd == 0 ) && ( TransferError == SD_OK ) && timeout )
        {
            timeout--;
        }
        if( timeout == 0 )
        {
            return SD_DATA_TIMEOUT;
        }
        if( TransferError != SD_OK )
        {
            err = TransferError;
        }
    }
    return err;
}

__attribute__( ( aligned( 4 ) ) ) uint32_t *g_tempbuff;

SD_Error SD::readBurstBlocks( uint8_t *buf, uint64_t addr, uint16_t blksize, uint32_t nblks )
{
    SD_Error err = SD_OK;
    uint8_t power;
    uint32_t count = 0;
    uint32_t timeout = SDIO_DATATIMEOUT;
    g_tempbuff = ( uint32_t * )buf;

    SDIO->DCTRL = 0x0;
    if( CardType == SDIO_HIGH_CAPACITY_SD_CARD )
    {
        blksize = 512;
        addr >>= 9;
    }

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0; ;
    SDIO_DataInitStructure.SDIO_DataLength = 0 ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    if( SDIO->RESP1 & SD_CARD_LOCKED )
    {
        return SD_LOCK_UNLOCK_FAILED;
    }
    if( ( blksize > 0 ) && ( blksize <= 2048 ) && ( ( blksize & ( blksize - 1 ) ) == 0 ) )
    {
        power = convert_from_bytes_to_power_of_two( blksize );

        SDIO_CmdInitStructure.SDIO_Argument =  blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_SET_BLOCKLEN );
        if( err != SD_OK )
        {
            return err;
        }
    }
    else
    {
        return SD_INVALID_PARAMETER;
    }
    if( nblks > 1 )
    {
        if( nblks * blksize > SD_MAX_DATA_LENGTH )
        {
            return SD_INVALID_PARAMETER;
        }
        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; ;
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize ;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig( &SDIO_DataInitStructure );

        SDIO_CmdInitStructure.SDIO_Argument =  addr;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_READ_MULT_BLOCK );
        if( err != SD_OK )
        {
            return err;
        }
        if( DeviceMode == SD_POLLING_MODE )
        {
            while( !( SDIO->STA & ( ( 1 << 5 ) | ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 9 ) ) ) )
            {
                if( SDIO_GetFlagStatus( SDIO_FLAG_RXFIFOHF ) != RESET )
                {
                    for( count = 0; count < 8; count++ )
                    {
                        *( g_tempbuff + count ) = SDIO->FIFO;
                    }
                    g_tempbuff += 8;
                    timeout = 0X7FFFFF;
                }
                else
                {
                    if( timeout == 0 )
                    {
                        return SD_DATA_TIMEOUT;
                    }
                    timeout--;
                }
            }
            if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
                return SD_DATA_TIMEOUT;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
                return SD_DATA_CRC_FAIL;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_RXOVERR ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_RXOVERR );
                return SD_RX_OVERRUN;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_STBITERR );
                return SD_START_BIT_ERR;
            }

            while( SDIO_GetFlagStatus( SDIO_FLAG_RXDAVL ) != RESET )
            {
                *g_tempbuff = SDIO_ReadData();
                g_tempbuff++;
            }
            if( SDIO_GetFlagStatus( SDIO_FLAG_DATAEND ) != RESET )
            {
                if( ( SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType ) || ( SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType ) || ( SDIO_HIGH_CAPACITY_SD_CARD == CardType ) )
                {

                    SDIO_CmdInitStructure.SDIO_Argument =  0;
                    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                    SDIO_SendCommand( &SDIO_CmdInitStructure );

                    err = CmdResp1Error( SD_CMD_STOP_TRANSMISSION );
                    if( err != SD_OK )
                    {
                        return err;
                    }
                }
            }
            SDIO_ClearFlag( SDIO_STATIC_FLAGS );
        }
        else if( DeviceMode == SD_DMA_MODE )
        {
            DMA_Config( ( uint32_t * )buf, nblks * blksize, DMA_DIR_PeripheralSRC );
            TransferError = SD_OK;
            StopCondition = 1;
            TransferEnd = 0;
            SDIO->MASK |= ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 5 ) | ( 1 << 9 );
            SDIO->DCTRL |= 1 << 3;
            DMA_Cmd( DMA2_Channel4, ENABLE );
            while( ( ( DMA2->INTFR & 0X2000 ) == RESET ) && timeout )
            {
                timeout--;
            }
            if( timeout == 0 )
            {
                return SD_DATA_TIMEOUT;
            }
            while( ( TransferEnd == 0 ) && ( TransferError == SD_OK ) );
            if( TransferError != SD_OK )
            {
                err = TransferError;
            }
        }
    }
    return err;
}


SD_Error SD::WriteBlock(const uint8_t *buf, uint64_t addr,  uint16_t blksize )
{
    SD_Error err = SD_OK;
    uint8_t  power = 0, cardstate = 0;
    uint32_t timeout = 0, bytestransferred = 0;
    uint32_t cardstatus = 0, count = 0, restwords = 0;
    uint32_t tlen = blksize;
    const uint32_t *tempbuff = (const uint32_t * )buf;

    SDIO->DCTRL = 0x0;

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0; ;
    SDIO_DataInitStructure.SDIO_DataLength = 0 ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );


    if( SDIO->RESP1 & SD_CARD_LOCKED ){
        return SD_LOCK_UNLOCK_FAILED;
    }

    if( CardType == SDIO_HIGH_CAPACITY_SD_CARD ){
        blksize = 512;
        addr >>= 9;
    }

    if( ( blksize > 0 ) && ( blksize <= 2048 ) && ( ( blksize & ( blksize - 1 ) ) == 0 ) ){
        power = convert_from_bytes_to_power_of_two( blksize );

        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_SET_BLOCKLEN );

        if( err != SD_OK ){
            return err;
        }

    }else{
        return SD_INVALID_PARAMETER;
    }

    SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_SEND_STATUS );

    if( err != SD_OK ){
        return err;
    }
    cardstatus = SDIO->RESP1;
    timeout = SD_DATATIMEOUT;

    while( ( ( cardstatus & 0x00000100 ) == 0 ) && ( timeout > 0 ) )
    {
        timeout--;

        SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )RCA << 16;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_SEND_STATUS );
        if( err != SD_OK )
        {
            return err;
        }
        cardstatus = SDIO->RESP1;
    }
    if( timeout == 0 )
    {
        return SD_ERROR;
    }

    SDIO_CmdInitStructure.SDIO_Argument = addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_WRITE_SINGLE_BLOCK );
    if( err != SD_OK )
    {
        return err;
    }
    StopCondition = 0;

    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; ;
    SDIO_DataInitStructure.SDIO_DataLength = blksize ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    timeout = SDIO_DATATIMEOUT;
    if( DeviceMode == SD_POLLING_MODE )
    {
        while( !( SDIO->STA & ( ( 1 << 10 ) | ( 1 << 4 ) | ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 9 ) ) ) )
        {
            if( SDIO_GetFlagStatus( SDIO_FLAG_TXFIFOHE ) != RESET )
            {
                if( ( tlen - bytestransferred ) < SD_HALFFIFOBYTES )
                {
                    restwords = ( ( tlen - bytestransferred ) % 4 == 0 ) ? ( ( tlen - bytestransferred ) / 4 ) : ( ( tlen - bytestransferred ) / 4 + 1 );

                    for( count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4 )
                    {
                        SDIO_WriteData( *tempbuff );
                    }
                }
                else
                {
                    for( count = 0; count < 8; count++ )
                    {
                        SDIO_WriteData( *( tempbuff + count ) );
                    }
                    tempbuff += 8;
                    bytestransferred += 32;
                }
                timeout = 0X3FFFFFFF;
            }
            else
            {
                if( timeout == 0 )
                {
                    return SD_DATA_TIMEOUT;
                }
                timeout--;
            }
        }
        if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
            return SD_DATA_TIMEOUT;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
            return SD_DATA_CRC_FAIL;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_TXUNDERR ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_TXUNDERR );
            return SD_TX_UNDERRUN;
        }
        else if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
        {
            SDIO_ClearFlag( SDIO_FLAG_STBITERR );
            return SD_START_BIT_ERR;
        }

        SDIO->ICR = 0X5FF;
    }
    else if( DeviceMode == SD_DMA_MODE )
    {
        DMA_Config( (const uint32_t * )buf, blksize, DMA_DIR_PeripheralDST );
        TransferError = SD_OK;
        StopCondition = 0;
        TransferEnd = 0;
        SDIO->MASK |= ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 4 ) | ( 1 << 9 );
        SDIO->DCTRL |= 1 << 3;
        DMA_Cmd( DMA2_Channel4, ENABLE );
        while( ( ( DMA2->INTFR & 0X2000 ) == RESET ) && timeout )
        {
            timeout--;
        }
        if( timeout == 0 )
        {
            Init();
            return SD_DATA_TIMEOUT;
        }
        timeout = SDIO_DATATIMEOUT;
        while( ( TransferEnd == 0 ) && ( TransferError == SD_OK ) && timeout )
        {
            timeout--;
        }
        if( timeout == 0 )
        {
            return SD_DATA_TIMEOUT;
        }
        if( TransferError != SD_OK )
        {
            return TransferError;
        }
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    err = IsCardProgramming( &cardstate );
    while( ( err == SD_OK ) && ( ( cardstate == SD_CARD_PROGRAMMING ) || ( cardstate == SD_CARD_RECEIVING ) ) )
    {
        err = IsCardProgramming( &cardstate );
    }
    return err;
}


SD_Error SD::writeBurstBlocks(const uint8_t *buf, uint64_t addr, uint16_t blksize, uint32_t nblks )
{
    SD_Error err = SD_OK;
    uint8_t  power = 0, cardstate = 0;
    uint32_t timeout = 0, bytestransferred = 0;
    uint32_t count = 0, restwords = 0;
    uint32_t tlen = nblks * blksize;
    const uint32_t *tempbuff = (const uint32_t *)buf;
    if( buf == NULL )
    {
        return SD_INVALID_PARAMETER;
    }
    SDIO->DCTRL = 0x0;

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    SDIO_DataInitStructure.SDIO_DataLength = 0 ;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    if( SDIO->RESP1 & SD_CARD_LOCKED )
    {
        return SD_LOCK_UNLOCK_FAILED;
    }
    if( CardType == SDIO_HIGH_CAPACITY_SD_CARD )
    {
        blksize = 512;
        addr >>= 9;
    }
    if( ( blksize > 0 ) && ( blksize <= 2048 ) && ( ( blksize & ( blksize - 1 ) ) == 0 ) )
    {
        power = convert_from_bytes_to_power_of_two( blksize );

        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_SET_BLOCKLEN );

        if( err != SD_OK )
        {
            return err;
        }

    }
    else
    {
        return SD_INVALID_PARAMETER;
    }
    if( nblks > 1 )
    {
        if( nblks * blksize > SD_MAX_DATA_LENGTH )
        {
            return SD_INVALID_PARAMETER;
        }
        if( ( SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType ) || ( SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType ) || ( SDIO_HIGH_CAPACITY_SD_CARD == CardType ) )
        {
            SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )RCA << 16;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            err = CmdResp1Error( SD_CMD_APP_CMD );

            if( err != SD_OK )
            {
                return err;
            }

            SDIO_CmdInitStructure.SDIO_Argument = nblks;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            err = CmdResp1Error( SD_CMD_SET_BLOCK_COUNT );

            if( err != SD_OK )
            {
                return err;
            }

        }

        SDIO_CmdInitStructure.SDIO_Argument = addr;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_WRITE_MULT_BLOCK );

        if( err != SD_OK )
        {
            return err;
        }

        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; ;
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize ;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT ;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig( &SDIO_DataInitStructure );

        if( DeviceMode == SD_POLLING_MODE )
        {
            timeout = SDIO_DATATIMEOUT;

            while( !( SDIO->STA & ( ( 1 << 4 ) | ( 1 << 1 ) | ( 1 << 8 ) | ( 1 << 3 ) | ( 1 << 9 ) ) ) )
            {
                if( SDIO->STA & ( 1 << 14 ) )
                {
                    if( ( tlen - bytestransferred ) < SD_HALFFIFOBYTES )
                    {
                        restwords = ( ( tlen - bytestransferred ) % 4 == 0 ) ? ( ( tlen - bytestransferred ) / 4 ) : ( ( tlen - bytestransferred ) / 4 + 1 );
                        for( count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4 )
                        {
                            SDIO_WriteData( *tempbuff );
                        }
                    }
                    else
                    {
                        for( count = 0; count < SD_HALFFIFO; count++ )
                        {
                            SDIO_WriteData( *( tempbuff + count ) );
                        }
                        tempbuff += SD_HALFFIFO;
                        bytestransferred += SD_HALFFIFOBYTES;
                    }
                    timeout = 0X3FFFFFFF;
                }
                else
                {
                    if( timeout == 0 )
                    {
                        return SD_DATA_TIMEOUT;
                    }
                    timeout--;
                }
            }
            if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
                return SD_DATA_TIMEOUT;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
                return SD_DATA_CRC_FAIL;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_TXUNDERR ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_TXUNDERR );
                return SD_TX_UNDERRUN;
            }
            else if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
            {
                SDIO_ClearFlag( SDIO_FLAG_STBITERR );
                return SD_START_BIT_ERR;
            }
            if( SDIO_GetFlagStatus( SDIO_FLAG_DATAEND ) != RESET )
            {
                if( ( SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType ) || ( SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType ) || ( SDIO_HIGH_CAPACITY_SD_CARD == CardType ) )
                {
                    SDIO_CmdInitStructure.SDIO_Argument = 0;
                    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                    SDIO_SendCommand( &SDIO_CmdInitStructure );

                    err = CmdResp1Error( SD_CMD_STOP_TRANSMISSION );
                    if( err != SD_OK )
                    {
                        return err;
                    }
                }
            }
            SDIO_ClearFlag( SDIO_STATIC_FLAGS );
        }
        else if( DeviceMode == SD_DMA_MODE )
        {
            DMA_Config(buf, nblks * blksize, DMA_DIR_PeripheralDST );
            TransferError = SD_OK;
            StopCondition = 1;
            TransferEnd = 0;
            SDIO->MASK |= ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 4 ) | ( 1 << 9 );
            SDIO->DCTRL |= 1 << 3;
            DMA_Cmd( DMA2_Channel4, ENABLE );
            timeout = SDIO_DATATIMEOUT;
            while( ( ( DMA2->INTFR & 0X2000 ) == RESET ) && timeout )
            {
                timeout--;
            }
            if( timeout == 0 )
            {
                Init();
                return SD_DATA_TIMEOUT;
            }
            timeout = SDIO_DATATIMEOUT;
            while( ( TransferEnd == 0 ) && ( TransferError == SD_OK ) && timeout )
            {
                timeout--;
            }
            if( timeout == 0 )
            {
                return SD_DATA_TIMEOUT;
            }
            if( TransferError != SD_OK )
            {
                return TransferError;
            }
        }
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    err = IsCardProgramming( &cardstate );
    while( ( err == SD_OK ) && ( ( cardstate == SD_CARD_PROGRAMMING ) || ( cardstate == SD_CARD_RECEIVING ) ) )
    {
        err = IsCardProgramming( &cardstate );
    }
    return err;
}
void SDIO_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      SDIO_IRQHandler
 *
 * @brief   This function handles SDIO exception.
 *
 * @return  None
 */
void SDIO_IRQHandler( void )
{
    // SD_ProcessIRQSrc();
    //TODO
}


SD_Error SD::ProcessIRQSrc( void )
{
    if( SDIO->STA & ( 1 << 8 ) )
    {
        if( StopCondition == 1 )
        {
            SDIO_CmdInitStructure.SDIO_Argument = 0;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand( &SDIO_CmdInitStructure );

            TransferError = CmdResp1Error( SD_CMD_STOP_TRANSMISSION );
        }
        else
        {
            TransferError = SD_OK;
        }
        SDIO->ICR |= 1 << 8;
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferEnd = 1;
        return( TransferError );
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferError = SD_DATA_CRC_FAIL;
        return( SD_DATA_CRC_FAIL );
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferError = SD_DATA_TIMEOUT;
        return( SD_DATA_TIMEOUT );
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_RXOVERR ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_RXOVERR );
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferError = SD_RX_OVERRUN;
        return( SD_RX_OVERRUN );
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_TXUNDERR ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_TXUNDERR );
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferError = SD_TX_UNDERRUN;
        return( SD_TX_UNDERRUN );
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_STBITERR );
        SDIO->MASK &= ~( ( 1 << 1 ) | ( 1 << 3 ) | ( 1 << 8 ) | ( 1 << 14 ) | ( 1 << 15 ) | ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 9 ) );
        TransferError = SD_START_BIT_ERR;
        return( SD_START_BIT_ERR );
    }
    return( SD_OK );
}


SD_Error CmdError( void )
{
    SD_Error err = SD_OK;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while( timeout-- )
    {
        if( SDIO_GetFlagStatus( SDIO_FLAG_CMDSENT ) != RESET )
        {
            break;
        }
    }
    if( timeout == 0 )
    {
        return SD_CMD_RSP_TIMEOUT;
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    return err;
}


SD_Error CmdResp7Error( void )
{
    SD_Error err = SD_OK;
    uint32_t status=0;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while( timeout-- )
    {
        status = SDIO->STA;
        if( status & ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 6 ) ) )
        {
            break;
        }
    }
    if( ( timeout == 0 ) || ( status & ( 1 << 2 ) ) )
    {
        err = SD_CMD_RSP_TIMEOUT;
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return err;
    }
    if( ( status ) & ( 1 << 6 ) )
    {
        err = SD_OK;
        SDIO_ClearFlag( SDIO_FLAG_CMDREND );
    }
    return err;
}


SD_Error CmdResp1Error( uint8_t cmd )
{
    uint32_t status;
    while( 1 )
    {
        status = SDIO->STA;
        if( status & ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 6 ) ) )
        {
            break;
        }
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return SD_CMD_RSP_TIMEOUT;
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CCRCFAIL ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CCRCFAIL );
        return SD_CMD_CRC_FAIL;
    }
    if( SDIO->RESPCMD != cmd )
    {
        return SD_ILLEGAL_CMD;
    }
    SDIO->ICR = 0X5FF;
    return ( SD_Error )( SDIO->RESP1 & SD_OCR_ERRORBITS );
}


SD_Error CmdResp3Error( void )
{
    uint32_t status;
    while( 1 )
    {
        status = SDIO->STA;
        if( status & ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 6 ) ) )
        {
            break;
        }
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return SD_CMD_RSP_TIMEOUT;
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    return SD_OK;
}


SD_Error CmdResp2Error( void )
{
    SD_Error err = SD_OK;
    uint32_t status=0;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while( timeout-- )
    {
        status = SDIO->STA;
        if( status & ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 6 ) ) )
        {
            break;
        }
    }
    if( ( timeout == 0 ) || ( status & ( 1 << 2 ) ) )
    {
        err = SD_CMD_RSP_TIMEOUT;
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return err;
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CCRCFAIL ) != RESET )
    {
        err = SD_CMD_CRC_FAIL;
        SDIO_ClearFlag( SDIO_FLAG_CCRCFAIL );
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );
    return err;
}


SD_Error CmdResp6Error( uint8_t cmd, uint16_t *prca )
{
    SD_Error err = SD_OK;
    uint32_t status=0;
    uint32_t rspr1;
    while( 1 )
    {
        status = SDIO->STA;
        if( status & ( ( 1 << 0 ) | ( 1 << 2 ) | ( 1 << 6 ) ) )
        {
            break;
        }
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return SD_CMD_RSP_TIMEOUT;
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CCRCFAIL ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CCRCFAIL );
        return SD_CMD_CRC_FAIL;
    }
    if( SDIO->RESPCMD != cmd )
    {
        return SD_ILLEGAL_CMD;
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );

    rspr1 = SDIO->RESP1;
    if( SD_ALLZERO == ( rspr1 & ( SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED ) ) )
    {
        *prca = ( uint16_t )( rspr1 >> 16 );
        return err;
    }
    if( rspr1 & SD_R6_GENERAL_UNKNOWN_ERROR )
    {
        return SD_GENERAL_UNKNOWN_ERROR;
    }
    if( rspr1 & SD_R6_ILLEGAL_CMD )
    {
        return SD_ILLEGAL_CMD;
    }
    if( rspr1 & SD_R6_COM_CRC_FAILED )
    {
        return SD_COM_CRC_FAILED;
    }
    return err;
}


SD_Error SD::SDEnWideBus( uint8_t enx )
{
    SD_Error err = SD_OK;
    uint32_t scr[2] = {0, 0};
    uint8_t arg = 0X00;
    if( enx )
    {
        arg = 0X02;
    }
    else
    {
        arg = 0X00;
    }
    if( SDIO->RESP1 & SD_CARD_LOCKED )
    {
        return SD_LOCK_UNLOCK_FAILED;
    }
    err = FindSCR( RCA, scr );
    if( err != SD_OK )
    {
        return err;
    }
    if( ( scr[1]&SD_WIDE_BUS_SUPPORT ) != SD_ALLZERO )
    {
        SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t ) RCA << 16;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_APP_CMD );

        if( err != SD_OK )
        {
            return err;
        }

        SDIO_CmdInitStructure.SDIO_Argument = arg;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand( &SDIO_CmdInitStructure );

        err = CmdResp1Error( SD_CMD_APP_SD_SET_BUSWIDTH );
        return err;
    }
    else
    {
        return SD_REQUEST_NOT_APPLICABLE;
    }
}


SD_Error SD::IsCardProgramming( uint8_t *pstatus )
{
    volatile uint32_t respR1 = 0, status = 0;

    SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t ) RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    status = SDIO->STA;

    while( !( status & ( ( 1 << 0 ) | ( 1 << 6 ) | ( 1 << 2 ) ) ) )
    {
        status = SDIO->STA;
    }

    if( SDIO_GetFlagStatus( SDIO_FLAG_CCRCFAIL ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CCRCFAIL );
        return SD_CMD_CRC_FAIL;
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_CTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_CTIMEOUT );
        return SD_CMD_RSP_TIMEOUT;
    }
    if( SDIO->RESPCMD != SD_CMD_SEND_STATUS )
    {
        return SD_ILLEGAL_CMD;
    }

    SDIO_ClearFlag( SDIO_STATIC_FLAGS );

    respR1 = SDIO->RESP1;

    *pstatus = ( uint8_t )( ( respR1 >> 9 ) & 0x0000000F );
    return SD_OK;
}


SD_Error SD::SendStatus( uint32_t *pcardstatus )
{
    SD_Error err = SD_OK;
    if( pcardstatus == NULL )
    {
        err = SD_INVALID_PARAMETER;
        return err;
    }

    SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t ) RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_SEND_STATUS );
    if( err != SD_OK )
    {
        return err;
    }
    *pcardstatus = SDIO->RESP1;
    return err;
}


SDCardState SD::GetState( void )
{
    uint32_t resp1 = 0;
    if( SendStatus( &resp1 ) != SD_OK )
    {
        return SD_CARD_ERROR;
    }
    else
    {
        return ( SDCardState )( ( resp1 >> 9 ) & 0x0F );
    }
}


SD_Error SD::FindSCR( uint16_t rca, uint32_t *pscr )
{
    uint32_t index = 0;
    SD_Error err = SD_OK;
    uint32_t tempscr[2] = {0, 0};

    SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t )8;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_SET_BLOCKLEN );

    if( err != SD_OK )
    {
        return err;
    }

    SDIO_CmdInitStructure.SDIO_Argument = ( uint32_t ) RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_APP_CMD );

    if( err != SD_OK )
    {
        return err;
    }

    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DataLength = 8;
    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b  ;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataConfig( &SDIO_DataInitStructure );

    SDIO_CmdInitStructure.SDIO_Argument = 0x0;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r1
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand( &SDIO_CmdInitStructure );

    err = CmdResp1Error( SD_CMD_SD_APP_SEND_SCR );

    if( err != SD_OK )
    {
        return err;
    }

    while( !( SDIO->STA & ( SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR ) ) )
    {
        if( SDIO_GetFlagStatus( SDIO_FLAG_RXDAVL ) != RESET )
        {
            *( tempscr + index ) = SDIO_ReadData();
            index++;
            if( index >= 2 )
            {
                break;
            }
        }
    }
    if( SDIO_GetFlagStatus( SDIO_FLAG_DTIMEOUT ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_DTIMEOUT );
        return SD_DATA_TIMEOUT;
    }
    else if( SDIO_GetFlagStatus( SDIO_FLAG_DCRCFAIL ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_DCRCFAIL );
        return SD_DATA_CRC_FAIL;
    }
    else if( SDIO_GetFlagStatus( SDIO_FLAG_RXOVERR ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_RXOVERR );
        return SD_RX_OVERRUN;
    }
    else if( SDIO_GetFlagStatus( SDIO_FLAG_STBITERR ) != RESET )
    {
        SDIO_ClearFlag( SDIO_FLAG_STBITERR );
        return SD_START_BIT_ERR;
    }
    SDIO_ClearFlag( SDIO_STATIC_FLAGS );

    *( pscr + 1 ) = ( ( tempscr[0] & SD_0TO7BITS ) << 24 ) | ( ( tempscr[0] & SD_8TO15BITS ) << 8 ) | ( ( tempscr[0] & SD_16TO23BITS ) >> 8 ) | ( ( tempscr[0] & SD_24TO31BITS ) >> 24 );
    *( pscr ) = ( ( tempscr[1] & SD_0TO7BITS ) << 24 ) | ( ( tempscr[1] & SD_8TO15BITS ) << 8 ) | ( ( tempscr[1] & SD_16TO23BITS ) >> 8 ) | ( ( tempscr[1] & SD_24TO31BITS ) >> 24 );
    return err;
}




void SD::DMA_Config(const void *mbuf, uint32_t bufsize, uint32_t DMA_DIR )
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA2, ENABLE );

    DMA_DeInit( DMA2_Channel4 );
    DMA_Cmd( DMA2_Channel4, DISABLE );

    DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t )&SDIO->FIFO;
    DMA_InitStructure.DMA_MemoryBaseAddr = ( uint32_t )mbuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR;
    DMA_InitStructure.DMA_BufferSize = bufsize / 4;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA2_Channel4, &DMA_InitStructure );

    DMA_Cmd( DMA2_Channel4, ENABLE);
}


SD_Error SD::ReadDisk( uint8_t *buf, uint32_t sector, uint8_t cnt )
{
    auto sta = SD_OK;
    uint64_t lsector = sector;
    uint8_t n;
    lsector <<= 9;
    if( ( uint32_t )buf % 4 != 0 )
    {
        for( n = 0; n < cnt; n++ )
        {
            sta = ReadBlock( SDIO_DATA_BUFFER, lsector + 512 * n, 512 );
            memcpy( buf, SDIO_DATA_BUFFER, 512 );
            buf += 512;
        }
    }
    else
    {
        if( cnt == 1 )
        {
            sta = ReadBlock( buf, lsector, 512 );
        }
        else
        {
            sta = readBurstBlocks( buf, lsector, 512, cnt );
        }
    }
    return sta;
}


SD_Error SD::WriteDisk(const uint8_t *buf, uint32_t sector, uint8_t cnt ){
    auto sta = SD_OK;
    uint8_t n;
    const uint64_t lsector = sector << 9;

    if((uint32_t)buf % 4 != 0 ){
        for(n = 0; n < cnt; n++ ){
            memcpy(SDIO_DATA_BUFFER, buf, 512);
            sta = WriteBlock(SDIO_DATA_BUFFER, lsector + (n << 9), 512);
            buf += 512;
        }
    }else{
        if(cnt == 1){
            sta = WriteBlock(buf, lsector, 512);
        }else{
            sta = writeBurstBlocks(buf, lsector, 512, cnt);
        }
    }
    return sta;
}


OutputStream & operator<<(OutputStream & os, const SD & sd){
    switch(sd.type()){
        case SDIO_STD_CAPACITY_SD_CARD_V1_1:os.println("Card Type:SDSC V1.1");break;
        case SDIO_STD_CAPACITY_SD_CARD_V2_0:os.println("Card Type:SDSC V2.0");break;
        case SDIO_HIGH_CAPACITY_SD_CARD:os.println("Card Type:SDHC V2.0");break;
        case SDIO_MULTIMEDIA_CARD:os.println("Card Type:MMC Card");break;
    }
    os.println("Card ManufacturerID:",sd.manufacturer());
    os.println("Card RCA:",sd.rca());
    os.println("Card Capacity:", (uint32_t)(sd.capacity()>>20), "MB");
    os.println("Card BlockSize:", sd.blocksize());

    return os;
}

#endif