#pragma once

#include <cstdint>

namespace hwspec::V4C{

struct CFGR_Reg{
    uint32_t :7;
    uint32_t SYSRESET:1;
    uint32_t :8;
    uint32_t KEYCODE:16;
};


struct GISR_Reg{
    //当前中断嵌套状态
    uint32_t NESTSTA:8;

    // 当前是否有中断被执行
    uint32_t GACTSTA:1;

    // 当前是否有中断处于挂起
    uint32_t GPENDSTA:1;
    uint32_t :22;
};

struct SCTLR_Reg{
    uint32_t :1;
    uint32_t SLEEPONEXIT:1;
    uint32_t SLEEPDEEP:1;
    uint32_t WFIOWFE:1;
    uint32_t SEVONPEND:1;
    uint32_t SETEVENT:1;
    uint32_t :25;
    uint32_t SYSRESET:1;
};

struct INTSYSCR_Reg{
    uint32_t HWSTKEN:1;    //硬件硬件压栈使能
    uint32_t INESTEN:1;    // 中断嵌套使能
    uint32_t PMTCFG:2;    // 中断嵌套深度配置
    uint32_t HWSTKOVEN:1;    // 硬件压栈溢出后中断使能
    uint32_t GIHWSTKNEN:1;    // 全局中断和硬件压栈关闭使能
    uint32_t :2;
    uint32_t PMTSTA:8;    // 抢占位状态指示
    uint32_t :16;
};

struct MTVEC_Reg{
    uint32_t MODE0:1;//中断或异常入口地址模式选择
    uint32_t MODE1:1;//中断向量表识别模式
    uint32_t BASEADDR:30;//异常基地址寄存器
};

struct ITHRESDR_Reg{
    uint32_t :5;
    uint32_t THRESD:3;
    uint32_t :24;
};

struct PFIC_Def{
    //中断 0-31 使能状态寄存器，
    // 共 32 个状态位[n]，表示#n
    // 中断使能状态
    volatile  uint32_t    ISR[8];

    // 中断 0-31 挂起状态寄存器，
    // 共 32 个状态位[n]，表示#n
    // 中断的挂起状态
    volatile  uint32_t    IPR[8];
    volatile  ITHRESDR_Reg    ITHRESDR;
    volatile  uint32_t    __RESV__;
    volatile  CFGR_Reg    CFGR;
    volatile  GISR_Reg    GISR;
    volatile  uint8_t     VTFIDR[4];
    uint8_t               __RESV0__[12];
    volatile  uint32_t    VTFADDR[4];
    uint8_t               __RESV1__[0x90];
    volatile  uint32_t    IENR[8];
    uint8_t               __RESV2__[0x60];
    volatile  uint32_t    IRER[8];
    uint8_t               __RESV3__[0x60];
    volatile  uint32_t    IPSR[8];
    uint8_t               __RESV4__[0x60];
    volatile  uint32_t    IPRR[8];
    uint8_t               __RESV5__[0x60];
    volatile  uint32_t    IACTR[8];
    uint8_t               __RESV6__[0xE0];
    volatile uint8_t      IPRIOR[256];
    uint8_t               __RESV7__[0x810];
    volatile uint32_t     SCTLR;
};

struct PMPiCfg{
    uint8_t R:1;
    uint8_t W:1;
    uint8_t X:1;
    uint8_t A:2;
    uint8_t :2;
    uint8_t LOCK:2;
    
};

struct PMP_Def{

};

struct SYSTICK_Def{
    struct CTLR_Reg{
        uint32_t STE:1;
        uint32_t STIE:1;
        uint32_t STCLK:1;
        uint32_t STRE:1;
        uint32_t MODE:1;
        uint32_t INIT:1;
        uint32_t :25;
        uint32_t SWIE:1;
    };

    struct SR_Reg{
        uint32_t CNTIF:1;
        uint32_t :31;
    };

    volatile CTLR_Reg   CTLR;
    volatile SR_Reg     SR;
    volatile uint64_t   CNT;
    volatile uint64_t   CMP;
};

struct DBG_Def{
    uint32_t DATA[2];
};

static inline PFIC_Def * PFIC_Inst = (PFIC_Def *)(0xE000E000);

__inline bool isInterruptPending(){
    return PFIC_Inst->GISR.GPENDSTA;
}

__inline bool isIntrruptActing(){
    return PFIC_Inst->GISR.GACTSTA;
}

__inline uint8_t getInterruptDepth(){
    return PFIC_Inst->GISR.NESTSTA;
}

}