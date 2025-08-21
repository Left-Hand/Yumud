#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::CH32{

struct R32_SERDES_CTRL{
    uint32_t CLR_ALL:1;
    uint32_t RESET_LINK:1;
    uint32_t RESET_PHY:1;
    uint32_t INT_BUSY_EN:1;
    uint32_t RX_POLARITY:1;
    uint32_t RX_EN:1;
    uint32_t TX_EN:1;
    uint32_t DMA_EN:1;
    uint32_t __RESV1__:6;
    uint32_t RX_PWR_UP:1;
    uint32_t TX_PWR_UP:1;
    uint32_t __RESV2__:1;
    uint32_t CONT_EN:1;
    uint32_t ALIGN_EN:1;
    uint32_t :13;
};ASSERT_REG_IS_32BIT(R32_SERDES_CTRL)

struct R32_SERDES_INT_EN{

    uint32_t PHYTDY_IE:1;
    uint32_t TRAN_DONE_IE:1;
    uint32_t RECV_DONE_IE:1;
    uint32_t FIFO_OVE_IE:1;
    uint32_t :1;
    uint32_t COMINIT_IT:1;

    uint32_t COMINIT_IT:26;
};ASSERT_REG_IS_32BIT(R32_SERDES_INT_EN)

struct R32_SERDES_INT_FS{
    //物理层 READY 中断标志位
    uint32_t PHYTDY_IF:1;

    //发送模式：发送完成中断标志位。
    //接收模式：接收 CRC 错误中断标志位。
    uint32_t TRAN_DONE_IF:1;

    uint32_t RECV_DONE_IF:1;

    uint32_t FIFO_OV_IF:1;

    uint32_t :1;

    uint32_t COMINIT_IF:1;
    uint32_t :10;

    uint32_t PHYRDY:1;
    uint32_t RX_SEQ_MATCH:1;
    uint32_t RECV_CRC_OK:1;
    uint32_t PLL_LOCK:1;
    uint32_t LINK_FREE:1;
    uint32_t R_FIFO_RDY:1;
    uint32_t :2;

    uint32_t RX_SEQ_NUM:1;
    uint32_t TX_SEQ_NUM:1;
};ASSERT_REG_IS_32BIT(R32_SERDES_INT_FS)


struct R32_SERDESx_RTX_CTRL{
    uint32_t SERDES_TX_LEN:16;
    uint32_t LINK_INIT:1;
    uint32_t TX_VLD:1;
    uint32_t BUF_MODE:1;
    uint32_t :13;
};ASSERT_REG_IS_32BIT(R32_SERDESx_RTX_CTRL)

struct R32_SERDES_RX_LEN0{
    uint32_t LEN:16;
    uint32_t :16;
};ASSERT_REG_IS_32BIT(R32_SERDES_RX_LEN0)


struct R32_SERDES_DATA0{
    uint32_t DATA;
};ASSERT_REG_IS_32BIT(R32_SERDES_DATA0)

struct R32_SERDES_DMA0{
    uint32_t ADDR;
};ASSERT_REG_IS_32BIT(R32_SERDES_RX_DMA0)

struct R32_SERDES_RX_LEN1{
    uint32_t LEN:16;
    uint32_t :16;
};ASSERT_REG_IS_32BIT(R32_SERDES_RX_LEN1)

struct R32_SERDES_DATA1{
    uint32_t DATA;
};ASSERT_REG_IS_32BIT(R32_SERDES_DATA1)

struct R32_SERDES_DMA1{
    uint32_t ADDR;
};ASSERT_REG_IS_32BIT(R32_SERDES_RX_DMA1)

struct R32_SERDES_SYS_CFGR{
    uint32_t :3;
    uint32_t TX_OUTPUT_SWING:2;
    uint32_t TX_DE_EMPHASIS:2;
    uint32_t :25;
};ASSERT_REG_IS_32BIT(R32_SERDES_SYS_CFGR)

}