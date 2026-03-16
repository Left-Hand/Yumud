#pragma once

#include "sxx32_common_can_regs.hpp"

namespace ymd::ral::can::ch32l103_specified{

struct [[nodiscard]] R32_CAN_TERR_CNT{
    // 当前离线恢复错误计数值，修改该计数值可
    // 从离线立即恢复
    uint32_t TX_ERR_CNT:8;
    uint32_t :24;
};

struct [[nodiscard]] R32_CANFD_CR{
    uint32_t TX_FD:1;
    uint32_t TX_BRS_B:3;
    uint32_t USER_ESI_B:3;

    uint32_t RES_EXECEPT:1;
    uint32_t CLAS_LONG_TS1:1;
    uint32_t RESTRICT_MODE:1;
    
    uint32_t :22;
};

struct [[nodiscard]] R32_CANFD_BTR{
    uint32_t BTR_SJW_FD:4;
    uint32_t BTR_TS2_FD:4;
    uint32_t BTR_TS1_FD:5;
    uint32_t :3;

    uint32_t BTR_BRP_FD:5;
    uint32_t :2;
    uint32_t TDCE:1;
    uint32_t :8;
};

struct [[nodiscard]] R32_CANFD_TDCT{

    uint32_t TDC0:6;
    uint32_t :2;

    uint32_t TDC_FLTER:6;
    uint32_t :18;
};

struct [[nodiscard]] R32_CANFD_PSR{
    
    uint32_t :16;
    uint32_t TDCV:8;
    uint32_t :8;
};


VALIDATE_R32(R32_CAN_TERR_CNT)
VALIDATE_R32(R32_CANFD_CR)
VALIDATE_R32(R32_CANFD_BTR)
VALIDATE_R32(R32_CANFD_TDCT)
VALIDATE_R32(R32_CANFD_PSR)

}