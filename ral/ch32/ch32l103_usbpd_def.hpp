#pragma once

#include "core/io/regs.hpp"

namespace ymd::ral::ch32::ch32l103_usbpd{

struct [[nodiscard]] R16_CONFIG{
    uint16_t __resv__:1;
    
    uint16_t PD_ALL_CLR:1;
    uint16_t CC_SEL:1;
    uint16_t PD_DMA_EN:1;
    uint16_t PD_RST_EN:1;
    uint16_t WAKE_POLAR:1;
    uint16_t :2;
    uint16_t MULTI_0:1;
    uint16_t RTX_BIT0:1;
    uint16_t IE_PD_I0:1;
    uint16_t IE_RX_BIT:1;
    uint16_t IE_RX_BYTE:1;
    uint16_t IE_RX_ACT:1;
    uint16_t IE_RX_RESET:1;
    uint16_t IE_TX_END:1;
};

struct [[nodiscard]] R16_BMC_CLK{
    uint16_t BMC_CLK_CNT:9;
    uint16_t :7;
};

struct [[nodiscard]] R8_CONTROL{
    uint8_t PD_TX_EN:1;
    uint8_t BMC_START:1;
    uint8_t :3;
    uint8_t DATA_FLAG:1;
    uint8_t RX_ST_L:1;
    uint8_t RX_ST_H:1;
};

struct [[nodiscard]] R8_TX_SEL{
    uint8_t TX_SEL1:1;
    uint8_t __resv__:1;
    uint8_t TX_SEL2:2;
    uint8_t TX_SEL3:2;
    uint8_t TX_SEL4:2;
};

struct [[nodiscard]] R16_BMC_TX_SZ{
    uint16_t BMC_TX_SZ:9;
    uint16_t :7;
};


struct [[nodiscard]] R8_DATA_BUF{
    uint8_t DATA_BUF:8;
};

struct [[nodiscard]] R8_STATUS{
    uint8_t BMC_AUX:2;
    uint8_t BUF_ERR:1;
    uint8_t IF_RX_BIT:1;
    uint8_t IF_RX_BYTE:1;
    uint8_t IF_RX_ACT:1;
    uint8_t IF_RX_RESET:1;
    uint8_t IF_TX_END:1;
};

struct [[nodiscard]] R16_BMC_BYTE_CNT{
    uint16_t BMC_BYTE_CNT:9;
    uint16_t :7;
};

struct [[nodiscard]] __R16_PORT_CC{
    uint16_t CC_AI:1;
    uint16_t CC_PD:1;
    uint16_t CC_PU:2;
    uint16_t CC_LVE:1;
    uint16_t CC_CE:3;
    uint16_t :8;
};

struct [[nodiscard]] R16_PORT_CC1:public __R16_PORT_CC{
};

struct [[nodiscard]] R16_PORT_CC2:public __R16_PORT_CC{
};

struct [[nodiscard]] R16_DMA{
    uint16_t BITS;
};


struct [[nodiscard]] USBPD_Def{
    volatile R16_CONFIG CONFIG;
    volatile R16_BMC_CLK BMC_CLK_CNT;
    volatile R8_CONTROL CONTROL;
    volatile R8_TX_SEL TX_SEL;
    volatile R16_BMC_TX_SZ BMC_TX_SZ;
    volatile R8_DATA_BUF DATA_BUF;
    volatile R8_STATUS STATUS;
    volatile R16_BMC_BYTE_CNT BMC_BYTE_CNT;
    volatile R16_PORT_CC1 PORT_CC1;
    volatile R16_PORT_CC2 PORT_CC2;
    volatile R16_DMA DMA;
};


static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::CONFIG) == 0x00);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::BMC_CLK_CNT) == 0x02);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::CONTROL) == 0x04);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::TX_SEL) == 0x05);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::BMC_TX_SZ) == 0x06);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::DATA_BUF) == 0x08);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::STATUS) == 0x09);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::BMC_BYTE_CNT) == 0x0a);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::PORT_CC1) == 0x0c);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::PORT_CC2) == 0x0e);
static_assert(__builtin_offsetof(USBPD_Def, USBPD_Def::DMA) == 0x10);

VALIDATE_R16(R16_CONFIG)
VALIDATE_R16(R16_BMC_CLK)
VALIDATE_R8(R8_CONTROL)
VALIDATE_R8(R8_TX_SEL)
VALIDATE_R16(R16_BMC_TX_SZ)
VALIDATE_R8(R8_DATA_BUF)
VALIDATE_R8(R8_STATUS)
VALIDATE_R16(R16_BMC_BYTE_CNT)
VALIDATE_R16(R16_PORT_CC1)
VALIDATE_R16(R16_PORT_CC2)
VALIDATE_R16(R16_DMA)

[[maybe_unused]] static constexpr USBPD_Def * USB_PD_Inst = reinterpret_cast<USBPD_Def *>(0x40027000); 
}
