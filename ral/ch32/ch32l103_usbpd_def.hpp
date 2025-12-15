#pragma once

#include <core/io/regs.hpp>

namespace ymd::ral::ch32::ch32l103_usbpd{

struct [[nodiscard]] R16_CONFIG{
    static constexpr uint32_t offset = 0x00;

    uint16_t :1;
    
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
    static constexpr uint32_t offset = 0x02;

    uint16_t BMC_CLK_CNT:9;
    uint16_t :7;
};

struct [[nodiscard]] R8_CONTROL{
    static constexpr uint32_t offset = 0x04;

    uint8_t PD_TX_EN:1;
    uint8_t BMC_START:1;
    uint8_t :3;
    uint8_t DATA_FLAG:1;
    uint8_t RX_ST_L:1;
    uint8_t RX_ST_H:1;
};

struct [[nodiscard]] R8_TX_SEL{
    static constexpr uint32_t offset = 0x05;
    uint8_t TX_SEL1:1;
    uint8_t :1;
    uint8_t TX_SEL2:2;
    uint8_t TX_SEL3:2;
    uint8_t TX_SEL4:2;
};

struct [[nodiscard]] R16_BMC_TX_SZ{
    static constexpr uint32_t offset = 0x06;

    uint16_t BMC_TX_SZ:9;
    uint16_t :7;
};


struct [[nodiscard]] R8_DATA_BUF{
    static constexpr uint32_t offset = 0x09;

    uint8_t DATA_BUF:8;
};

struct [[nodiscard]] R8_STATUS{
    static constexpr uint32_t offset = 0x09;

    uint8_t BMC_AUX:2;
    uint8_t BUF_ERR:1;
    uint8_t IF_RX_BIT:1;
    uint8_t IF_RX_BYTE:1;
    uint8_t IF_RX_ACT:1;
    uint8_t IF_RX_RESET:1;
    uint8_t IF_TX_END:1;
};

struct [[nodiscard]] R16_BMC_BYTE_CNT{
    static constexpr uint32_t offset = 0x0A;

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
    static constexpr uint32_t offset = 0x0C;
};

struct [[nodiscard]] R16_PORT_CC2:public __R16_PORT_CC{
    static constexpr uint32_t offset = 0x0E;
};

struct [[nodiscard]] R16_DMA{
    static constexpr uint32_t offset = 0x10;
    uint16_t DMA_ADDR:15;
};


struct [[nodiscard]] USBPD_Def{
    volatile R16_CONFIG CONFIG;
    volatile R16_BMC_CLK BMC_CLK_CNT;
    volatile R8_CONTROL CONTROL;
    volatile R8_TX_SEL TX_SEL;
    volatile R16_BMC_TX_SZ BMC_TX_SZ;
    uint8_t :8;
    volatile R8_DATA_BUF DATA_BUF;
    volatile R8_STATUS STATUS;
    volatile R16_BMC_BYTE_CNT BMC_BYTE_CNT;
    volatile R16_PORT_CC1 PORT_CC1;
    volatile R16_PORT_CC2 PORT_CC2;
    volatile R16_DMA DMA;
};

static constexpr USBPD_Def * USB_PD = (USBPD_Def *)0x40027000; 
}
