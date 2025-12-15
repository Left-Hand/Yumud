#pragma once

#include <cstdint> 

namespace ymd::ral::ch32::ch32v035_usbpd{

struct [[nodiscard]] R32_CONFIG{
    uint32_t PD_FILT_EN:1;
    uint32_t PD_ALL_CLR:1;
    uint32_t CC_SEL:1;
    uint32_t PD_DMA_EN:1;
    uint32_t PD_RST_EN:1;
    uint32_t WAKE_POLAR:1;
    uint32_t :4;
    uint32_t IE_PD_IO:1;
    uint32_t IE_RX_BIT:1;
    uint32_t IE_RX_BYTE:1;
    uint32_t IE_RX_ACT:1;
    uint32_t IE_RX_RESET:1;
    uint32_t IE_TX_END:1;
    uint32_t BMC_CLK_CNT:9;
    uint32_t :7;
};

struct [[nodiscard]] R32_CONTROL{
    uint32_t PD_TX_EN:1;
    uint32_t BMC_START:1;
    uint32_t RX_STATE:2;
    uint32_t DATA_FLAG:1;
    uint32_t TX_BIT_BACK:1;
    uint32_t BMC_BYTE_HI:1;

    uint32_t TX_SEL1:1;
    uint32_t :1;
    uint32_t TX_SEL2:2;
    uint32_t TX_SEL3:2;
    uint32_t TX_SEL4:2;

    uint32_t BMC_TX_SZ:9;
    uint32_t :7;
};

struct [[nodiscard]] R32_STATUS{
    uint32_t DATA_BUF:8;
    uint32_t BMC_AUX:2;
    uint32_t BUF_ERR:1;
    uint32_t IF_RX_BIT:1;
    uint32_t IF_RXBYTE:1;
    uint32_t IF_RX_ACT:1;
    uint32_t IF_RX_RESET:1;
    uint32_t IF_TX_END:1;
    uint32_t BMC_BYTE_CNT:9;
    uint32_t :7;
};

struct [[nodiscard]] R32_PORT{
    uint16_t PA_CC2_AI:1;
    uint16_t :1;
    uint16_t CC2_PU:2;
    uint16_t CC2_LVE:1; 
    uint16_t CC2_CE:3;
    uint16_t :7;
};

struct [[nodiscard]] R32_DMA{
    uint16_t DMA_ADDR;
};

struct [[nodiscard]] USBPD_Def{
    volatile R32_CONFIG CONFIG;
    volatile R32_CONTROL CONTROL;
    volatile R32_STATUS STATUS;
    volatile R32_PORT PORT;
    volatile R32_DMA DMA;
};

}