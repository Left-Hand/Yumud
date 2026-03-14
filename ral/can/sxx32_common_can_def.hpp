#pragma once

#include "core/constants/enums.hpp"
#include "core/io/regs.hpp"

namespace ymd::ral::sxx32::common_can{

struct [[nodiscard]] R32_CAN_CTLR{
    uint32_t INRQ:1;
    uint32_t SLEEP:1;
    uint32_t TXFP:1;
    uint32_t RFLM:1;

    uint32_t NART:1;
    uint32_t AWUM:1;
    uint32_t ABOM:1;
    uint32_t TTCM:1;

    uint32_t :7;
    uint32_t RST:1;

    uint32_t DBF:1;
    uint32_t :15;
};

struct [[nodiscard]] R32_CAN_STATR{
    uint32_t INAK:1;
    uint32_t SLAK:1;
    uint32_t ERRI:1;
    uint32_t WKUI:1;

    uint32_t SLAKI:1;
    uint32_t :3;

    uint32_t TXM:1;
    uint32_t RXM:1;
    uint32_t SAMP:1;
    uint32_t RX:1;

    uint32_t :20;

};

struct [[nodiscard]] R32_CAN_TSTATR{
    uint32_t RQCP0:1;
    uint32_t TXOK0:1;
    uint32_t ALST0:1;
    uint32_t TERR0:1;

    uint32_t :3;
    uint32_t ABRQ0:1;


    uint32_t RQCP1:1;
    uint32_t TXOK1:1;
    uint32_t ALST1:1;
    uint32_t TERR1:1;

    uint32_t :3;
    uint32_t ABRQ1:1;


    uint32_t RQCP2:1;
    uint32_t TXOK2:1;
    uint32_t ALST2:1;
    uint32_t TERR2:1;

    uint32_t :3;
    uint32_t ABRQ2:1;


    uint32_t CODE:2;
    uint32_t TME0:1;
    uint32_t TME1:1;

    uint32_t TME2:1;
    uint32_t LOW0:1;
    uint32_t LOW1:1;
    uint32_t LOW2:1;
};

struct [[nodiscard]] R32_CAN_RFIFO{
    uint32_t FMP0:2;
    uint32_t :1;
    uint32_t FULL:1;

    uint32_t FOVR:1;
    uint32_t RFOM:1;

    uint32_t :26;
};

struct [[nodiscard]] R32_CAN_INTEN{
    uint32_t TMEIE:1;
    uint32_t FMPIE0:1;
    uint32_t FFIE0:1;
    uint32_t FOVIE0:1;

    uint32_t FMPIE1:1;
    uint32_t FFIE1:1;
    uint32_t FOVIE1:1;
    uint32_t :1;

    uint32_t EWGIE:1;
    uint32_t EPVIE:1;
    uint32_t BOFIE:1;
    uint32_t LECIE:1;

    uint32_t :3;
    uint32_t ERRIE:1;

    uint32_t WKUIE:1;
    uint32_t SLKIE:1;

    uint32_t :14;
};

struct [[nodiscard]] R32_CAN_ERRSR{
    // 错误警告标志位。
    // 当收发错误计数器达到警告阈值时，即大于
    // 等于 96 时，硬件置 1。
    uint32_t EWGF:1;

    uint32_t EPVF:1;
    uint32_t BOFF:1;
    uint32_t :1;


    // 上次错误代号。
    // 检测到 CAN 总线上发送错误时，控制器根据
    // 出错情况设置，当正确收发报文时，置 000b。
    // 000：无错误；
    // 001：位填充错误；
    // 010：FORM 格式错误；
    // 011：ACK 确认错误；
    // 100：隐性位错误；
    // 101：显性位错误；
    // 110：CRC 错误；
    // 111：软件设置。
    // 通常应用软件读取到错误时，把代号设置为
    // 111b，可以检测到代号更新。
    uint32_t LEC:3;
    uint32_t :9;

    // 发送错误计数器。
    // 当 CAN 发送出错时，根据出错条件，该计数
    // 器加 1 或 8；发送成功后，该计数器减 1 或
    // 设为 120(错误计数值大于 127)。计数器值超
    // 过 127 时，CAN 进入错误被动状态。
    uint32_t TEC:8;

    // 接收错误计数器。
    // 当 CAN 接收出错时，根据出错条件，该计数
    // 器加 1 或 8；接收成功后，该计数器减 1 或
    // 设为 120(错误计数值大于 127)。计数器值超
    // 过 127 时，CAN 进入错误被动状态。
    uint32_t REC:8;

};

struct [[nodiscard]] R32_CAN_BTIMR{
    uint32_t BRP:10;
    uint32_t :6;

    uint32_t TS1:4;
    uint32_t TS2:3;
    uint32_t :1;

    uint32_t SJW:2;
    uint32_t :4;
    uint32_t LBKM:1;
    uint32_t SILM:1;
};

struct [[nodiscard]] R32_CAN_TTCTLR{
    uint32_t TIMCMV:16;
    uint32_t TIMRST:1;
    uint32_t MODE:1;
    uint32_t :14;
};

struct [[nodiscard]] R32_CAN_TTCNT{
    uint32_t TIMCNT:16;
    uint32_t :16;
};

struct [[nodiscard]] R32_CAN_TXMIR{
    uint32_t TXRQ:1;
    uint32_t RTR:1;
    uint32_t IDE:1;

    uint32_t EXID:29;
};

struct [[nodiscard]] R32_CAN_TXMDTR{
    uint32_t DLC:4;
    uint32_t :4;

    uint32_t TGT:1;
    uint32_t :7;
    uint32_t TIME:16;
};



using R32_CAN_TXMDLR = uint32_t;
using R32_CAN_TXMDHR = uint32_t;

struct [[nodiscard]] R32_CAN_RXMIR{
    uint32_t :1;
    uint32_t RTR:1;
    uint32_t IDE:1;

    uint32_t EXID:29;
};

struct [[nodiscard]] R32_CAN_RXMDTR{
    uint32_t DLC:4;
    uint32_t :4;
    uint32_t FMI:8;
    uint32_t TIME:16;
};

using R32_CAN_RXMDLR = uint32_t;
using R32_CAN_RXMDHR = uint32_t;

struct [[nodiscard]] R32_CAN_FCTLR{
    // 过滤器初始化模式使能标志位。
    // 1：过滤器组为初始化模式；
    // 0：过滤器组为正常模式。
    uint32_t FINIT:1;
    uint32_t :7;

    //CAN2 过滤器开始组（取值范围 1-27）
    uint32_t CAN2SB:6;
    uint32_t :2;
    
    #ifdef CAN3_PRESENT
    // 仅h417
    //CAN3 过滤器开始组（取值范围 CAN2SB-42）
    uint32_t CAN3SB:6;
    uint32_t :2;
    uint32_t :8;
    #else
    uint32_t :16;
    #endif
};

VALIDATE_R32(R32_CAN_FCTLR)

struct [[nodiscard]] R32_CAN_FMCFGR{
    uint32_t bits;
};

struct [[nodiscard]] R32_CAN_FSCFGR{
    uint32_t bits;
};

struct [[nodiscard]] R32_CAN_FAFIFOR{
    uint32_t bits;
};

struct [[nodiscard]] R32_CAN_FWR{
    uint32_t bits;
};

struct [[nodiscard]] R32_CAN_FiR{
    uint32_t bits;
};

static_assert(sizeof(R32_CAN_FiR) == 4);

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

    void enable_dbg_freeze(const Enable en){
        CTLR.DBF = (en == EN);
    }

    void enable_ttcom_mode(const Enable en){
        CTLR.TTCM = (en == EN);
        sTxMailBox[0].TXMDTR.TGT = (en == EN);
        sTxMailBox[1].TXMDTR.TGT = (en == EN);
        sTxMailBox[2].TXMDTR.TGT = (en == EN);
    }

    [[nodiscard]] uint8_t get_rx_err_cnt(){
        return static_cast<uint8_t>(ERRSR.REC);
    }

    [[nodiscard]] uint8_t get_tx_err_cnt(){
        return ERRSR.TEC;
    }

};


VALIDATE_R32(R32_CAN_CTLR);
VALIDATE_R32(R32_CAN_STATR);
VALIDATE_R32(R32_CAN_TSTATR);
VALIDATE_R32(R32_CAN_INTEN);
VALIDATE_R32(R32_CAN_ERRSR);
VALIDATE_R32(R32_CAN_BTIMR);


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
        R32_CAN_FiR FR1;
        R32_CAN_FiR FR2;
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

    volatile CAN_Filter_Pair FILTER_PAIR[28];
};

static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FCTLR) == 0);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FMCFGR) == 4);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FSCFGR) == 0x0c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FAFIFOR) == 0x14);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FWR) == 0x1c);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[0]) == 0x40);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[14]) == 0xb0);
static_assert(__builtin_offsetof(CAN_Filter_Def, CAN_Filter_Def::FILTER_PAIR[27]) == 0x118);



[[maybe_unused]] static inline CAN_Def * CAN1_Inst = reinterpret_cast<CAN_Def *>(0x40006400);
[[maybe_unused]] static inline CAN_Def * CAN2_Inst = reinterpret_cast<CAN_Def *>(0x40006800);
[[maybe_unused]] static inline CAN_Filter_Def * CAN_Filt = reinterpret_cast<CAN_Filter_Def *>(0x40006600);

}