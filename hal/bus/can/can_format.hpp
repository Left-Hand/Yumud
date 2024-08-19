#pragma once

#include <cstdint>

#pragma packed(push, 1)
namespace CanExperimental{
struct CtrlReg{
    bool INRQ:1;
    bool SLEEP:1;
    bool TXFP:1;
    bool RFLM:1;
    bool NART:1;
    bool AWUM:1;
    bool ABOM:1;
    bool TTCM:1;
    uint32_t:7;
    bool RST:1;
    bool DBF:1;
    uint32_t:15;
};

struct StatReg{
    bool INAK:1;
    bool SLAK:1;
    bool ERRI:1;
    bool WKUI:1;
    bool SKAKI:1;
    uint32_t :3;
    bool TXM:1;
    bool RXM:1;
    bool SAMP:1;
    bool RX:1;
    uint32_t:20;
};

struct TxStatReg{
    bool RQCP0:1;
    bool TXOK0:1;
    bool ALST0:1;
    bool TERR0:1;
    uint32_t :3;
    bool ABRQ0:1;

    bool RQCP1:1;
    bool TXOK1:1;
    bool ALST1:1;
    bool TERR1:1;
    uint32_t :3;
    bool ABRQ1:1;

    bool RQCP2:1;
    bool TXOK2:1;
    bool ALST2:1;
    bool TERR2:1;
    uint32_t :3;
    bool ABRQ2:1

    uint32_t CODE:2;
    bool TME0:1;
    bool TME1:1;
    bool TME2:1;
    bool LOW0:1;
    bool LOW1:1;
    bool LOW2:1;
};

struct FifoReg{
    uint32_t FMP:2;
    uint32_t :1;
    bool FULL:1;
    bool FOVR:1;
    bool RFOM:1;
    uint32_t:26;
};

struct IntEnReg{
    bool TMEIE:1;
    
    bool FMPIE0:1;
    bool FFIE0:1;
    bool FOVIE0:1;
    
    bool FMPIE1:1;
    bool FFIE1:1;
    bool FOVIE1:1;

    uint32_t:1;
    bool EWGIE:1;
    bool EPVIE:1;
    bool BOFIE:1;
    bool LEDIE:1;
    uint32_t :3;

    bool ERRIE:1;
    bool WKUIE:1;
    bool SLKIE:1;

    uint32_t:14;
};

struct ErrReg{
    bool EWGF:1;
    bool EPVF:1;
    bool BOFF:1;
    uint32_t:1;

    uint32_t LEC:3;
    uint32_t:9;
    uint8_t TEC:8;
    uint8_t REC:8;
};

struct BitTimReg{
    bool BRP:10;
    uint32_t:6;
    uint32_t TS1:4;
    uint32_t TS2:3;
    uint32_t :1;
    bool SJW:2;
    uint32_t :3;
    bool LBKM:1;
    bool SILM:1;
};

struct TTCReg{
    uint16_t TIMCMV:16;
    bool TIMRST:1;
    bool TTCEN:1;
    uint32_t :14;
};

struct TxInfoReg{
    bool TXRQ:1;
    bool RTR:1;
    bool IDE:1;
    uint32_t EXID:18;
    uint32_t STID:11;
};

struct TxLenReg{
    uint32_t DLC:4;
    uint32_t :4;
    bool TGT:1;
    uint32_t :7;
    uint32_t TIME:16;
};

struct DataLowReg{
    uint8_t data[4];
};

struct DataHighReg{
    uint8_t data[4];
};

struct RxInfoReg{
    uint32_t:1;
    bool RTR:1;
    bool IDE:1;
    uint32_t EXID:18;
    uint32_t STID:11;
};

struct RxLenReg{
    uint32_t DLC:4;
    uint32_t :4;
    bool FMI:8;
    uint32_t TIME:16;
};

struct FilterCtrlReg{
    bool FINIT:1;
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

#pragma packed(pop)