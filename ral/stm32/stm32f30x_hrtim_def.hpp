#pragma once

#include <cstdint>

namespace ymd::ral::stm32::stm32f30x_hrtim{

struct [[nodiscard]] R32_MCR{
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

struct [[nodiscard]] R32_MISR{
    uint32_t MCMP1:1;
    uint32_t MCMP2:1;
    uint32_t MCMP3:1;
    uint32_t MCMP4:1;
    uint32_t MREP:1;
    uint32_t SYNC:1;
    uint32_t MUPD:1;

    uint32_t :29;
};

struct [[nodiscard]] R32_MICR{
    uint32_t MCMP1C:1;
    uint32_t MCMP2C:1;
    uint32_t MCMP3C:1;
    uint32_t MCMP4C:1;

    uint32_t MCMPC:1;
    uint32_t SYNCC:1;
    uint32_t MUPDC:1;

    uint32_t :29;
};

struct [[nodiscard]] R32_MDIER{
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

// struct [[nodiscard]] R32_MCNTR{
//     uint32_t MCNT:16;
//     uint32_t :16;
// };

// struct [[nodiscard]] R32_MPER{
//     uint32_t MPER:16;
//     uint32_t :16;
// };

// struct [[nodiscard]] R32_MREP{
//     uint32_t MREP:16;
//     uint32_t :16;
// };


struct [[nodiscard]] HRTIM_Master_Def{
    volatile R32_MCR MCR;            /*!< HRTIM Master Timer control register,                     Address offset: 0x00 */
    volatile R32_MISR MISR;           /*!< HRTIM Master Timer interrupt status register,            Address offset: 0x04 */
    volatile R32_MICR MICR;           /*!< HRTIM Master Timer interupt clear register,              Address offset: 0x08 */
    volatile R32_MDIER MDIER;          /*!< HRTIM Master Timer DMA/interrupt enable register         Address offset: 0x0C */
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

struct [[nodiscard]] R32_TIMxCR{
    uint32_t :3;
    uint32_t CONT:1;
    uint32_t RETRIG:1;
    uint32_t HALF:1;
    uint32_t PSHPLL:1;
    uint32_t :3;
    uint32_t SYNCRSTx:1;
    uint32_t SYNCSTRTx:1;
    uint32_t DELCMP2:2;
    uint32_t DELCMP4:2;

    uint32_t :1;
    uint32_t TxREPU:1;
    uint32_t TxRSTU:1;
    uint32_t :1;
    uint32_t TBU:1;
    uint32_t TCU:1;
    uint32_t TDU:1;
    uint32_t TEU:1;
    uint32_t MSTU:1;
    uint32_t DACSYNC:2;
    uint32_t PREEN:1;
    uint32_t UPDGAT:4;
};

struct [[nodiscard]] R32_TIMxISR{
    uint32_t CMP1:1;
    uint32_t CMP2:1;
    uint32_t CMP3:1;
    uint32_t CMP4:1;
    uint32_t REP:1;
    uint32_t :1;
    uint32_t UPD:1;
    uint32_t CPT1:1;
    uint32_t CPT2:1;
    uint32_t SETx1:1;
    uint32_t RSTx1:1;
    uint32_t SETx2:1;
    uint32_t RSTx2:1;
    uint32_t RSTx2:1;
    uint32_t RST:1;
    uint32_t DLYPRT:1;
    uint32_t :1;


    uint32_t CPPSTAT:1;
    uint32_t IPPSTAT:1;
    uint32_t O1STAT:1;
    uint32_t O2STAT:1;
    uint32_t O1CPY:1;
    uint32_t O2CPY:1;
    uint32_t :10;
};

struct [[nodiscard]] R32_TIMxICR{
    uint32_t CMP1C:1;
    uint32_t CMP2C:1;
    uint32_t CMP3C:1;
    uint32_t CMP4C:1;
    uint32_t REPC:1;
    uint32_t :1;
    uint32_t UPDC:1;
    uint32_t CPT1C:1;
    uint32_t CPT2C:1;
    uint32_t SETx1C:1;
    uint32_t RSTx1C:1;
    uint32_t SETx2C:1;
    uint32_t RSTx2C:1;
    uint32_t RSTC:1;
    uint32_t DLYPRTC:1;
    uint32_t :17;
};

struct [[nodiscard]] R32_TIMxDIER{
    uint32_t CMP1IE:1;
    uint32_t CMP2IE:1;
    uint32_t CMP3IE:1;
    uint32_t CMP4IE:1;
    uint32_t REPIR:1;
    uint32_t :1;
    uint32_t UPDIE:1;
    uint32_t CPT1IE:1;
    uint32_t CPT2IE:1;
    uint32_t SETx1IE:1;
    uint32_t RSTx1IE:1;
    uint32_t SETx2IE:1;
    uint32_t RSTx2IE:1;
    uint32_t RSTxIE:1;
    uint32_t DLYPRTIE:1;
    uint32_t :1;

    uint32_t CMP1DE:1;
    uint32_t CMP2DE:1;
    uint32_t CMP3DE:1;
    uint32_t CMP4DE:1;
    uint32_t REPDE:1;
    uint32_t :1;
    uint32_t UPDDE:1;
    uint32_t CPT1DE:1;
    uint32_t CPT2DE:1;
    uint32_t SETx1DE:1;
    uint32_t RSTx1DE:1;
    uint32_t SETx2DE:1;
    uint32_t RSTx2DE:1;
    uint32_t RSTDE:1;
    uint32_t DLYPRTDE:1;
    uint32_t :1;
};

struct [[nodiscard]] R32_DTxR{
    uint32_t :9;
    uint32_t SDTRx:1;
    uint32_t DTPRSC:3;
    uint32_t :1;
    uint32_t DTRSLKx:1;
    uint32_t DTRLKx:1;
    uint32_t DTFx:9;
    uint32_t SDTFx:1;
    uint32_t :4;
    uint32_t DTFSLKx:1;
    uint32_t DTFLKx:1;
};

struct [[nodiscard]] R32_SETx1_2R{
    uint32_t SST:1;
    uint32_t RESYNC:1;
    uint32_t PER:1;
    uint32_t CMP1:1;

    uint32_t CMP2:1;
    uint32_t CMP3:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;

    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;

    uint32_t TIMEVNT1:1;
    uint32_t TIMEVNT2:1;
    uint32_t TIMEVNT3:1;
    uint32_t TIMEVNT4:1;

    uint32_t TIMEVNT5:1;
    uint32_t TIMEVNT6:1;
    uint32_t TIMEVNT7:1;
    uint32_t TIMEVNT8:1;
    uint32_t TIMEVNT9:1;
    
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;
    uint32_t UPDATE:1;
};

struct [[nodiscard]] R32_RSTx1_2R{
    uint32_t SRT:1;
    uint32_t RESYNC:1;
    uint32_t PER:1;
    uint32_t CMP1:1;

    uint32_t CMP2:1;
    uint32_t CMP3:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;

    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;

    uint32_t TIMEVNT1:1;
    uint32_t TIMEVNT2:1;
    uint32_t TIMEVNT3:1;
    uint32_t TIMEVNT4:1;

    uint32_t TIMEVNT5:1;
    uint32_t TIMEVNT6:1;
    uint32_t TIMEVNT7:1;
    uint32_t TIMEVNT8:1;
    uint32_t TIMEVNT9:1;
    
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;
    uint32_t UPDATE:1;
};

struct [[nodiscard]] R32_EEFxR1{
    uint32_t EE1LTCH:1;
    uint32_t EE1FLTR:4;
    uint32_t :1;

    uint32_t EE2LTCH:1;
    uint32_t EE2FLTR:4;
    uint32_t :1;

    uint32_t EE3LTCH:1;
    uint32_t EE3FLTR:4;
    uint32_t :1;

    uint32_t EE4LTCH:1;
    uint32_t EE4FLTR:4;
    uint32_t :1;

    uint32_t EE5LTCH:1;
    uint32_t EE5FLTR:4;
    uint32_t :3;
};

struct [[nodiscard]] R32_EEFxR2{
    uint32_t EE6LTCH:1;
    uint32_t EE6FLTR:4;
    uint32_t :1;

    uint32_t EE7LTCH:1;
    uint32_t EE7FLTR:4;
    uint32_t :1;

    uint32_t EE8LTCH:1;
    uint32_t EE8FLTR:4;
    uint32_t :1;

    uint32_t EE9LTCH:1;
    uint32_t EE9FLTR:4;
    uint32_t :1;

    uint32_t EE10LTCH:1;
    uint32_t EE10FLTR:4;
    uint32_t :3;
};

struct [[nodiscard]] R32_RSTAR{
    uint32_t :1;
    uint32_t UPDT:1;
    uint32_t CMP2:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;
    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;

    uint32_t TIMBCMP1:1;
    uint32_t TIMBCMP2:1;
    uint32_t TIMBCMP4:1;

    uint32_t TIMCCMP1:1;
    uint32_t TIMCCMP2:1;
    uint32_t TIMCCMP4:1;

    uint32_t TIMDCMP1:1;
    uint32_t TIMDCMP2:1;
    uint32_t TIMDCMP4:1;

    uint32_t TIMECMP1:1;
    uint32_t TIMECMP2:1;
    uint32_t TIMECMP4:1;

    uint32_t :1;
};

struct [[nodiscard]] R32_RSTBR{
    uint32_t :1;
    uint32_t UPDT:1;
    uint32_t CMP2:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;
    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;

    uint32_t TIMACMP1:1;
    uint32_t TIMACMP2:1;
    uint32_t TIMACMP4:1;

    uint32_t TIMCCMP1:1;
    uint32_t TIMCCMP2:1;
    uint32_t TIMCCMP4:1;

    uint32_t TIMDCMP1:1;
    uint32_t TIMDCMP2:1;
    uint32_t TIMDCMP4:1;

    uint32_t TIMECMP1:1;
    uint32_t TIMECMP2:1;
    uint32_t TIMECMP4:1;

    uint32_t :1;
};

struct [[nodiscard]] R32_RSTCR{
    uint32_t :1;
    uint32_t UPDT:1;
    uint32_t CMP2:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;
    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;

    uint32_t TIMACMP1:1;
    uint32_t TIMACMP2:1;
    uint32_t TIMACMP4:1;

    uint32_t TIMBCMP1:1;
    uint32_t TIMBCMP2:1;
    uint32_t TIMBCMP4:1;

    uint32_t TIMDCMP1:1;
    uint32_t TIMDCMP2:1;
    uint32_t TIMDCMP4:1;

    uint32_t TIMECMP1:1;
    uint32_t TIMECMP2:1;
    uint32_t TIMECMP4:1;

    uint32_t :1;
};

struct [[nodiscard]] R32_RSTDR{
    uint32_t :1;
    uint32_t UPDT:1;
    uint32_t CMP2:1;
    uint32_t CMP4:1;
    uint32_t MSTPER:1;
    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;
    uint32_t EXTEVNT1:1;
    uint32_t EXTEVNT2:1;
    uint32_t EXTEVNT3:1;
    uint32_t EXTEVNT4:1;
    uint32_t EXTEVNT5:1;
    uint32_t EXTEVNT6:1;
    uint32_t EXTEVNT7:1;
    uint32_t EXTEVNT8:1;
    uint32_t EXTEVNT9:1;
    uint32_t EXTEVNT10:1;

    uint32_t TIMACMP1:1;
    uint32_t TIMACMP2:1;
    uint32_t TIMACMP4:1;

    uint32_t TIMBCMP1:1;
    uint32_t TIMBCMP2:1;
    uint32_t TIMBCMP4:1;

    uint32_t TIMCCMP1:1;
    uint32_t TIMCCMP2:1;
    uint32_t TIMCCMP4:1;


    uint32_t TIMECMP1:1;
    uint32_t TIMECMP2:1;
    uint32_t TIMECMP4:1;

    uint32_t :1;
};

struct [[nodiscard]] R32_CHPxR{
    uint32_t CARFRQ:4;
    uint32_t CARDTY:3;
    uint32_t STRTPW:4;
};

struct [[nodiscard]] CPTnxCR{
    uint32_t SWCPT:1;
    uint32_t UPDCPT:1;
    uint32_t EXEV1CPT:1;
    uint32_t EXEV2CPT:1;
    uint32_t EXEV3CPT:1;
    uint32_t EXEV4CPT:1;
    uint32_t EXEV5CPT:1;
    uint32_t EXEV6CPT:1;
    uint32_t EXEV7CPT:1;
    uint32_t EXEV8CPT:1;
    uint32_t EXEV9CPT:1;
    uint32_t EXEV10CPT:1;

    
    uint32_t TA1SET:1;
    uint32_t TA1RET:1;
    uint32_t TA1CMP1:1;
    uint32_t TA1CMP2:1;

    uint32_t TB1SET:1;
    uint32_t TB1RET:1;
    uint32_t TB1CMP1:1;
    uint32_t TB1CMP2:1;

    uint32_t TC1SET:1;
    uint32_t TC1RET:1;
    uint32_t TC1CMP1:1;
    uint32_t TC1CMP2:1;

    uint32_t TD1SET:1;
    uint32_t TD1RET:1;
    uint32_t TD1CMP1:1;
    uint32_t TD1CMP2:1;

    uint32_t TE1SET:1;
    uint32_t TE1RET:1;
    uint32_t TE1CMP1:1;
    uint32_t TE1CMP2:1;
};

struct [[nodiscard]] OUTxR{
    uint32_t :1;
    uint32_t POL1:1;
    uint32_t IDLEM1:1;
    uint32_t IDLES1:1;
    uint32_t FAULT1:2;
    uint32_t CHP1:1;
    uint32_t DIDL1:1;

    uint32_t DTEN:1;
    uint32_t DLYPRTEN:1;
    uint32_t DLYPRT:3;
    uint32_t :4;

    uint32_t POL:1;
    uint32_t IDLEM:1;
    uint32_t IDLES:1;
    uint32_t FAULT:2;
    uint32_t CHP2:1;
    uint32_t DIDL:1;
};

struct [[nodiscard]] R32_CR1{
    uint32_t MUDIS:1;
    uint32_t TAUDIS:1;
    uint32_t TBUDIS:1;
    uint32_t TCUDIS:1;
    uint32_t TDUDIS:1;
    uint32_t TEUDIS:1;
    uint32_t :11;
    uint32_t AD1USRC:3;
    uint32_t AD2USRC:3;
    uint32_t AD3USRC:3;
    uint32_t AD4USRC:3;
    uint32_t :4;
};


struct [[nodiscard]] R32_CR2{
    uint32_t MSWU:1;
    uint32_t TASWU:1;
    uint32_t TBSWU:1;
    uint32_t TCSWU:1;
    uint32_t TDSWU:1;
    uint32_t TESWU:1;
    uint32_t :2;
    uint32_t MRST:1;
    uint32_t TARST:1;
    uint32_t TBRST:1;
    uint32_t TCRST:1;
    uint32_t TDRST:1;
    uint32_t TERST:1;
    uint32_t :18;
};

struct [[nodiscard]] R32_ISR{
    uint32_t FLT1:1;
    uint32_t FLT2:1;
    uint32_t FLT3:1;
    uint32_t FLT4:1;
    uint32_t FLT5:1;
    uint32_t SYSFLT:1;
    uint32_t :9;
    uint32_t DLLRDY:1;
    uint32_t BMPER:1;
    uint32_t :14;
};

struct [[nodiscard]] R32_ICR{
    uint32_t FLT1C:1;
    uint32_t FLT2C:1;
    uint32_t FLT3C:1;
    uint32_t FLT4C:1;
    uint32_t FLT5C:1;
    uint32_t SYSFLTC:1;
    uint32_t :9;
    uint32_t DLLRDYC:1;
    uint32_t BMPERC:1;
    uint32_t :14;
};


struct [[nodiscard]] R32_IER{
    uint32_t FLT1IE:1;
    uint32_t FLT2IE:1;
    uint32_t FLT3IE:1;
    uint32_t FLT4IE:1;
    uint32_t FLT5IE:1;
    uint32_t SYSFLTIE:1;
    uint32_t :9;
    uint32_t DLLRDYIE:1;
    uint32_t BMPERIE:1;
    uint32_t :14;
};

struct [[nodiscard]] R32_OENR{
    uint32_t TA1OEN:1;
    uint32_t TA2OEN:1;
    uint32_t TB1OEN:1;
    uint32_t TB2OEN:1;
    uint32_t TC1OEN:1;
    uint32_t TC2OEN:1;
    uint32_t TD1OEN:1;
    uint32_t TD2OEN:1;
    uint32_t TE1OEN:1;
    uint32_t TE2OEN:1;
    uint32_t :22;
};

struct [[nodiscard]] R32_ODISR{
    uint32_t TA1ODIS:1;
    uint32_t TA2ODIS:1;
    uint32_t TB1ODIS:1;
    uint32_t TB2ODIS:1;
    uint32_t TC1ODIS:1;
    uint32_t TC2ODIS:1;
    uint32_t TD1ODIS:1;
    uint32_t TD2ODIS:1;
    uint32_t TE1ODIS:1;
    uint32_t TE2ODIS:1;
};

struct [[nodiscard]] R32_ODSR{
    uint32_t TA1ODS:1;
    uint32_t TA2ODS:1;
    uint32_t TB1ODS:1;
    uint32_t TB2ODS:1;
    uint32_t TC1ODS:1;
    uint32_t TC2ODS:1;
    uint32_t TD1ODS:1;
    uint32_t TD2ODS:1;
    uint32_t TE1ODS:1;
    uint32_t TE2ODS:1;
};

struct [[nodiscard]] R32_BMCR{
    uint32_t BME:1;
    uint32_t BMOM:1;
    uint32_t BMCLK:4;
    uint32_t BMPRSC:4;
    uint32_t BMPREN:1;
    uint32_t :5;
    uint32_t MTBM:1;
    uint32_t TABM:1;
    uint32_t TBBM:1;
    uint32_t TCBM:1;
    uint32_t TDBM:1;
    uint32_t TEBM:1;
    uint32_t :9;
    uint32_t BMSTAT:1;
};

struct [[nodiscard]] R32_BMTRGR{
    uint32_t SW:1;
    uint32_t MSTRST:1;
    uint32_t MSTREP:1;

    uint32_t MSTCMP1:1;
    uint32_t MSTCMP2:1;
    uint32_t MSTCMP3:1;
    uint32_t MSTCMP4:1;

    uint32_t TARST:1;
    uint32_t TAREP:1;
    uint32_t TACMP1:1;
    uint32_t TACMP2:1;

    uint32_t TBRST:1;
    uint32_t TBREP:1;
    uint32_t TBCMP1:1;
    uint32_t TBCMP2:1;

    uint32_t TCRST:1;
    uint32_t TCREP:1;
    uint32_t TCCMP1:1;
    uint32_t TCCMP2:1;

    uint32_t TDRST:1;
    uint32_t TDREP:1;
    uint32_t TDCMP1:1;
    uint32_t TDCMP2:1;

    uint32_t TERST:1;
    uint32_t TEREP:1;
    uint32_t TECMP1:1;
    uint32_t TECMP2:1;

    uint32_t TAEEV7:1;
    uint32_t TAEEV8:1;
    uint32_t EEV7:1;
    uint32_t EEV8:1;
    uint32_t OCHPEV:1;
};

struct [[nodiscard]] R32_EECR1{
    uint32_t EE1SRC:2;
    uint32_t EE1POL:1;
    uint32_t EE1SNS:2;
    uint32_t EE1FAST:1;

    uint32_t EE2SRC:2;
    uint32_t EE2POL:1;
    uint32_t EE2SNS:2;
    uint32_t EE2FAST:1;

    uint32_t EE3SRC:2;
    uint32_t EE3POL:1;
    uint32_t EE3SNS:2;
    uint32_t EE3FAST:1;

    uint32_t EE4SRC:2;
    uint32_t EE4SRC:2;
    uint32_t EE4POL:1;
    uint32_t EE4POL:1;

    uint32_t EE5SNS:2;
    uint32_t EE5SNS:2;
    uint32_t EE5FAST:1;
    uint32_t EE5FAST:1;

    uint32_t :2;
};

struct [[nodiscard]] R32_EECR2{
    uint32_t EE6SRC:1;
    uint32_t EE6POL:1;
    uint32_t EE6SNS:2;
    uint32_t :1;

    uint32_t EE7SRC:1;
    uint32_t EE7POL:1;
    uint32_t EE7SNS:2;
    uint32_t :1;

    uint32_t EE8SRC:1;
    uint32_t EE8POL:1;
    uint32_t EE8SNS:2;
    uint32_t :1;

    uint32_t EE9SRC:1;
    uint32_t EE9POL:1;
    uint32_t EE9SNS:2;
    uint32_t :1;

    uint32_t EE10SRC:1;
    uint32_t EE10POL:1;
    uint32_t EE10SNS:2;
    uint32_t :3;
};


struct [[nodiscard]] R32_EECR3{
    uint32_t EE6F:4;
    uint32_t :2;
    uint32_t EE7F:4;
    uint32_t :2;
    uint32_t EE8F:4;
    uint32_t :2;
    uint32_t EE9F:4;
    uint32_t :2;
    uint32_t EE10F:4;
    uint32_t :2;
    uint32_t EEVSD:2;
};

struct [[nodiscard]] R32_ADC1R{
    uint32_t AD1MC1:1;
    uint32_t AD1MC2:1;
    uint32_t AD1MC3:1;
    uint32_t AD1MC4:1;
    uint32_t AD1MPER:1;
    uint32_t AD1EEV1:1;
    uint32_t AD1EEV2:1;
    uint32_t AD1EEV3:1;
    uint32_t AD1EEV4:1;
    uint32_t AD1EEV5:1;
    uint32_t AD1TAC2:1;
    uint32_t AD1TAC3:1;
    uint32_t AD1TAC4:1;
    uint32_t AD1TAPER:1;
    uint32_t AD1TARST:1;
    uint32_t AD1TBC2:1;
    uint32_t AD1TBC3:1;
    uint32_t AD1TBC4:1;
    uint32_t AD1TBPER:1;
    uint32_t AD1TBRST:1;
    uint32_t AD1TC2:1;
    uint32_t AD1TC3:1;
    uint32_t AD1TC4:1;
    uint32_t AD1TCPER:1;
    uint32_t AD1TDC2:1;
    uint32_t AD1TDC3:1;
    uint32_t AD1TDC4:1;
    uint32_t AD1TDPER:1;
    uint32_t AD1TEC2:1;
    uint32_t AD1TEC3:1;
    uint32_t AD1TEC4:1;
    uint32_t AD1TEPER:1;
};

struct [[nodiscard]] R32_ADC2R{
    uint32_t AD2MC1:1;
    uint32_t AD2MC2:1;
    uint32_t AD2MC3:1;
    uint32_t AD2MC4:1;
    uint32_t AD2MPER:1;
    uint32_t ADC2EEV6:1;
    uint32_t ADC2EEV7:1;
    uint32_t ADC2EEV8:1;
    uint32_t ADC2EEV9:1;
    uint32_t ADC2EEV10:1;
    uint32_t ADC2TAC2:1;
    uint32_t ADC2TAC3:1;
    uint32_t ADC2TAC4:1;
    uint32_t ADC2TAPER:1;
    uint32_t ADC2TBC2:1;
    uint32_t ADC2TBC3:1;

    uint32_t ADC2TBC4:1;
    uint32_t ADC2TBPER:1;
    uint32_t ADC2TCC2:1;

    uint32_t ADC2TCC4:1;
    uint32_t ADC2TCPER:1;
    uint32_t ADC2TCRST:1;
    uint32_t ADC2TDC2:1;
    uint32_t ADC2TDC3:1;
    uint32_t ADC2TDC4:1;
    uint32_t ADC2TDPER:1;
    uint32_t ADC2TDRST:1;
    uint32_t ADC2TEC2:1;
    uint32_t ADC2TEC3:1;
    uint32_t ADC2TEC4:1;
    uint32_t ADC2TERST:1;
};


struct [[nodiscard]] R32_ADC3R{
    uint32_t AD3MC1:1;
    uint32_t AD3MC2:1;
    uint32_t AD3MC3:1;
    uint32_t AD3MC4:1;
    uint32_t AD3MPER:1;
    uint32_t AD3EEV1:1;
    uint32_t AD3EEV2:1;
    uint32_t AD3EEV3:1;
    uint32_t AD3EEV4:1;
    uint32_t AD3EEV5:1;
    uint32_t AD3TAC2:1;
    uint32_t AD3TAC3:1;
    uint32_t AD3TAC4:1;
    uint32_t AD3TAPER:1;
    uint32_t AD3TARST:1;
    uint32_t AD3TBC2:1;
    uint32_t AD3TBC3:1;
    uint32_t AD3TBC4:1;
    uint32_t AD3TBPER:1;
    uint32_t AD3TBRST:1;
    uint32_t AD3TC2:1;
    uint32_t AD3TC3:1;
    uint32_t AD3TC4:1;
    uint32_t AD3TCPER:1;
    uint32_t AD3TDC2:1;
    uint32_t AD3TDC3:1;
    uint32_t AD3TDC4:1;
    uint32_t AD3TDPER:1;
    uint32_t AD3TEC2:1;
    uint32_t AD3TEC3:1;
    uint32_t AD3TEC4:1;
    uint32_t AD3TEPER:1;
};

struct [[nodiscard]] R32_ADC4R{
    uint32_t AD4MC1:1;
    uint32_t AD4MC2:1;
    uint32_t AD4MC3:1;
    uint32_t AD4MC4:1;
    uint32_t AD4MPER:1;
    uint32_t ADC4EEV6:1;
    uint32_t ADC4EEV7:1;
    uint32_t ADC4EEV8:1;
    uint32_t ADC4EEV9:1;
    uint32_t ADC4EEV10:1;
    uint32_t ADC4TAC2:1;
    uint32_t ADC4TAC3:1;
    uint32_t ADC4TAC4:1;
    uint32_t ADC4TAPER:1;
    uint32_t ADC4TBC2:1;
    uint32_t ADC4TBC3:1;

    uint32_t ADC4TBC4:1;
    uint32_t ADC4TBPER:1;
    uint32_t ADC4TCC2:1;

    uint32_t ADC4TCC4:1;
    uint32_t ADC4TCPER:1;
    uint32_t ADC4TCRST:1;
    uint32_t ADC4TDC2:1;
    uint32_t ADC4TDC3:1;
    uint32_t ADC4TDC4:1;
    uint32_t ADC4TDPER:1;
    uint32_t ADC4TDRST:1;
    uint32_t ADC4TEC2:1;
    uint32_t ADC4TEC3:1;
    uint32_t ADC4TEC4:1;
    uint32_t ADC4TERST:1;
};

struct [[nodiscard]] R32_DLLCR{
    uint32_t CAL:1;
    uint32_t CALEN:1;
    uint32_t CALRTE:2;
};

struct [[nodiscard]] FLTINR1{
    uint32_t FLT1E:1;
    uint32_t FLT1P:1;
    uint32_t FLT1SRC:1;
    uint32_t FLT1F:4;
    uint32_t FLT1LCK:1;

    uint32_t FLT2E:1;
    uint32_t FLT2P:1;
    uint32_t FLT2SRC:1;
    uint32_t FLT2F:4;
    uint32_t FLT2LCK:1;

    uint32_t FLT3E:1;
    uint32_t FLT3P:1;
    uint32_t FLT3SRC:1;
    uint32_t FLT3F:4;
    uint32_t FLT3LCK:1;

    uint32_t FLT4E:1;
    uint32_t FLT4P:1;
    uint32_t FLT4SRC:1;
    uint32_t FLT4F:4;
    uint32_t FLT4LCK:1;
};

struct [[nodiscard]] FLTINR2{
    uint32_t FLT5E:1;
    uint32_t FLT5P:1;
    uint32_t FLT5SRC:1;
    uint32_t FLT5F:4;
    uint32_t FLT5LCK:1;
    uint32_t :16;
    uint32_t FLTSD:2;
    uint32_t :6;
};

struct [[nodiscard]] R32_BDMUPR{
    uint32_t MCR:1;
    uint32_t MICR:1;
    uint32_t MDIER:1;
    uint32_t MCNT:1;
    uint32_t MPER:1;
    
    uint32_t MCMP1:1;
    uint32_t MCMP2:1;
    uint32_t MCMP3:1;
    uint32_t MCMP4:1;
    uint32_t :22;
};

struct [[nodiscard]] R32_BDTxUPR{
    uint32_t TIMxCR:1;
    uint32_t TIMxICR:1;
    uint32_t TIMxDIER:1;
    uint32_t TIMxCNT:1;
    uint32_t TIMxPER:1;
    uint32_t TIMxCMP1:1;
    uint32_t TIMxCMP21;
    uint32_t TIMxCMP3:1;
    uint32_t TIMxCMP4:1;
    uint32_t TIMxDTxR:1;

    uint32_t TIMxSET1R:1;
    uint32_t TIMxRST1R:1;

    uint32_t TIMxSET2R:1;
    uint32_t TIMxRST2R:1;

    uint32_t TIMxEEFR1:1; 
    uint32_t TIMxEEFR2:1; 
    uint32_t TIMxCHPR:1;
    uint32_t TIMxOUTR:1;
    uint32_t TIMxFLTR:1;

    uint32_t :11;
};

struct [[nodiscard]] FLTxR{
    uint32_t FLT1EN:1;
    uint32_t FLT2EN:1;
    uint32_t FLT3EN:1;
    uint32_t FLT5EN:1;
    uint32_t :26;
    uint32_t FLTCLK:1;
};


typedef struct{[[nodiscard]] 
    __IO R32_TIMxCR TIMxCR;     /*!< HRTIM Timerx control register,                              Address offset: 0x00  */
    __IO R32_TIMxISR TIMxISR;    /*!< HRTIM Timerx interrupt status register,                     Address offset: 0x04  */
    __IO R32_TIMxICR TIMxICR;    /*!< HRTIM Timerx interrupt clear register,                      Address offset: 0x08  */
    __IO R32_TIMxDIER TIMxDIER;   /*!< HRTIM Timerx DMA/interrupt enable register,                 Address offset: 0x0C  */
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
    __IO R32_DTxR DTxR;       /*!< HRTIM Timerx dead time register,                            Address offset: 0x38 */
    __IO R32_SETx1_2R SETx1R;     /*!< HRTIM Timerx output 1 set register,                         Address offset: 0x3C */
    __IO R32_RSTx1_2R RSTx1R;     /*!< HRTIM Timerx output 1 reset register,                       Address offset: 0x40 */
    __IO R32_SETx1_2R SETx2R;     /*!< HRTIM Timerx output 2 set register,                         Address offset: 0x44 */
    __IO R32_RSTx1_2R RSTx2R;     /*!< HRTIM Timerx output 2 reset register,                       Address offset: 0x48 */
    __IO R32_EEFxR1 EEFxR1;     /*!< HRTIM Timerx external event filtering 1 register,           Address offset: 0x4C */
    __IO R32_EEFxR2 EEFxR2;     /*!< HRTIM Timerx external event filtering 2 register,           Address offset: 0x50 */
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
[[nodiscard]] }HRTIM_Common_TypeDef;

typedef struct [[nodiscard]] {
  HRTIM_Master_Def sMasterRegs;
  HRTIM_Timerx_TypeDef sTimerxRegs[5];
  uint32_t             RESERVED0[32];
  HRTIM_Common_TypeDef sCommonRegs;
}HRTIM_TypeDef;

}