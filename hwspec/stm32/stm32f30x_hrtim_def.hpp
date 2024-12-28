#pragma once

#include <cstdint>

#define __IO volatile
namespace hwspec::STM32F30x::HRTIM_Regs{

struct MCR_Reg{
    uint32_t CKPSC:3;//0~2
    uint32_t CONT:1;//3

    uint32_t RETRIG:1;//4
    uint32_t HALF:1;//5

    uint32_t :2;

    uint32_t SYNC_IN_0:1;//8
    uint32_t SYNC_IN_1:1;//9

    uint32_t SYNCRSTM:1;//10
    uint32_t SYNCSTRTM:1;//11

    uint32_t SYNC_OUT_0:1;//12
    uint32_t SYNC_OUT_1:1;//13

    uint32_t SYNC_SRC_0:1;//14
    uint32_t SYNC_SRC_1:1;//15

    uint32_t MCEN:1;//16
    uint32_t TACEN:1;//17
    uint32_t TBCEN:1;//18
    uint32_t TCCEN:1;//19
    uint32_t TDCEN:1;//20
    uint32_t TECEN:1;//21

    uint32_t :3;//22~24   
    uint32_t DACSYNC_0;//25
    uint32_t DACSYNC_1;//26

    uint32_t PREEN:1;//26
    uint32_t :1;
    uint32_t MREPU:1;//26

    uint32_t BRSTDMA_0:1;//30
    uint32_t BRSTDMA_1:1;//31

    void set_clock_prescale(uint32_t prescale){
        CKPSC = prescale;
    }

    void enable_cont_mode(bool en){
        CONT = en;
    }


};

struct MISR_Reg{
    uint32_t MCMP1:1;
    uint32_t MCMP2:1;
    uint32_t MCMP3:1;
    uint32_t MCMP4:1;
    uint32_t MREP:1;
    uint32_t SYNC:1;
    uint32_t MUPD:1;

    uint32_t :29;
};

struct MICR_Reg{
    uint32_t MCMP1C:1;
    uint32_t MCMP2C:1;
    uint32_t MCMP3C:1;
    uint32_t MCMP4C:1;

    uint32_t MCMPC:1;
    uint32_t SYNCC:1;
    uint32_t MUPDC:1;

    uint32_t :29;
};

struct MDIER_Reg{
    uint32_t MCMP1IE:1;
    uint32_t MCMP2IE:1;
    uint32_t MCMP3IE:1;
    uint32_t MCMP4IE:1;
    uint32_t MREPIE:1;

    uint32_t SYNCIE:1;
    uint32_t MUPDIE:1;

    uint32_t :10;

    uint32_t MCMP1DE:1;
    uint32_t MCMP2DE:1;
    uint32_t MCMP3DE:1;
    uint32_t MCMP4DE:1;
    uint32_t MREPDE:1;

    uint32_t SYNCDE:1;
    uint32_t MUPDDE:1;
    uint32_t :10;
};

struct HRTIM_Master_Def{
    MCR_Reg MCR;            /*!< HRTIM Master Timer control register,                     Address offset: 0x00 */
    MISR_Reg MISR;           /*!< HRTIM Master Timer interrupt status register,            Address offset: 0x04 */
    MICR_Reg MICR;           /*!< HRTIM Master Timer interupt clear register,              Address offset: 0x08 */
    MDIER_Reg MDIER;          /*!< HRTIM Master Timer DMA/interrupt enable register         Address offset: 0x0C */
    volatile uint32_t MCNTR;          /*!< HRTIM Master Timer counter register,                     Address offset: 0x10 */
    volatile uint32_t MPER;           /*!< HRTIM Master Timer period register,                      Address offset: 0x14 */
    volatile uint32_t MREP;           /*!< HRTIM Master Timer repetition register,                  Address offset: 0x18 */
    volatile uint32_t MCMP1R;         /*!< HRTIM Master Timer compare 1 register,                   Address offset: 0x1C */
    uint32_t      RESERVED0;     /*!< Reserved,                                                                0x20 */
    volatile uint32_t MCMP2R;         /*!< HRTIM Master Timer compare 2 register,                   Address offset: 0x24 */
    volatile uint32_t MCMP3R;         /*!< HRTIM Master Timer compare 3 register,                   Address offset: 0x28 */
    volatile uint32_t MCMP4R;         /*!< HRTIM Master Timer compare 4 register,                   Address offset: 0x2C */
    uint32_t      RESERVED1[20];  /*!< Reserved,                                                          0x30..0x7C */
}; 


typedef struct{
    __IO uint32_t TIMxCR;     /*!< HRTIM Timerx control register,                              Address offset: 0x00  */
    __IO uint32_t TIMxISR;    /*!< HRTIM Timerx interrupt status register,                     Address offset: 0x04  */
    __IO uint32_t TIMxICR;    /*!< HRTIM Timerx interrupt clear register,                      Address offset: 0x08  */
    __IO uint32_t TIMxDIER;   /*!< HRTIM Timerx DMA/interrupt enable register,                 Address offset: 0x0C  */
    __IO uint32_t CNTxR;      /*!< HRTIM Timerx counter register,                              Address offset: 0x10  */
    __IO uint32_t PERxR;      /*!< HRTIM Timerx period register,                               Address offset: 0x14  */
    __IO uint32_t REPxR;      /*!< HRTIM Timerx repetition register,                           Address offset: 0x18  */
    __IO uint32_t CMP1xR;     /*!< HRTIM Timerx compare 1 register,                            Address offset: 0x1C  */
    __IO uint32_t CMP1CxR;    /*!< HRTIM Timerx compare 1 compound register,                   Address offset: 0x20  */
    __IO uint32_t CMP2xR;     /*!< HRTIM Timerx compare 2 register,                            Address offset: 0x24  */
    __IO uint32_t CMP3xR;     /*!< HRTIM Timerx compare 3 register,                            Address offset: 0x28  */
    __IO uint32_t CMP4xR;     /*!< HRTIM Timerx compare 4 register,                            Address offset: 0x2C  */
    __IO uint32_t CPT1xR;     /*!< HRTIM Timerx capture 1 register,                            Address offset: 0x30  */
    __IO uint32_t CPT2xR;     /*!< HRTIM Timerx capture 2 register,                            Address offset: 0x34 */
    __IO uint32_t DTxR;       /*!< HRTIM Timerx dead time register,                            Address offset: 0x38 */
    __IO uint32_t SETx1R;     /*!< HRTIM Timerx output 1 set register,                         Address offset: 0x3C */
    __IO uint32_t RSTx1R;     /*!< HRTIM Timerx output 1 reset register,                       Address offset: 0x40 */
    __IO uint32_t SETx2R;     /*!< HRTIM Timerx output 2 set register,                         Address offset: 0x44 */
    __IO uint32_t RSTx2R;     /*!< HRTIM Timerx output 2 reset register,                       Address offset: 0x48 */
    __IO uint32_t EEFxR1;     /*!< HRTIM Timerx external event filtering 1 register,           Address offset: 0x4C */
    __IO uint32_t EEFxR2;     /*!< HRTIM Timerx external event filtering 2 register,           Address offset: 0x50 */
    __IO uint32_t RSTxR;      /*!< HRTIM Timerx Reset register,                                Address offset: 0x54 */
    __IO uint32_t CHPxR;      /*!< HRTIM Timerx Chopper register,                              Address offset: 0x58 */
    __IO uint32_t CPT1xCR;    /*!< HRTIM Timerx Capture 1 register,                            Address offset: 0x5C */
    __IO uint32_t CPT2xCR;    /*!< HRTIM Timerx Capture 2 register,                            Address offset: 0x60 */
    __IO uint32_t OUTxR;      /*!< HRTIM Timerx Output register,                               Address offset: 0x64 */
    __IO uint32_t FLTxR;      /*!< HRTIM Timerx Fault register,                                Address offset: 0x68 */
    uint32_t      RESERVED0[5];  /*!< Reserved,                                                              0x6C..0x7C */
}HRTIM_Timerx_TypeDef;

typedef struct
{
  __IO uint32_t CR1;        /*!< HRTIM control register1,                                    Address offset: 0x00 */
  __IO uint32_t CR2;        /*!< HRTIM control register2,                                    Address offset: 0x04 */
  __IO uint32_t ISR;        /*!< HRTIM interrupt status register,                            Address offset: 0x08 */
  __IO uint32_t ICR;        /*!< HRTIM interrupt clear register,                             Address offset: 0x0C */
  __IO uint32_t IER;        /*!< HRTIM interrupt enable register,                            Address offset: 0x10 */
  __IO uint32_t OENR;       /*!< HRTIM Output enable register,                               Address offset: 0x14 */
  __IO uint32_t ODISR;      /*!< HRTIM Output disable register,                              Address offset: 0x18 */
  __IO uint32_t ODSR;       /*!< HRTIM Output disable status register,                       Address offset: 0x1C */
  __IO uint32_t BMCR;       /*!< HRTIM Burst mode control register,                          Address offset: 0x20 */
  __IO uint32_t BMTRGR;     /*!< HRTIM Busrt mode trigger register,                          Address offset: 0x24 */
  __IO uint32_t BMCMPR;     /*!< HRTIM Burst mode compare register,                          Address offset: 0x28 */
  __IO uint32_t BMPER;      /*!< HRTIM Burst mode period register,                           Address offset: 0x2C */
  __IO uint32_t EECR1;      /*!< HRTIM Timer external event control register1,               Address offset: 0x30 */
  __IO uint32_t EECR2;      /*!< HRTIM Timer external event control register2,               Address offset: 0x34 */
  __IO uint32_t EECR3;      /*!< HRTIM Timer external event control register3,               Address offset: 0x38 */
  __IO uint32_t ADC1R;      /*!< HRTIM ADC Trigger 1 register,                               Address offset: 0x3C */
  __IO uint32_t ADC2R;      /*!< HRTIM ADC Trigger 2 register,                               Address offset: 0x40 */
  __IO uint32_t ADC3R;      /*!< HRTIM ADC Trigger 3 register,                               Address offset: 0x44 */
  __IO uint32_t ADC4R;      /*!< HRTIM ADC Trigger 4 register,                               Address offset: 0x48 */
  __IO uint32_t DLLCR;      /*!< HRTIM DLL control register,                                 Address offset: 0x4C */
  __IO uint32_t FLTINR1;    /*!< HRTIM Fault input register1,                                Address offset: 0x50 */
  __IO uint32_t FLTINR2;    /*!< HRTIM Fault input register2,                                Address offset: 0x54 */
  __IO uint32_t BDMUPR;     /*!< HRTIM Burst DMA Master Timer update register,               Address offset: 0x58 */
  __IO uint32_t BDTAUPR;    /*!< HRTIM Burst DMA Timerx update register,                     Address offset: 0x5C */
  __IO uint32_t BDTBUPR;    /*!< HRTIM Burst DMA Timerx update register,                     Address offset: 0x60 */
  __IO uint32_t BDTCUPR;    /*!< HRTIM Burst DMA Timerx update register,                     Address offset: 0x64 */
  __IO uint32_t BDTDUPR;    /*!< HRTIM Burst DMA Timerx update register,                     Address offset: 0x68 */  
  __IO uint32_t BDTEUPR;    /*!< HRTIM Burst DMA Timerx update register,                     Address offset: 0x6C */  
  __IO uint32_t BDMADR;     /*!< HRTIM Burst DMA Master Data register,                       Address offset: 0x70 */
}HRTIM_Common_TypeDef;

typedef struct {
  HRTIM_Master_Def sMasterRegs;
  HRTIM_Timerx_TypeDef sTimerxRegs[5];
  uint32_t             RESERVED0[32];
  HRTIM_Common_TypeDef sCommonRegs;
}HRTIM_TypeDef;

}