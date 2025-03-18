/** 
 ******************************************************************************
* @file    lan8742.c
* @author  MCD Application Team
* @brief   This file provides a set of functions needed to manage the LAN742
*          PHY devices.
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/  

/* Includes ------------------------------------------------------------------*/
#include "LAN8742.hpp"

using namespace ymd::drivers;

#define LAN8742_SW_RESET_TO    ((uint32_t)500U)
#define LAN8742_INIT_TO        ((uint32_t)2000U)
#define LAN8742_MAX_DEV_ADDR   ((uint32_t)31U)

#define LAN8742_BCR      ((uint16_t)0x0000U)
#define LAN8742_BSR      ((uint16_t)0x0001U)
#define LAN8742_PHYI1R   ((uint16_t)0x0002U)
#define LAN8742_PHYI2R   ((uint16_t)0x0003U)
#define LAN8742_ANAR     ((uint16_t)0x0004U)
#define LAN8742_ANLPAR   ((uint16_t)0x0005U)
#define LAN8742_ANER     ((uint16_t)0x0006U)
#define LAN8742_ANNPTR   ((uint16_t)0x0007U)
#define LAN8742_ANNPRR   ((uint16_t)0x0008U)
#define LAN8742_MMDACR   ((uint16_t)0x000DU)
#define LAN8742_MMDAADR  ((uint16_t)0x000EU)
#define LAN8742_ENCTR    ((uint16_t)0x0010U)
#define LAN8742_MCSR     ((uint16_t)0x0011U)
#define LAN8742_SMR      ((uint16_t)0x0012U)
#define LAN8742_TPDCR    ((uint16_t)0x0018U)
#define LAN8742_TCSR     ((uint16_t)0x0019U)
#define LAN8742_SECR     ((uint16_t)0x001AU)
#define LAN8742_SCSIR    ((uint16_t)0x001BU)
#define LAN8742_CLR      ((uint16_t)0x001CU)
#define LAN8742_ISFR     ((uint16_t)0x001DU)
#define LAN8742_IMR      ((uint16_t)0x001EU)
#define LAN8742_PHYSCSR  ((uint16_t)0x001FU)

#define LAN8742_BCR_SOFT_RESET         ((uint16_t)0x8000U)
#define LAN8742_BCR_LOOPBACK           ((uint16_t)0x4000U)
#define LAN8742_BCR_SPEED_SELECT       ((uint16_t)0x2000U)
#define LAN8742_BCR_AUTONEGO_EN        ((uint16_t)0x1000U)
#define LAN8742_BCR_POWER_DOWN         ((uint16_t)0x0800U)
#define LAN8742_BCR_ISOLATE            ((uint16_t)0x0400U)
#define LAN8742_BCR_RESTART_AUTONEGO   ((uint16_t)0x0200U)
#define LAN8742_BCR_DUPLEX_MODE        ((uint16_t)0x0100U) 

#define LAN8742_BSR_100BASE_T4       ((uint16_t)0x8000U)
#define LAN8742_BSR_100BASE_TX_FD    ((uint16_t)0x4000U)
#define LAN8742_BSR_100BASE_TX_HD    ((uint16_t)0x2000U)
#define LAN8742_BSR_10BASE_T_FD      ((uint16_t)0x1000U)
#define LAN8742_BSR_10BASE_T_HD      ((uint16_t)0x0800U)
#define LAN8742_BSR_100BASE_T2_FD    ((uint16_t)0x0400U)
#define LAN8742_BSR_100BASE_T2_HD    ((uint16_t)0x0200U)
#define LAN8742_BSR_EXTENDED_STATUS  ((uint16_t)0x0100U)
#define LAN8742_BSR_AUTONEGO_CPLT    ((uint16_t)0x0020U)
#define LAN8742_BSR_REMOTE_FAULT     ((uint16_t)0x0010U)
#define LAN8742_BSR_AUTONEGO_ABILITY ((uint16_t)0x0008U)
#define LAN8742_BSR_LINK_STATUS      ((uint16_t)0x0004U)
#define LAN8742_BSR_JABBER_DETECT    ((uint16_t)0x0002U)
#define LAN8742_BSR_EXTENDED_CAP     ((uint16_t)0x0001U)

#define LAN8742_PHYI1R_OUI_3_18           ((uint16_t)0xFFFFU)

#define LAN8742_PHYI2R_OUI_19_24          ((uint16_t)0xFC00U)
#define LAN8742_PHYI2R_MODEL_NBR          ((uint16_t)0x03F0U)
#define LAN8742_PHYI2R_REVISION_NBR       ((uint16_t)0x000FU)

#define LAN8742_ANAR_NEXT_PAGE               ((uint16_t)0x8000U)
#define LAN8742_ANAR_REMOTE_FAULT            ((uint16_t)0x2000U)
#define LAN8742_ANAR_PAUSE_OPERATION         ((uint16_t)0x0C00U)
#define LAN8742_ANAR_PO_NOPAUSE              ((uint16_t)0x0000U)
#define LAN8742_ANAR_PO_SYMMETRIC_PAUSE      ((uint16_t)0x0400U)
#define LAN8742_ANAR_PO_ASYMMETRIC_PAUSE     ((uint16_t)0x0800U)
#define LAN8742_ANAR_PO_ADVERTISE_SUPPORT    ((uint16_t)0x0C00U)
#define LAN8742_ANAR_100BASE_TX_FD           ((uint16_t)0x0100U)
#define LAN8742_ANAR_100BASE_TX              ((uint16_t)0x0080U)
#define LAN8742_ANAR_10BASE_T_FD             ((uint16_t)0x0040U)
#define LAN8742_ANAR_10BASE_T                ((uint16_t)0x0020U)
#define LAN8742_ANAR_SELECTOR_FIELD          ((uint16_t)0x000FU)

#define LAN8742_ANLPAR_NEXT_PAGE            ((uint16_t)0x8000U)
#define LAN8742_ANLPAR_REMOTE_FAULT         ((uint16_t)0x2000U)
#define LAN8742_ANLPAR_PAUSE_OPERATION      ((uint16_t)0x0C00U)
#define LAN8742_ANLPAR_PO_NOPAUSE           ((uint16_t)0x0000U)
#define LAN8742_ANLPAR_PO_SYMMETRIC_PAUSE   ((uint16_t)0x0400U)
#define LAN8742_ANLPAR_PO_ASYMMETRIC_PAUSE  ((uint16_t)0x0800U)
#define LAN8742_ANLPAR_PO_ADVERTISE_SUPPORT ((uint16_t)0x0C00U)
#define LAN8742_ANLPAR_100BASE_TX_FD        ((uint16_t)0x0100U)
#define LAN8742_ANLPAR_100BASE_TX           ((uint16_t)0x0080U)
#define LAN8742_ANLPAR_10BASE_T_FD          ((uint16_t)0x0040U)
#define LAN8742_ANLPAR_10BASE_T             ((uint16_t)0x0020U)
#define LAN8742_ANLPAR_SELECTOR_FIELD       ((uint16_t)0x000FU)

#define LAN8742_ANER_RX_NP_LOCATION_ABLE    ((uint16_t)0x0040U)
#define LAN8742_ANER_RX_NP_STORAGE_LOCATION ((uint16_t)0x0020U)
#define LAN8742_ANER_PARALLEL_DETECT_FAULT  ((uint16_t)0x0010U)
#define LAN8742_ANER_LP_NP_ABLE             ((uint16_t)0x0008U)
#define LAN8742_ANER_NP_ABLE                ((uint16_t)0x0004U)
#define LAN8742_ANER_PAGE_RECEIVED          ((uint16_t)0x0002U)
#define LAN8742_ANER_LP_AUTONEG_ABLE        ((uint16_t)0x0001U)

#define LAN8742_ANNPTR_NEXT_PAGE         ((uint16_t)0x8000U)
#define LAN8742_ANNPTR_MESSAGE_PAGE      ((uint16_t)0x2000U)
#define LAN8742_ANNPTR_ACK2              ((uint16_t)0x1000U)
#define LAN8742_ANNPTR_TOGGLE            ((uint16_t)0x0800U)
#define LAN8742_ANNPTR_MESSAGGE_CODE     ((uint16_t)0x07FFU)

#define LAN8742_ANNPTR_NEXT_PAGE         ((uint16_t)0x8000U)
#define LAN8742_ANNPRR_ACK               ((uint16_t)0x4000U)
#define LAN8742_ANNPRR_MESSAGE_PAGE      ((uint16_t)0x2000U)
#define LAN8742_ANNPRR_ACK2              ((uint16_t)0x1000U)
#define LAN8742_ANNPRR_TOGGLE            ((uint16_t)0x0800U)
#define LAN8742_ANNPRR_MESSAGGE_CODE     ((uint16_t)0x07FFU)

#define LAN8742_MMDACR_MMD_FUNCTION       ((uint16_t)0xC000U) 
#define LAN8742_MMDACR_MMD_FUNCTION_ADDR  ((uint16_t)0x0000U)
#define LAN8742_MMDACR_MMD_FUNCTION_DATA  ((uint16_t)0x4000U)
#define LAN8742_MMDACR_MMD_DEV_ADDR       ((uint16_t)0x001FU)

#define LAN8742_ENCTR_TX_ENABLE             ((uint16_t)0x8000U)
#define LAN8742_ENCTR_TX_TIMER              ((uint16_t)0x6000U)
#define LAN8742_ENCTR_TX_TIMER_1S           ((uint16_t)0x0000U)
#define LAN8742_ENCTR_TX_TIMER_768MS        ((uint16_t)0x2000U)
#define LAN8742_ENCTR_TX_TIMER_512MS        ((uint16_t)0x4000U)
#define LAN8742_ENCTR_TX_TIMER_265MS        ((uint16_t)0x6000U)
#define LAN8742_ENCTR_RX_ENABLE             ((uint16_t)0x1000U)
#define LAN8742_ENCTR_RX_MAX_INTERVAL       ((uint16_t)0x0C00U)
#define LAN8742_ENCTR_RX_MAX_INTERVAL_64MS  ((uint16_t)0x0000U)
#define LAN8742_ENCTR_RX_MAX_INTERVAL_256MS ((uint16_t)0x0400U)
#define LAN8742_ENCTR_RX_MAX_INTERVAL_512MS ((uint16_t)0x0800U)
#define LAN8742_ENCTR_RX_MAX_INTERVAL_1S    ((uint16_t)0x0C00U)
#define LAN8742_ENCTR_EX_CROSS_OVER         ((uint16_t)0x0002U)
#define LAN8742_ENCTR_EX_MANUAL_CROSS_OVER  ((uint16_t)0x0001U)

#define LAN8742_MCSR_EDPWRDOWN        ((uint16_t)0x2000U)
#define LAN8742_MCSR_FARLOOPBACK      ((uint16_t)0x0200U)
#define LAN8742_MCSR_ALTINT           ((uint16_t)0x0040U)
#define LAN8742_MCSR_ENERGYON         ((uint16_t)0x0002U)

#define LAN8742_SMR_MODE       ((uint16_t)0x00E0U)
#define LAN8742_SMR_PHY_ADDR   ((uint16_t)0x001FU)

#define LAN8742_TPDCR_DELAY_IN                 ((uint16_t)0x8000U)
#define LAN8742_TPDCR_LINE_BREAK_COUNTER       ((uint16_t)0x7000U)
#define LAN8742_TPDCR_PATTERN_HIGH             ((uint16_t)0x0FC0U)
#define LAN8742_TPDCR_PATTERN_LOW              ((uint16_t)0x003FU)

#define LAN8742_TCSR_TDR_ENABLE           ((uint16_t)0x8000U)
#define LAN8742_TCSR_TDR_AD_FILTER_ENABLE ((uint16_t)0x4000U)
#define LAN8742_TCSR_TDR_CH_CABLE_TYPE    ((uint16_t)0x0600U)
#define LAN8742_TCSR_TDR_CH_CABLE_DEFAULT ((uint16_t)0x0000U)
#define LAN8742_TCSR_TDR_CH_CABLE_SHORTED ((uint16_t)0x0200U)
#define LAN8742_TCSR_TDR_CH_CABLE_OPEN    ((uint16_t)0x0400U)
#define LAN8742_TCSR_TDR_CH_CABLE_MATCH   ((uint16_t)0x0600U)
#define LAN8742_TCSR_TDR_CH_STATUS        ((uint16_t)0x0100U)
#define LAN8742_TCSR_TDR_CH_LENGTH        ((uint16_t)0x00FFU)

#define LAN8742_SCSIR_AUTO_MDIX_ENABLE    ((uint16_t)0x8000U)
#define LAN8742_SCSIR_CHANNEL_SELECT      ((uint16_t)0x2000U)
#define LAN8742_SCSIR_SQE_DISABLE         ((uint16_t)0x0800U)
#define LAN8742_SCSIR_XPOLALITY           ((uint16_t)0x0010U)

#define LAN8742_CLR_CABLE_LENGTH       ((uint16_t)0xF000U)

#define LAN8742_INT_8       ((uint16_t)0x0100U)
#define LAN8742_INT_7       ((uint16_t)0x0080U)
#define LAN8742_INT_6       ((uint16_t)0x0040U)
#define LAN8742_INT_5       ((uint16_t)0x0020U)
#define LAN8742_INT_4       ((uint16_t)0x0010U)
#define LAN8742_INT_3       ((uint16_t)0x0008U)
#define LAN8742_INT_2       ((uint16_t)0x0004U)
#define LAN8742_INT_1       ((uint16_t)0x0002U)

#define LAN8742_PHYSCSR_AUTONEGO_DONE   ((uint16_t)0x1000U)
#define LAN8742_PHYSCSR_HCDSPEEDMASK    ((uint16_t)0x001CU)
#define LAN8742_PHYSCSR_10BT_HD         ((uint16_t)0x0004U)
#define LAN8742_PHYSCSR_10BT_FD         ((uint16_t)0x0014U)
#define LAN8742_PHYSCSR_100BTX_HD       ((uint16_t)0x0008U)
#define LAN8742_PHYSCSR_100BTX_FD       ((uint16_t)0x0018U) 

#define  LAN8742_STATUS_READ_ERROR            ((int32_t)-5)
#define  LAN8742_STATUS_WRITE_ERROR           ((int32_t)-4)
#define  LAN8742_STATUS_ADDRESS_ERROR         ((int32_t)-3)
#define  LAN8742_STATUS_RESET_TIMEOUT         ((int32_t)-2)
#define  LAN8742_STATUS_ERROR                 ((int32_t)-1)
#define  LAN8742_STATUS_OK                    ((int32_t) 0)
#define  LAN8742_STATUS_LINK_DOWN             ((int32_t) 1)
#define  LAN8742_STATUS_100MBITS_FULLDUPLEX   ((int32_t) 2)
#define  LAN8742_STATUS_100MBITS_HALFDUPLEX   ((int32_t) 3)
#define  LAN8742_STATUS_10MBITS_FULLDUPLEX    ((int32_t) 4)
#define  LAN8742_STATUS_10MBITS_HALFDUPLEX    ((int32_t) 5)
#define  LAN8742_STATUS_AUTONEGO_NOTDONE      ((int32_t) 6)

#define  LAN8742_WOL_IT                        LAN8742_INT_8
#define  LAN8742_ENERGYON_IT                   LAN8742_INT_7
#define  LAN8742_AUTONEGO_COMPLETE_IT          LAN8742_INT_6
#define  LAN8742_REMOTE_FAULT_IT               LAN8742_INT_5
#define  LAN8742_LINK_DOWN_IT                  LAN8742_INT_4
#define  LAN8742_AUTONEGO_LP_ACK_IT            LAN8742_INT_3
#define  LAN8742_PARALLEL_DETECTION_FAULT_IT   LAN8742_INT_2
#define  LAN8742_AUTONEGO_PAGE_RECEIVED_IT     LAN8742_INT_1


struct Interrupts{
    uint8_t autonego_page_receivied:1;
    uint8_t parallel_detection_fault:1;
    uint8_t autonego_lp_ack:1;
    uint8_t link_down:1;
    uint8_t remote_fault:1;
    uint8_t autonego_complete:1;
    uint8_t energyon:1;
    uint8_t wol:1;
};

// int32_t LAN8742::init(){
//     uint32_t tickstart = 0, regvalue = 0, addr = 0;
//     int32_t status = LAN8742_STATUS_OK;

//     if(this->Is_Initialized == 0)
//     {

//         this->init();

//         /* for later check */
//         this->DevAddr = LAN8742_MAX_DEV_ADDR + 1;

//         /* Get the device address from special mode register */  
//         for(addr = 0; addr <= LAN8742_MAX_DEV_ADDR; addr ++)
//         {
//         if(this->ReadReg(addr, LAN8742_SMR, &regvalue) < 0)
//         { 
//             status = LAN8742_STATUS_READ_ERROR;
//             /* Can't read from this device address 
//                 continue with next address */
//             continue;
//         }
        
//         if((regvalue & LAN8742_SMR_PHY_ADDR) == addr)
//         {
//             this->DevAddr = addr;
//             status = LAN8742_STATUS_OK;
//             break;
//         }
//         }

//         if(this->DevAddr > LAN8742_MAX_DEV_ADDR)
//         {
//         status = LAN8742_STATUS_ADDRESS_ERROR;
//         }
        
//         /* if device address is matched */
//         if(status == LAN8742_STATUS_OK)
//         {
//         /* set a software reset  */
//         if(this->WriteReg(this->DevAddr, LAN8742_BCR, LAN8742_BCR_SOFT_RESET) >= 0)
//         { 
//             /* get software reset status */
//             if(this->ReadReg(this->DevAddr, LAN8742_BCR, &regvalue) >= 0)
//             { 
//             tickstart = millis();
            
//             /* wait until software reset is done or timeout occured  */
//             while(regvalue & LAN8742_BCR_SOFT_RESET)
//             {
//                 if((millis() - tickstart) <= LAN8742_SW_RESET_TO)
//                 {
//                 if(this->ReadReg(this->DevAddr, LAN8742_BCR, &regvalue) < 0)
//                 { 
//                     status = LAN8742_STATUS_READ_ERROR;
//                     break;
//                 }
//                 }
//                 else
//                 {
//                 status = LAN8742_STATUS_RESET_TIMEOUT;
//                 break;
//                 }
//             } 
//             }
//             else
//             {
//             status = LAN8742_STATUS_READ_ERROR;
//             }
//         }
//         else
//         {
//             status = LAN8742_STATUS_WRITE_ERROR;
//         }
//         }
//     }
        
//     if(status == LAN8742_STATUS_OK)
//     {
//         tickstart =  millis();
        
//         /* Wait for 2s to perform initialization */
//         while((millis() - tickstart) <= LAN8742_INIT_TO)
//         {
//         }
//         this->Is_Initialized = 1;
//     }

//     return status;
// }


int32_t LAN8742::deinit(){
    return LAN8742_STATUS_OK;
}

int32_t LAN8742::disablePowerDownMode(){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) >= 0)
    {
        readval &= ~LAN8742_BCR_POWER_DOWN;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, readval) < 0)
        {
        status =  LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

int32_t LAN8742::enablePowerDownMode(){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) >= 0)
    {
        readval |= LAN8742_BCR_POWER_DOWN;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, readval) < 0)
        {
        status =  LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

int32_t LAN8742::startAutoNego(){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) >= 0)
    {
        readval |= LAN8742_BCR_AUTONEGO_EN;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, readval) < 0)
        {
        status =  LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Get the link state of LAN8742 device.
 * @param  pObj: Pointer to device object. 
 * @param  pLinkState: Pointer to link state
 * @retval LAN8742_STATUS_LINK_DOWN  if link is down
 *         LAN8742_STATUS_AUTONEGO_NOTDONE if Auto nego not completed 
 *         LAN8742_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
 *         LAN8742_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
 *         LAN8742_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
 *         LAN8742_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD       
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::getLinkState(){
    uint32_t readval = 0;

    /* Read Status register  */
    if(this->ReadReg(this->DevAddr, LAN8742_BSR, &readval) < 0)
    {
        return LAN8742_STATUS_READ_ERROR;
    }

    /* Read Status register again */
    if(this->ReadReg(this->DevAddr, LAN8742_BSR, &readval) < 0)
    {
        return LAN8742_STATUS_READ_ERROR;
    }

    if((readval & LAN8742_BSR_LINK_STATUS) == 0)
    {
        /* Return Link Down status */
        return LAN8742_STATUS_LINK_DOWN;    
    }

    /* Check Auto negotiaition */
    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) < 0)
    {
        return LAN8742_STATUS_READ_ERROR;
    }

    if((readval & LAN8742_BCR_AUTONEGO_EN) != LAN8742_BCR_AUTONEGO_EN)
    {
        if(((readval & LAN8742_BCR_SPEED_SELECT) == LAN8742_BCR_SPEED_SELECT) && ((readval & LAN8742_BCR_DUPLEX_MODE) == LAN8742_BCR_DUPLEX_MODE)) 
        {
        return LAN8742_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((readval & LAN8742_BCR_SPEED_SELECT) == LAN8742_BCR_SPEED_SELECT)
        {
        return LAN8742_STATUS_100MBITS_HALFDUPLEX;
        }        
        else if ((readval & LAN8742_BCR_DUPLEX_MODE) == LAN8742_BCR_DUPLEX_MODE)
        {
        return LAN8742_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
        return LAN8742_STATUS_10MBITS_HALFDUPLEX;
        }  		
    }
    else /* Auto Nego enabled */
    {
        if(this->ReadReg(this->DevAddr, LAN8742_PHYSCSR, &readval) < 0)
        {
        return LAN8742_STATUS_READ_ERROR;
        }
        
        /* Check if auto nego not done */
        if((readval & LAN8742_PHYSCSR_AUTONEGO_DONE) == 0)
        {
        return LAN8742_STATUS_AUTONEGO_NOTDONE;
        }
        
        if((readval & LAN8742_PHYSCSR_HCDSPEEDMASK) == LAN8742_PHYSCSR_100BTX_FD)
        {
        return LAN8742_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((readval & LAN8742_PHYSCSR_HCDSPEEDMASK) == LAN8742_PHYSCSR_100BTX_HD)
        {
        return LAN8742_STATUS_100MBITS_HALFDUPLEX;
        }
        else if ((readval & LAN8742_PHYSCSR_HCDSPEEDMASK) == LAN8742_PHYSCSR_10BT_FD)
        {
        return LAN8742_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
        return LAN8742_STATUS_10MBITS_HALFDUPLEX;
        }				
    }
}

/**
 * @brief  Set the link state of LAN8742 device.
 * @param  pObj: Pointer to device object. 
 * @param  pLinkState: link state can be one of the following
 *         LAN8742_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
 *         LAN8742_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
 *         LAN8742_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
 *         LAN8742_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD   
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_ERROR  if parameter error  
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::setLinkState(uint32_t LinkState){
    uint32_t bcrvalue = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &bcrvalue) >= 0)
    {
        /* Disable link config (Auto nego, speed and duplex) */
        bcrvalue &= ~(LAN8742_BCR_AUTONEGO_EN | LAN8742_BCR_SPEED_SELECT | LAN8742_BCR_DUPLEX_MODE);
        
        if(LinkState == LAN8742_STATUS_100MBITS_FULLDUPLEX)
        {
        bcrvalue |= (LAN8742_BCR_SPEED_SELECT | LAN8742_BCR_DUPLEX_MODE);
        }
        else if (LinkState == LAN8742_STATUS_100MBITS_HALFDUPLEX)
        {
        bcrvalue |= LAN8742_BCR_SPEED_SELECT;
        }
        else if (LinkState == LAN8742_STATUS_10MBITS_FULLDUPLEX)
        {
        bcrvalue |= LAN8742_BCR_DUPLEX_MODE;
        }
        else
        {
        /* Wrong link status parameter */
        status = LAN8742_STATUS_ERROR;
        }	
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    if(status == LAN8742_STATUS_OK)
    {
        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, bcrvalue) < 0)
        {
        status = LAN8742_STATUS_WRITE_ERROR;
        }
    }

    return status;
}

/**
 * @brief  Enable loopback mode.
 * @param  pObj: Pointer to device object. 
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::enableLoopbackMode(){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) >= 0)
    {
        readval |= LAN8742_BCR_LOOPBACK;
        
        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, readval) < 0)
        {
        status = LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Disable loopback mode.
 * @param  pObj: Pointer to device object. 
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::disableLoopbackMode(){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_BCR, &readval) >= 0)
    {
        readval &= ~LAN8742_BCR_LOOPBACK;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_BCR, readval) < 0)
        {
        status =  LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Enable IT source.
 * @param  pObj: Pointer to device object. 
 * @param  Interrupt: IT source to be enabled
 *         should be a value or a combination of the following:
 *         LAN8742_WOL_IT                     
 *         LAN8742_ENERGYON_IT                
 *         LAN8742_AUTONEGO_COMPLETE_IT       
 *         LAN8742_REMOTE_FAULT_IT            
 *         LAN8742_LINK_DOWN_IT               
 *         LAN8742_AUTONEGO_LP_ACK_IT         
 *         LAN8742_PARALLEL_DETECTION_FAULT_IT
 *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::enableIT(uint32_t Interrupt){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_IMR, &readval) >= 0)
    {
        readval |= Interrupt;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_IMR, readval) < 0)
        {
        status =  LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Disable IT source.
 * @param  pObj: Pointer to device object. 
 * @param  Interrupt: IT source to be disabled
 *         should be a value or a combination of the following:
 *         LAN8742_WOL_IT                     
 *         LAN8742_ENERGYON_IT                
 *         LAN8742_AUTONEGO_COMPLETE_IT       
 *         LAN8742_REMOTE_FAULT_IT            
 *         LAN8742_LINK_DOWN_IT               
 *         LAN8742_AUTONEGO_LP_ACK_IT         
 *         LAN8742_PARALLEL_DETECTION_FAULT_IT
 *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_READ_ERROR if connot read register
 *         LAN8742_STATUS_WRITE_ERROR if connot write to register
 */
int32_t LAN8742::disableIT(uint32_t Interrupt){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;

    if(this->ReadReg(this->DevAddr, LAN8742_IMR, &readval) >= 0)
    {
        readval &= ~Interrupt;

        /* Apply configuration */
        if(this->WriteReg(this->DevAddr, LAN8742_IMR, readval) < 0)
        {
        status = LAN8742_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Clear IT flag.
 * @param  pObj: Pointer to device object. 
 * @param  Interrupt: IT flag to be cleared
 *         should be a value or a combination of the following:
 *         LAN8742_WOL_IT                     
 *         LAN8742_ENERGYON_IT                
 *         LAN8742_AUTONEGO_COMPLETE_IT       
 *         LAN8742_REMOTE_FAULT_IT            
 *         LAN8742_LINK_DOWN_IT               
 *         LAN8742_AUTONEGO_LP_ACK_IT         
 *         LAN8742_PARALLEL_DETECTION_FAULT_IT
 *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
 * @retval LAN8742_STATUS_OK  if OK
 *         LAN8742_STATUS_READ_ERROR if connot read register
 */
int32_t  LAN8742::clearIT(uint32_t Interrupt){
    uint32_t readval = 0;
    int32_t status = LAN8742_STATUS_OK;  

    if(this->ReadReg(this->DevAddr, LAN8742_ISFR, &readval) < 0)
    {
        status =  LAN8742_STATUS_READ_ERROR;
    }

    return status;
}

/**
 * @brief  Get IT Flag status.
 * @param  pObj: Pointer to device object. 
 * @param  Interrupt: IT Flag to be checked, 
 *         should be a value or a combination of the following:
 *         LAN8742_WOL_IT                     
 *         LAN8742_ENERGYON_IT                
 *         LAN8742_AUTONEGO_COMPLETE_IT       
 *         LAN8742_REMOTE_FAULT_IT            
 *         LAN8742_LINK_DOWN_IT               
 *         LAN8742_AUTONEGO_LP_ACK_IT         
 *         LAN8742_PARALLEL_DETECTION_FAULT_IT
 *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT  
 * @retval 1 IT flag is SET
 *         0 IT flag is RESET
 *         LAN8742_STATUS_READ_ERROR if connot read register
 */
int32_t LAN8742::getITStatus(uint32_t Interrupt){
    uint32_t readval = 0;
    int32_t status = 0;

    if(this->ReadReg(this->DevAddr, LAN8742_ISFR, &readval) >= 0)
    {
        status = ((readval & Interrupt) == Interrupt);
    }
    else
    {
        status = LAN8742_STATUS_READ_ERROR;
    }
        
    return status;
}
