#pragma once

#include "sxx32_common_can_regs.hpp"
#include "ch32l103_canfd_regs.hpp"
#include "sxx32_common_can_def.hpp"

namespace ymd::ral::can::ch32l103_specified{



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
    volatile uint32_t       CANFD_DMA_T[3];
    volatile uint32_t       CANFD_DMA_R[2];

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
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T[0]) == 0x3c);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T[1]) == 0x40);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_T[2]) == 0x44);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_R[0]) == 0x48);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CANFD_DMA_R[1]) == 0x4c);


static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[0]) == 0x180);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[1]) == 0x190);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[2]) == 0x1a0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[0]) == 0x1B0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[1]) == 0x1c0);

using CAN_Filter_Def = sxx32_common::CAN_Filter_Def;
}