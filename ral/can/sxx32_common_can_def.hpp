#pragma once

#include "sxx32_common_can_regs.hpp"

namespace ymd::ral::can::sxx32_common{

struct [[nodiscard]] CAN_Def{
    struct [[nodiscard]] TxMailBox{
        volatile R32_CAN_TXMIR  TXMIR;
        volatile R32_CAN_TXMDTR TXMDTR;
        volatile R32_CAN_TXMDLR TXMDLR;
        volatile R32_CAN_TXMDHR TXMDHR;

        static_assert(sizeof(R32_CAN_TXMIR) == 4);
        static_assert(sizeof(R32_CAN_TXMDTR) == 4);
        static_assert(sizeof(R32_CAN_TXMDLR) == 4);
        static_assert(sizeof(R32_CAN_TXMDHR) == 4);
    };

    struct [[nodiscard]] FifoMailBox{
        volatile R32_CAN_RXMIR  RXMIR;
        volatile R32_CAN_RXMDTR RXMDTR;
        volatile R32_CAN_RXMDLR RXMDLR;
        volatile R32_CAN_RXMDHR RXMDHR;

        static_assert(sizeof(R32_CAN_RXMIR) == 4);
        static_assert(sizeof(R32_CAN_RXMDTR) == 4);
        static_assert(sizeof(R32_CAN_RXMDLR) == 4);
        static_assert(sizeof(R32_CAN_RXMDHR) == 4);
    };


    volatile R32_CAN_CTLR CTLR;
    volatile R32_CAN_STATR STATR;
    volatile R32_CAN_TSTATR TSTATR;
    volatile R32_CAN_RFIFO RFIFO[2];
    volatile R32_CAN_INTEN INTENR;
    volatile R32_CAN_ERRSR ERRSR;
    volatile R32_CAN_BTIMR BTIMR;
    volatile R32_CAN_TTCTLR TTCTLR;
    volatile R32_CAN_TTCNT TTCNT;

    uint32_t __RESV1__[86];

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
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[0]) == 0x180);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[1]) == 0x190);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sTxMailBox[2]) == 0x1a0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[0]) == 0x1B0);
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::sFifoMailBox[1]) == 0x1c0);

struct [[nodiscard]] CAN_Filter_Def{
private:
    struct [[nodiscard]] CAN_Filter_Pair{
        // R32_CAN_FiR FIR[2];
        volatile R32_CAN_FiR FR1;
        volatile R32_CAN_FiR FR2;
    };
public:
    //0x40006600
    volatile R32_CAN_FCTLR FCTLR;
    volatile R32_CAN_FMCFGR FMCFGR;
    uint32_t __RESV0__;
    volatile R32_CAN_FSCFGR FSCFGR;
    uint32_t __RESV1__;
    volatile R32_CAN_FAFIFOR FAFIFOR;
    uint32_t __RESV2__;
    volatile R32_CAN_FWR FWR;

    uint32_t __RESV__[8];

    CAN_Filter_Pair FILTER_PAIR[28];
};

static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FCTLR) == 0);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FMCFGR) == 4);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FSCFGR) == 0x0c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FAFIFOR) == 0x14);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FWR) == 0x1c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[0]) == 0x40);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[14]) == 0xb0);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[27]) == 0x118);



}