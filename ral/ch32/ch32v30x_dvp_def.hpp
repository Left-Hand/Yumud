#pragma once

#include <cstdint>

struct [[nodiscard]] R8_DVP_CR0{
    uint8_t ENABLE:1;
    uint8_t V_POLAR:1;
    uint8_t H_POLAR:1;
    uint8_t P_POLAR:1;
    uint8_t MSK_DAT_MOD:2;
    uint8_t JPEG:1;
    uint8_t :2;
};

struct [[nodiscard]] R8_DVP_CR1{
    uint8_t DMA_EN:1;
    uint8_t ALL_CLR:1;
    uint8_t RCV_CLR:1;
    uint8_t BUF_TOG:1;
    uint8_t CM:1;
    uint8_t CROP:1;
    uint8_t FCRC:2;
};

struct [[nodiscard]] R8_DVP_IER{
    uint8_t STR_FRM:1;
    uint8_t ROW_DONE:1;
    uint8_t FRM_DONE:1;
    uint8_t FIFO_OV:1;
    uint8_t STP_FRM:1;
    uint8_t :3;
};

struct [[nodiscard]] R16_DVP_ROW_NUM{

};

struct [[nodiscard]] R16_DVP_COL_NUM{

};

using R32_DVP_DMA_BUF0 = uint32_t;
using R32_DVP_DMA_BUF1 = uint32_t;

struct [[nodiscard]] R8_DVP_IFR{

};

struct [[nodiscard]] R8_DVP_STATUS{

};

struct [[nodiscard]] R16_DVP_ROW_CNT{

};

struct [[nodiscard]] R16_DVP_HOFFCNT{

};

struct [[nodiscard]] R16_DVP_VST{
};

struct [[nodiscard]] R16_DVP_CAPCNT{
};

struct [[nodiscard]] R16_DVP_VLINE{

};

struct [[nodiscard]] R16_DVP_DR{

};