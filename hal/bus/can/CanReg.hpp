#pragma once

#include <cstdint>


namespace yumud::CanExperimental{
struct CtrlReg{
    uint32_t INRQ:1;
    uint32_t SLEEP:1;
    uint32_t TXFP:1;
    uint32_t RFLM:1;
    uint32_t NART:1;
    uint32_t AWUM:1;
    uint32_t ABOM:1;
    uint32_t TTCM:1;
    uint32_t:7;
    uint32_t RST:1;
    uint32_t DBF:1;
    uint32_t:15;
};

struct StatReg{
    uint32_t INAK:1;
    uint32_t SLAK:1;
    uint32_t ERRI:1;
    uint32_t WKUI:1;
    uint32_t SKAKI:1;
    uint32_t :3;
    uint32_t TXM:1;
    uint32_t RXM:1;
    uint32_t SAMP:1;
    uint32_t RX:1;
    uint32_t:20;
};

struct TxStatReg{
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
    uint32_t ABRQ2:1

    uint32_t CODE:2;
    uint32_t TME0:1;
    uint32_t TME1:1;
    uint32_t TME2:1;
    uint32_t LOW0:1;
    uint32_t LOW1:1;
    uint32_t LOW2:1;
};

struct FifoReg{
    uint32_t FMP:2;
    uint32_t :1;
    uint32_t FULL:1;
    uint32_t FOVR:1;
    uint32_t RFOM:1;
    uint32_t:26;
};

struct IntEnReg{
    uint32_t TMEIE:1;
    
    uint32_t FMPIE0:1;
    uint32_t FFIE0:1;
    uint32_t FOVIE0:1;
    
    uint32_t FMPIE1:1;
    uint32_t FFIE1:1;
    uint32_t FOVIE1:1;

    uint32_t:1;
    uint32_t EWGIE:1;
    uint32_t EPVIE:1;
    uint32_t BOFIE:1;
    uint32_t LEDIE:1;
    uint32_t :3;

    uint32_t ERRIE:1;
    uint32_t WKUIE:1;
    uint32_t SLKIE:1;

    uint32_t:14;
};

struct ErrReg{
    uint32_t EWGF:1;
    uint32_t EPVF:1;
    uint32_t BOFF:1;
    uint32_t:1;

    uint32_t LEC:3;
    uint32_t:9;
    uint32_t TEC:8;
    uint32_t REC:8;
};

struct BitTimReg{
    uint32_t BRP:10;
    uint32_t:6;
    uint32_t TS1:4;
    uint32_t TS2:3;
    uint32_t :1;
    uint32_t SJW:2;
    uint32_t :3;
    uint32_t LBKM:1;
    uint32_t SILM:1;
};

struct TTCReg{
    uint32_t TIMCMV:16;
    uint32_t TIMRST:1;
    uint32_t TTCEN:1;
    uint32_t :14;
};

struct TxInfoReg{
    uint32_t TXRQ:1;
    uint32_t RTR:1;
    uint32_t IDE:1;
    uint32_t EXID:18;
    uint32_t STID:11;
};

struct TxLenReg{
    uint32_t DLC:4;
    uint32_t :4;
    uint32_t TGT:1;
    uint32_t :7;
    uint32_t TIME:16;
};

struct DataLowReg{
    uint32_t data[4];
};

struct DataHighReg{
    uint32_t data[4];
};

struct RxInfoReg{
    uint32_t:1;
    uint32_t RTR:1;
    uint32_t IDE:1;
    uint32_t EXID:18;
    uint32_t STID:11;
};

struct RxLenReg{
    uint32_t DLC:4;
    uint32_t :4;
    uint32_t FMI:8;
    uint32_t TIME:16;
};

struct FilterCtrlReg{
    uint32_t FINIT:1;
    uint32_t:7;
    uint32_t CAN2SB:6;
    uint32_t :18;
};

struct FilterModeReg{
    uint32_t FBM:28;
    uint32_t :4;
};

struct FilterWidthReg{
    uint32_t FSC:28;
    uint32_t :4;
};

struct FilterRelativeReg{
    uint32_t FFA:28;
    uint32_t :4;
};

struct FilterAwakenReg{
    uint32_t FACT:28;
    uint32_t :4;
};

struct FilterExpectReg{
    uint32_t FB:32;
};

struct CanTxMailBoxDef{
    volatile TxInfoReg tx_info_reg;
    volatile TxLenReg tx_len_reg;
    volatile uint8_t data[8];
};

struct CanRxMailBoxDef{
    volatile RxInfoReg rx_info_reg;
    volatile RxLenReg rx_len_reg;
    volatile uint8_t data[8];
};

struct FilterDef{
    volatile uint32_t data[2];
};

struct CanDef{
    volatile CtrlReg ctrl_reg;
    volatile StatReg stat_reg;
    volatile TxStatReg tx_stat_reg;
    volatile FifoReg fifo0_reg;
    volatile FifoReg fifo1_reg;
    volatile IntEnReg int_en_reg;
    volatile ErrReg err_reg;
    volatile BitTimReg bit_tim_reg;
    uint32_t __resv0__[88];

    volatile TTCReg ttc_reg;
    CanTxMailBoxDef tx_mailbox[3];
    CanRxMailBoxDef rx_mailbox[2];

    uint32_t __resv1__[12];

    volatile FilterCtrlReg filter_ctrl_reg;
    volatile FilterModeReg filter_mode_reg;

    uint32_t __resv2__;
    volatile FilterWidthReg filter_width_reg;

    uint32_t __resv3__;
    volatile FilterRelativeReg filter_relative_reg;
    uint32_t __resv4__;
    volatile FilterAwakenReg filter_awaken_reg;

    uint32_t __resv5__[8];
    FilterDef filters[4];
};
}
