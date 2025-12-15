#pragma once

#include <cstdint>

namespace ymd::CH32L103::USBFS{
struct [[nodiscard]] R8_USB_CTRL{
    uint8_t DMA_EN:1;
    uint8_t CLR_ALL:1;
    uint8_t RST_SIE:1;
    uint8_t INT_BUSY:1;
    uint8_t SYS_CTRL:2;
    uint8_t DEV_PU_EN:2;
    uint8_t LOW_SPEED:2;
    uint8_t HOST_MODE:2;
};

struct [[nodiscard]] R8_USB_INT_EN{
    uint8_t BUS_RST:1;
    uint8_t UIE_TRANSFER:1;
    uint8_t SUSPEND:1;
    uint8_t HST_SOF:1;
    uint8_t FIFO_OV:1;
    uint8_t WIRE_1:1;
    uint8_t DEV_NAK:1;
    uint8_t :1;
};

struct [[nodiscard]] R8_USB_DEV_AD{
    uint8_t USB_ADDR:7;
    uint8_t GP_BIT:1;
};

struct [[nodiscard]] R8_USB_MIS_ST{
    uint8_t DEV_ATTACH:1;
    uint8_t DM_LEVEL:1;
    uint8_t SUSPEND:1;
    uint8_t BUS_RST:1;
    uint8_t RFIFO_RDY:1;
    uint8_t SIE_FREE:1;
    uint8_t SOF_ACT:1;
    uint8_t SOF_PRES:1;
};


struct [[nodiscard]] R8_USB_INT_FG{
    uint8_t BUS_RST:1;
    uint8_t UIE_TRANSFER:1;
    uint8_t SUSPEND:1;
    uint8_t HST_SOF:1;
    uint8_t FIFO_OV:1;
    uint8_t SIE_FREE:1;
    uint8_t TOG_OK:1;
    uint8_t IS_NAK:1;
};

struct [[nodiscard]] R8_USB_INT_ST{
    uint8_t ENDP:4;
    uint8_t OTKEN:2;
    uint8_t TOG_OK:1;
    uint8_t SETUP_ACT:1;
};

struct [[nodiscard]] R16_USB_RX_LEN{
    uint16_t LEN:9;
    uint16_t :7;
};

struct [[nodiscard]] R8_UDEV_CTRL{
    uint8_t PORT_EN:1;
    uint8_t GP_BIT:1;
    uint8_t LOW_SPEED:1;
    uint8_t :1;
    uint8_t DM_PIN:1;
    uint8_t DP_PIN:1;
    uint8_t :1;
    uint8_t PD_DIS:1;
};

struct [[nodiscard]] R8_UEP4_1_MOD{
    uint8_t UEP4_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP4_TX_EN:1;
    uint8_t UEP4_RX_EN:1;
    uint8_t UEP1_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP1_TX_EN:1;
    uint8_t UEP1_RX_EN:1;
};

struct [[nodiscard]] R8_UEP2_3_MOD{
    uint8_t UEP2_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP2_TX_EN:1;
    uint8_t UEP2_RX_EN:1;
    uint8_t UEP3_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP3_TX_EN:1;
    uint8_t UEP3_RX_EN:1;
};


struct [[nodiscard]] R8_UEP5_6_MOD{
    uint8_t UEP5_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP5_TX_EN:1;
    uint8_t UEP5_RX_EN:1;
    uint8_t UEP6_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP6_TX_EN:1;
    uint8_t UEP6_RX_EN:1;
};

struct [[nodiscard]] R8_UEP7_MOD{
    uint8_t UEP5_BUF_MODE:1;
    uint8_t :1;
    uint8_t UEP5_TX_EN:1;
    uint8_t UEP5_RX_EN:1;
    uint8_t :4;
};

struct [[nodiscard]] R32_UEP_DMA{
    uint32_t ADDR:15;
    uint32_t :17;
};

struct [[nodiscard]] R32_USB_EP_CTRL{
};

struct [[nodiscard]] R16_UEP_0_1_T_LEN{
    uint16_t T_LEN:7;
    uint16_t :9;
};

}