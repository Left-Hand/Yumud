#pragma once

#include "sxx32_common_can_regs.hpp"

namespace ymd::ral::can::ch32l103_specified{

struct [[nodiscard]] R32_CAN_TERR_CNT{
    // 当前离线恢复错误计数值，修改该计数值可
    // 从离线立即恢复
    uint32_t TX_ERR_CNT:8;
    uint32_t :24;
};

struct [[nodiscard]] R32_CANFD_CR{
    uint32_t TX_FD:1;
    uint32_t TX_BRS_B:1;
    uint32_t USER_ESI_B:1;
    uint32_t RES_EXECEPT:1;
    uint32_t CLAS_LONG_TS1:1;
    uint32_t RESTRICT_MODE:1;
    uint32_t :22;
};

struct [[nodiscard]] R32_CANFD_BTR{
    uint32_t BTR_SJW_FD:4;
    uint32_t BTR_TS2_FD:4;
    uint32_t BTR_TS1_FD:5;
    uint32_t :3;

    uint32_t BTR_BRP_FD:5;
    uint32_t :2;
    uint32_t TDCE:1;
    uint32_t :8;
};

struct [[nodiscard]] R32_CANFD_TDCT{

    uint32_t TDC0:6;
    uint32_t :2;

    uint32_t TDC_FLTER:6;
    uint32_t :2;
};

struct [[nodiscard]] R32_CANFD_PSR{
    
    uint32_t :16;
    uint32_t TDCV:8;
    uint32_t :8;
};


struct [[nodiscard]] R32_CANFD_DMA_T0{
    uint32_t DMA_ADDR;
};

struct [[nodiscard]] R32_CANFD_DMA_T1{
    uint32_t DMA_ADDR;
};

struct [[nodiscard]] R32_CANFD_DMA_T2{
    uint32_t DMA_ADDR;
};

struct [[nodiscard]] R32_CANFD_DMA_R0{
    uint32_t DMA_ADDR;
};

struct [[nodiscard]] R32_CANFD_DMA_R1{
    uint32_t DMA_ADDR;
};

struct [[nodiscard]] R32_CANFD_DMA_R2{
    uint32_t DMA_ADDR;
};


struct [[nodiscard]] CAN_Def{
    struct [[nodiscard]] TxMailBox{
        volatile sxx32_common::R32_CAN_TXMIR  TXMIR;
        volatile sxx32_common::R32_CAN_TXMDTR TXMDTR;
        volatile sxx32_common::R32_CAN_TXMDLR TXMDLR;
        volatile sxx32_common::R32_CAN_TXMDHR TXMDHR;
    };

    struct [[nodiscard]] FifoMailBox{
        volatile sxx32_common::R32_CAN_RXMIR  RXMIR;
        volatile sxx32_common::R32_CAN_RXMDTR RXMDTR;
        volatile sxx32_common::R32_CAN_RXMDLR RXMDLR;
        volatile sxx32_common::R32_CAN_RXMDHR RXMDHR;
    };


    volatile sxx32_common::R32_CAN_CTLR CTLR;
    volatile sxx32_common::R32_CAN_STATR STATR;
    volatile sxx32_common::R32_CAN_TSTATR TSTATR;
    volatile sxx32_common::R32_CAN_RFIFO RFIFO[2];
    volatile sxx32_common::R32_CAN_INTEN INTENR;
    volatile sxx32_common::R32_CAN_ERRSR ERRSR;
    volatile sxx32_common::R32_CAN_BTIMR BTIMR;
    volatile sxx32_common::R32_CAN_TTCTLR TTCTLR;
    volatile sxx32_common::R32_CAN_TTCNT TTCNT;

    volatile R32_CAN_TERR_CNT       TERR_CNT;
    volatile R32_CANFD_CR           CANFD_CR;
    volatile R32_CANFD_BTR          CANFD_BTR;
    volatile R32_CANFD_TDCT         CANFD_TDCT;
    volatile R32_CANFD_PSR          CANFD_PSR;
    volatile R32_CANFD_DMA_T0       CANFD_DMA_T0;
    volatile R32_CANFD_DMA_T1       CANFD_DMA_T1;
    volatile R32_CANFD_DMA_T2       CANFD_DMA_T2;
    volatile R32_CANFD_DMA_R0       CANFD_DMA_R0;
    volatile R32_CANFD_DMA_R1       CANFD_DMA_R1;

    uint32_t __RESV1__[76];

    TxMailBox sTxMailBox[3];
    FifoMailBox sFifoMailBox[3];
};



static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CTLR) == 0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::STATR) == 4);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TSTATR) == 8);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RFIFO[0]) == 12);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RFIFO[1]) == 16);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::INTENR) == 20);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ERRSR) == 24);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::BTIMR) == 0x1c);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TTCTLR) == 0x20);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TTCNT) == 0x24);

static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TERR_CNT) == 0x28);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_CR) == 0x2c);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_BTR) == 0x30);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_TDCT) == 0x34);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_PSR) == 0x38);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T0) == 0x3c);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T1) == 0x40);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T2) == 0x44);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_R0) == 0x48);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_R1) == 0x4c);


static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[0]) == 0x180);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[1]) == 0x190);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[2]) == 0x1a0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[0]) == 0x1B0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[1]) == 0x1c0);

struct [[nodiscard]] CAN_Filter_Def{
private:
    struct [[nodiscard]] CAN_Filter_Pair{
        // R32_CAN_FiR FIR[2];
        volatile sxx32_common::R32_CAN_FiR FR1;
        volatile sxx32_common::R32_CAN_FiR FR2;
    };
public:
    //0x40006600
    volatile sxx32_common::R32_CAN_FCTLR FCTLR;
    volatile sxx32_common::R32_CAN_FMCFGR FMCFGR;
    uint32_t __RESV0__;
    volatile sxx32_common::R32_CAN_FSCFGR FSCFGR;
    uint32_t __RESV1__;
    volatile sxx32_common::R32_CAN_FAFIFOR FAFIFOR;
    uint32_t __RESV2__;
    volatile sxx32_common::R32_CAN_FWR FWR;

    uint32_t __RESV__[8];

    CAN_Filter_Pair FILTER_PAIR[14];
};

static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FCTLR) == 0);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FMCFGR) == 4);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FSCFGR) == 0x0c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FAFIFOR) == 0x14);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FWR) == 0x1c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[0]) == 0x40);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[13]) == 0xa8);

}