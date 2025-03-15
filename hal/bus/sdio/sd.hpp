/********************************** (C) COPYRIGHT *******************************
* File Name          : sdio.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains the headers of the SDIO.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#pragma once

#include "sys/core/platform.h"

#ifdef ENABLE_SDIO

void SDIO_Clock_Set(uint8_t clkdiv);
void SDIO_Send_Cmd(uint8_t cmdindex,uint8_t waitrsp,uint32_t arg);
void SDIO_Send_Data_Cfg(uint32_t datatimeout,uint32_t datalen,uint8_t blksize,uint8_t dir);


/* SDIO work mode */
#define SD_POLLING_MODE     0  //If there are problems with reading and writing, it is recommended to add SDIO_TRANSFER_CLK_DIV settings.
#define SD_DMA_MODE         1  //If there are problems with reading and writing, it is recommended to add SDIO_TRANSFER_CLK_DIV settings.

/* SD definition */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)


namespace ymd::drivers{
class SD{
public:
    #pragma pack(push, 1)
    struct SD_CSD{
        uint8_t  CSDStruct;
        uint8_t  SysSpecVersion;
        uint8_t  Reserved1;
        uint8_t  TAAC;
        uint8_t  NSAC;
        uint8_t  MaxBusClkFrec;
        uint16_t CardComdClasses;
        uint8_t  RdBlockLen;
        uint8_t  PartBlockRead;
        uint8_t  WrBlockMisalign;
        uint8_t  RdBlockMisalign;
        uint8_t  DSRImpl;
        uint8_t  Reserved2;
        uint32_t DeviceSize;
        uint8_t  MaxRdCurrentVDDMin;
        uint8_t  MaxRdCurrentVDDMax;
        uint8_t  MaxWrCurrentVDDMin;
        uint8_t  MaxWrCurrentVDDMax;
        uint8_t  DeviceSizeMul;
        uint8_t  EraseGrSize;
        uint8_t  EraseGrMul;
        uint8_t  WrProtectGrSize;
        uint8_t  WrProtectGrEnable;
        uint8_t  ManDeflECC;
        uint8_t  WrSpeedFact;
        uint8_t  MaxWrBlockLen;
        uint8_t  WriteBlockPaPartial;
        uint8_t  Reserved3;
        uint8_t  ContentProtectAppli;
        uint8_t  FileFormatGrouop;
        uint8_t  CopyFlag;
        uint8_t  PermWrProtect;
        uint8_t  TempWrProtect;
        uint8_t  FileFormat;
        uint8_t  ECC;
        uint8_t  CSD_CRC;
        uint8_t  Reserved4;
    };   


    /*SD——CID*/
    struct SD_CID{
        uint8_t  ManufacturerID;
        uint16_t OEM_AppliID;
        uint32_t ProdName1;
        uint8_t  ProdName2;
        uint8_t  ProdRev;
        uint32_t ProdSN;
        uint8_t  Reserved1;
        uint16_t ManufactDate;
        uint8_t  CID_CRC;
        uint8_t  Reserved2;
    };

    #pragma pack(pop)

    /*SDIO Err define */
    enum class SD_Error:uint8_t{
        /* special err */
        SD_CMD_CRC_FAIL                    = (1),
        SD_DATA_CRC_FAIL                   = (2),
        SD_CMD_RSP_TIMEOUT                 = (3),
        SD_DATA_TIMEOUT                    = (4),
        SD_TX_UNDERRUN                     = (5),
        SD_RX_OVERRUN                      = (6),
        SD_START_BIT_ERR                   = (7),
        SD_CMD_OUT_OF_RANGE                = (8),
        SD_ADDR_MISALIGNED                 = (9),
        SD_BLOCK_LEN_ERR                   = (10),
        SD_ERASE_SEQ_ERR                   = (11),
        SD_BAD_ERASE_PARAM                 = (12),
        SD_WRITE_PROT_VIOLATION            = (13),
        SD_LOCK_UNLOCK_FAILED              = (14),
        SD_COM_CRC_FAILED                  = (15),
        SD_ILLEGAL_CMD                     = (16),
        SD_CARD_ECC_FAILED                 = (17),
        SD_CC_ERROR                        = (18),
        SD_GENERAL_UNKNOWN_ERROR           = (19),
        SD_STREAM_READ_UNDERRUN            = (20),
        SD_STREAM_WRITE_OVERRUN            = (21),
        SD_CID_CSD_OVERWRITE               = (22),
        SD_WP_ERASE_SKIP                   = (23),
        SD_CARD_ECC_DISABLED               = (24),
        SD_ERASE_RESET                     = (25),
        SD_AKE_SEQ_ERROR                   = (26),
        SD_INVALID_VOLTRANGE               = (27),
        SD_ADDR_OUT_OF_RANGE               = (28),
        SD_SWITCH_ERROR                    = (29),
        SD_SDIO_DISABLED                   = (30),
        SD_SDIO_FUNCTION_BUSY              = (31),
        SD_SDIO_FUNCTION_FAILED            = (32),
        SD_SDIO_UNKNOWN_FUNCTION           = (33),
        /* Standard err */
        SD_INTERNAL_ERROR,
        SD_NOT_CONFIGURED,
        SD_REQUEST_PENDING,
        SD_REQUEST_NOT_APPLICABLE,
        SD_INVALID_PARAMETER,
        SD_UNSUPPORTED_FEATURE,
        SD_UNSUPPORTED_HW,
        SD_ERROR,
        SD_OK = 0
    };

    /* SD statue */
    typedef enum
    {
        SD_CARD_READY                  = ((uint32_t)0x00000001),
        SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
        SD_CARD_STANDBY                = ((uint32_t)0x00000003),
        SD_CARD_TRANSFER               = ((uint32_t)0x00000004),
        SD_CARD_SENDING                = ((uint32_t)0x00000005),
        SD_CARD_RECEIVING              = ((uint32_t)0x00000006),
        SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
        SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
        SD_CARD_ERROR                  = ((uint32_t)0x000000FF)
    }SDCardState;

    /* SD informatiion */
    struct SD_CardInfo{
        SD_CSD SD_csd;
        SD_CID SD_cid;
        uint64_t CardCapacity;
        uint32_t CardBlockSize;
        uint16_t RCA;
        uint8_t CardType;
    };


protected:
    void DMA_Config(const void * mbuf,uint32_t bufsize,uint32_t dir); 
    uint8_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
    uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;
    uint8_t DeviceMode = SD_POLLING_MODE;
    uint8_t StopCondition = 0;
    volatile SD_Error TransferError = SD_Error::SD_OK;
    volatile uint8_t TransferEnd = 0;

    SD_Error SDEnWideBus( uint8_t enx );
    SD_Error IsCardProgramming( uint8_t *pstatus );
    SD_Error FindSCR( uint16_t rca, uint32_t *pscr );

    void enableRcc(const bool en);
public:
    SD_CardInfo cardinfo;

    SD(const SD & other) = delete;
    SD(SD && other) = delete;

    SD_Error Init(void);
    SD_Error PowerON(void);    
    SD_Error PowerOFF(void);
    SD_Error InitializeCards(void);
    SD_Error GetCardInfo(SD_CardInfo & info);
    SD_Error EnableWideBusOperation(uint32_t wmode);
    SD_Error SetDeviceMode(uint32_t mode);
    SD_Error SelectDeselect(uint32_t addr); 
    SD_Error SendStatus(uint32_t *pcardstatus);
    SDCardState GetState(void);


    SD_Error ReadBlock(uint8_t *buf,uint64_t addr,uint16_t blksize);  
    SD_Error readBurstBlocks(uint8_t *buf,uint64_t  addr,uint16_t blksize,uint32_t nblks);  
    SD_Error WriteBlock(const uint8_t *buf,uint64_t addr,  uint16_t blksize);
    SD_Error writeBurstBlocks(const uint8_t *buf,uint64_t addr,uint16_t blksize,uint32_t nblks);
    SD_Error ProcessIRQSrc(void);

    SD_Error ReadDisk(uint8_t * buf,uint32_t sector,uint8_t cnt);
    SD_Error WriteDisk(const uint8_t*buf,uint32_t sector,uint8_t cnt);

    auto type() const {return CardType;}
    auto manufacturer() const {return cardinfo.SD_cid.ManufacturerID;}
    auto rca() const {return cardinfo.RCA;}

    auto blocksize() const{return cardinfo.CardBlockSize;}
    auto capacity() const{return cardinfo.CardCapacity;}

};

OutputStream & operator<<(OutputStream & os, const SD & sd);
}

#endif

