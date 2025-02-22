#pragma once

#include <sys/io/regs.hpp>

namespace hwspec::CH32L103::AFIO_Regs{

struct CTLR_Reg{
    static constexpr uint32_t offset = 0;

    // 初始化模式请求位
    // 1：置 1 请求 CAN 控制器进入初始化模式，当
    // 前活动完成后，控制器进入初始化模式，硬
    // 件对寄存器 CAN_STATR 的 INAK 位置 1；
    // 0：置 0 请求 CAN 控制器退出初始化模式，进
    // 入正常模式，硬件对寄存器 CAN_STATR 的
    // INAK 位清 0。
    uint32_t INRQ:1;

    // 睡眠模式请求位
    // 1：置 1 请求 CAN 控制器进入睡眠模式，当前
    // 活动完成后，控制器进入睡眠模式，若 AWUM
    // 位置 1，则在接收到报文时，控制器把 SLEEP
    // 位清 0；
    // 0：软件清 0 后，控制器退出睡眠模式。
    uint32_t SLEEP:1;

    // 发送邮箱优先级方式选择
    // 1：优先级由发送请求的先后顺序决定；
    // 0：优先级由报文标识符来决定。
    uint32_t TXFP:1;

    // 接收 FIFO 报文锁定模式使能
    // 1：当接收 FIFO 溢出时，已接收邮箱报文未读
    // 出，邮箱未释放时，新接收到的报文被丢弃；
    // 0：当接收 FIFO 溢出时，已接收邮箱报文未
    // 读出，邮箱未释放时，新接收到的报文会覆
    // 盖原有报文。
    // 注：该位只用于传统 CAN
    uint32_t RFLM:1;

    // 报文自动重传功能禁止
    // 1：无论发送成功与否，报文只能被发送一次；
    // 0：CAN 控制器一直重传至发送成功为止。
    uint32_t NART:1;

    // CAN 控制器自动唤醒使能
    // 1：当检测到报文时，硬件自动唤醒，寄存器
    // CAN_STATR 的 SLEEP 和 SLAK 位自动清 0；
    // 0：需要软件操作寄存器 CAN_CTLR 的 SLEEP
    // 位清 0，唤醒 CAN 控制器。
    uint32_t AWUM:1;

    // 离线自动退出控制
    // 1：硬件检测到 128 次连续 11 个隐性位，自
    // 动退出离线状态；
    // 0：需要软件操作寄存器 CAN_CTLR 的 INRQ 位
    // 置 1 然后清 0，当检测到 128 次连续 11 个隐
    // 性位后，退出离线状态。
    uint32_t ABOM:1;

    // 是否允许时间触发模式
    // 1：使能时间触发模式；
    // 0：禁止时间触发模式。
    // 时间触发模式主要是配合 TTCAN 协议使用。
    uint32_t TTCM:1;

    uint32_t :7;

    // CAN 控制器软件复位请求，该位写 0 无效
    // 1：对 CAN 控制器进行复位，复位后控制器进
    // 入睡眠模式，然后硬件自动清 0；
    // 0：CAN 控制器正常状态。
    uint32_t RST:1;

    // 调试是否禁止 CAN 总线工作
    // 1：调试时，CAN 的收发被禁止，但是接收 FIFO
    // 的控制和读写操作一切正常；
    // 0：调试时，CAN 控制器正常工作。
    uint32_t DBF:1;
    uint32_t :15;
};

struct STATR_Reg{
    static constexpr uint32_t offset = 4;

    // 初始化模式指示位。
    // 1：CAN 控制器正在初始化模式；
    // 0：CAN 控制器工作在非初始化模式。
    uint32_t INAK:1;

    // 睡眠模式指示位。
    // 1：CAN 控制器正处于睡眠模式；
    // 0：CAN 控制器不在睡眠模式。
    uint32_t SLAK:1;
    
    // 出错中断状态标志位。当寄存器 CAN_INTENR
    // 的 ERRIE 位置 1 时，产生错误及状态变化中
    // 断。该位软件置 1 清 0，置 0 无效。
    uint32_t ERRI:1;

    // 唤醒中断标志位。当寄存器 CAN_INTENR 的
    // WKUI 位置 1 时,若 CAN 控制器处于睡眠模式
    // 时，检测到 SOF 位，则硬件置 1。软件置 1
    // 清 0，置 0 无效。
    uint32_t WKUI:1;

    // 睡眠中断使能时，即寄存器 CAN_INTENR 的
    // SLKIE 位置 1 时，中断产生标志位，写 1 清 0，
    // 写 0 无效。
    // 1：进入睡眠模式时，中断产生，硬件置 1；
    // 0：退出睡眠模式时，硬件清 0 也可软件清 0。
    uint32_t SLAKI:1;
    uint32_t :3;

    // 发送模式查询位
    // 1：当前 CAN 控制器为发送模式；
    // 0：当前 CAN 控制器非发送模式。
    uint32_t TXM:1;

    // 接收模式查询位
    // 1：当前 CAN 控制器为接收模式；
    // 0：当前 CAN 控制器非接收模式。
    uint32_t RXM:1;

    // CAN 控制器接收引脚 RX 上一个接收位的电平
    uint32_t SAMP:1;

    // CAN 控制器接收引脚 RX 当前实际电平。
    uint32_t RX:1;

    uint32_t :20;

};

struct TSTATR_Reg{
    static constexpr uint32_t offset = 8;

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
    uint32_t ABRQ2:1;


    uint32_t CODE:2;
    uint32_t TME0:1;
    uint32_t TME1:1;

    uint32_t TME2:1;
    uint32_t LOW0:1;
    uint32_t LOW1:1;
    uint32_t LOW2:1;
};

struct __RFIFO_Reg{
    uint32_t FMP:2;
    uint32_t :1;
    uint32_t FULL:1;
    uint32_t FOVR:1;
    uint32_t RFOM:1;
    uint32_t :26;
};

struct RFIFO0_Reg:public __RFIFO_Reg{
    static constexpr uint32_t offset = 0x0C;
};

struct RFIFO1_Reg:public __RFIFO_Reg{
    static constexpr uint32_t offset = 0x10;
};

struct INTEN_Reg{
    static constexpr uint32_t offset = 0x14;

    uint32_t TMEIE:1;
    uint32_t FMPIE0:1;
    uint32_t FFIE0:1;
    uint32_t FOVIE0:1;

    uint32_t FMPIE1:1;
    uint32_t FFIE1:1;
    uint32_t FOVIE1:1;
    uint32_t :1;

    uint32_t EWGIE:1;
    uint32_t EPVIE:1;
    uint32_t BOFIE:1;
    uint32_t LECIE:1;

    uint32_t :3;
    uint32_t ERRIE:1;

    uint32_t WKUIE:1;
    uint32_t SLKIE:1;

    uint32_t :14;
};


struct ERRSR_Reg{
    static constexpr uint32_t offset = 0x18;

    // 错误警告标志位。
    // 当收发错误计数器达到警告阈值时，即大于
    // 等于 96 时，硬件置 1。
    uint32_t EWGF:1;

    // 错误被动标志位。
    // 当收发错误计数器达到错误被动阈值时，即
    // 大于 127 时，硬件置 1。
    uint32_t EPVF:1;

    // 离线状态标志位。
    // 当 CAN 控制器进入离线状态时，硬件自动置
    // 1；退出离线状态时，硬件自动清 0。
    uint32_t BOFF:1;
    uint32_t :1;

    // 上次错误代号。
    // 检测到 CAN 总线上发送错误时，控制器根据
    // 出错情况设置，当正确收发报文时，置 000b。
    // 000：无错误；
    // 001：位填充错误；
    // 010：FORM 格式错误；
    // 011：ACK 确认错误；
    // 100：隐性位错误；
    // 101：显性位错误；
    // 110：CRC 错误；
    // 111：软件设置。
    // 通常应用软件读取到错误时，把代号设置为
    // 111b，可以检测到代号更新。
    uint32_t LEC:3;
    uint32_t :9;

    // 发送错误计数器。
    // 当 CAN 发送出错时，根据出错条件，该计数
    // 器加 1 或 8；发送成功后，该计数器减 1 或
    // 设为 120(错误计数值大于 127)。计数器值超
    // 过 127 时，CAN 进入错误被动状态。
    uint32_t TEC:8;

    // 接收错误计数器。
    // 当 CAN 接收出错时，根据出错条件，该计数
    // 器加 1 或 8；接收成功后，该计数器减 1 或
    // 设为 120(错误计数值大于 127)。计数器值超
    // 过 127 时，CAN 进入错误被动状态。
    uint32_t REC:8;

};


struct BTIMR_Reg{
    static constexpr uint32_t offset = 0x1c;

    // 最小时间单元长度设置值
    // Tq = (BRP[9:0]+1) × t_pcs
    uint32_t BRP:10;
    uint32_t :2;

    // 对于传统 CAN，CLAS_LONG_TS1=0，则 TS1 为
    // TS[3:0](4bit)；CLAS_LONG_TS1=1，则 TS1
    // 为 TS[1:0]+BTR_TS1_T[15:12] (6bit)。
    uint32_t BTR_TS1_T:4;

    // 时间段 1 设置值。
    // 定义了时间段 1 占用了多少个最小时间单
    // 元，实际值为（TS1[1:0]+1）。
    uint32_t TS1:4;

    // 时间段 2 设置值。
    // 定义了时间段 2 占用了多少个最小时间单
    // 元，实际值为（TS2[1:0]+1）。
    uint32_t TS2:3;
    uint32_t :1;

    // 定义了重新同步跳转宽度设置值。
    // 实现重新同步时，位中可以延长和缩小的最
    // 小 时 间 单 元 数 量 上 限 ， 实 际 值 为
    // （SJW[1:0]+1），范围可设置为 1 到 4 个最
    // 小时间单元。
    uint32_t SJW:2;
    uint32_t :4;

    // 环回模式设置位。
    uint32_t LBKM:1;

    // 静默模式设置位。
    uint32_t SILM:1;
};

struct TTCTLR_Reg{
    static constexpr uint32_t offset = 0x20;

    // 内部计数器计数终值
    uint32_t TIMCMV:16;

    // 内部计数器复位控制位。
    // 写 1 复位内部计数器，硬件自动清 0
    uint32_t TIMRST:1;

    // 时间触发模式选择位。
    // 1：增强模式；
    // 0：默认模式。
    uint32_t MODE:1;
    uint32_t :14;
};

struct TTCNT_Reg{
    static constexpr uint32_t offset = 0x24;
    // 时间触发计数值
    uint32_t TIMCNT:16;
    uint32_t :16;
};

struct TERR_CNT_Reg{
    static constexpr uint32_t offset = 0x28;
    // 当前离线恢复错误计数值，修改该计数值可
    // 从离线立即恢复
    uint32_t TX_ERR_CNT:8;
    uint32_t :24;
};

struct FD_CR_Reg{
    static constexpr uint32_t offset = 0x2c;
    uint32_t TX_FD:1;
    uint32_t TX_BRS_B:1;
    uint32_t USER_ESI_B:1;
    uint32_t RES_EXECEPT:1;
    uint32_t CLAS_LONG_TS1:1;
    uint32_t RESTRICT_MODE:1;
    uint32_t :22;
};

struct FD_BTR_Reg{
    static constexpr uint32_t offset = 0x30;
    uint32_t BTR_SJW_FD:4;
    uint32_t BTR_TS2_FD:4;
    uint32_t BTR_TS1_FD:5;
    uint32_t :3;
    uint32_t BTR_BRP_FD:5;
    uint32_t BTR_BRP_FD:2;
    uint32_t TDCE:1;
    uint32_t TDCE:8;
};

struct FD_TDCT_Reg{
    static constexpr uint32_t offset = 0x34;

    uint32_t TDC0:6;
    uint32_t :2;

    uint32_t TDC_FLTER:6;
    uint32_t :2;
};

struct FD_PSR_Reg{
    static constexpr uint32_t offset = 0x38;
    
    uint32_t :16;
    uint32_t TDCV:8;
    uint32_t :8;
};

struct __FD_DMA_Reg{
    uint32_t DMA_ADDR:15;
    uint32_t :17;
};

struct FD_DMA_T0_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x3c;
};

struct FD_DMA_T1_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x40;
};

struct FD_DMA_T2_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x44;
};

struct FD_DMA_R0_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x48;
};

struct FD_DMA_R1_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x4c;
};

struct FD_DMA_R2_Reg:public __FD_DMA_Reg{
    static constexpr uint32_t offset = 0x50;
};

struct __TXMIR_Reg{
    uint32_t TXRQ:1;
    uint32_t RTR:1;
    uint32_t IDE:1;
    union{
        uint32_t EXID:29;
        struct{
            uint32_t :18;
            uint32_t STID:11;
        };
    };
};

struct TXMIR0_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x180;
};

struct TXMIR1_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x190;
};

struct TXMIR2_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x1A0;
};

struct __TXMDTR_Reg{
    uint32_t DLC:4;
    uint32_t :4;

    uint32_t TGT:1;
    uint32_t :7;
    uint32_t TIME:16;
};

struct TXMDTR0_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x184;
};

struct TXMDTR1_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x194;
};

struct TXMDTR2_Reg:public __TXMIR_Reg{
    static constexpr uint32_t offset = 0x1A4;
};

struct __TXMDR_Reg{
    uint8_t DATA[8];
};

struct TXMDR0_Reg:public __TXMDR_Reg{
    static constexpr uint32_t offset = 0x188;
};

struct TXMDR0_Reg:public __TXMDR_Reg{
    static constexpr uint32_t offset = 0x198;
};

struct TXMDR0_Reg:public __TXMDR_Reg{
    static constexpr uint32_t offset = 0x1A8;
};

struct __RXMIR_Reg{
    uint32_t :1;
    uint32_t RTR:1;
    uint32_t IDE:1;
    union{
        uint32_t EXID:29;
        struct{
            uint32_t :18;
            uint32_t STID:11;
        };
    };
};

struct RXMIR0_Reg:public __RXMIR_Reg{
    static constexpr uint32_t offset = 0x1B0;
};

struct RXMIR1_Reg:public __RXMIR_Reg{
    static constexpr uint32_t offset = 0x1C0;
};

struct __RXMDTR_Reg{
    uint32_t DLC:4;
    uint32_t :4;
    uint32_t FMI:8;
    uint32_t TIME:16;
};

struct RXMDTR0_Reg:public __RXMDTR_Reg{
    static constexpr uint32_t offset = 0x1B4;
};

struct RXMDTR1_Reg:public __RXMDTR_Reg{
    static constexpr uint32_t offset = 0x1C4;
};

struct __RXMDR_Reg{
    uint8_t DATA[8];
};

struct RXMDR0_Reg:public __RXMDR_Reg{
    static constexpr uint32_t offset = 0x1BC;
};

struct RXMDR1_Reg:public __RXMDR_Reg{
    static constexpr uint32_t offset = 0x1CC;
};


struct FCTLR_Reg{
    static constexpr uint32_t offset = 0x200;
    // 过滤器初始化模式使能标志位。
    // 1：过滤器组为初始化模式；
    // 0：过滤器组为正常模式。
    uint32_t FINIT:1;
    uint32_t :31;
};

struct FMCFGR_Reg{
    static constexpr uint32_t offset = 0x204;

    uint32_t FBM0:1;
    uint32_t FBM1:1;
    uint32_t FBM2:1;
    uint32_t FBM3:1;
    uint32_t FBM4:1;
    uint32_t FBM5:1;
    uint32_t FBM6:1;
    uint32_t FBM7:1;
    uint32_t FBM8:1;
    uint32_t FBM9:1;

    uint32_t FBM10:1;
    uint32_t FBM11:1;
    uint32_t FBM12:1;
    uint32_t FBM13:1;
    uint32_t FBM14:1;
    uint32_t FBM15:1;
    uint32_t FBM16:1;
    uint32_t FBM17:1;
    uint32_t FBM18:1;
    uint32_t FBM19:1;

    uint32_t FBM20:1;
    uint32_t FBM21:1;
    uint32_t FBM22:1;
    uint32_t FBM23:1;
    uint32_t FBM24:1;
    uint32_t FBM25:1;
    uint32_t FBM26:1;
    uint32_t FBM27:1;

    uint32_t :4;
};

struct FSCFGR_Reg{
    static constexpr uint32_t offset = 0x20C;

    uint32_t FSC0:1;
    uint32_t FSC1:1;
    uint32_t FSC2:1;
    uint32_t FSC3:1;
    uint32_t FSC4:1;
    uint32_t FSC5:1;
    uint32_t FSC6:1;
    uint32_t FSC7:1;
    uint32_t FSC8:1;
    uint32_t FSC9:1;

    uint32_t FSC10:1;
    uint32_t FSC11:1;
    uint32_t FSC12:1;
    uint32_t FSC13:1;
    uint32_t FSC14:1;
    uint32_t FSC15:1;
    uint32_t FSC16:1;
    uint32_t FSC17:1;
    uint32_t FSC18:1;
    uint32_t FSC19:1;

    uint32_t FSC20:1;
    uint32_t FSC21:1;
    uint32_t FSC22:1;
    uint32_t FSC23:1;
    uint32_t FSC24:1;
    uint32_t FSC25:1;
    uint32_t FSC26:1;
    uint32_t FSC27:1;

    uint32_t :4;
};

struct FAFIFOR_Reg{
    static constexpr uint32_t offset = 0x214;

    uint32_t FFA0:1;
    uint32_t FFA1:1;
    uint32_t FFA2:1;
    uint32_t FFA3:1;
    uint32_t FFA4:1;
    uint32_t FFA5:1;
    uint32_t FFA6:1;
    uint32_t FFA7:1;
    uint32_t FFA8:1;
    uint32_t FFA9:1;

    uint32_t FFA10:1;
    uint32_t FFA11:1;
    uint32_t FFA12:1;
    uint32_t FFA13:1;
    uint32_t FFA14:1;
    uint32_t FFA15:1;
    uint32_t FFA16:1;
    uint32_t FFA17:1;
    uint32_t FFA18:1;
    uint32_t FFA19:1;

    uint32_t FFA20:1;
    uint32_t FFA21:1;
    uint32_t FFA22:1;
    uint32_t FFA23:1;
    uint32_t FFA24:1;
    uint32_t FFA25:1;
    uint32_t FFA26:1;
    uint32_t FFA27:1;

    uint32_t :4;
};

struct FWR_Reg{
    static constexpr uint32_t offset = 0x21C;

    uint32_t FACT0:1;
    uint32_t FACT1:1;
    uint32_t FACT2:1;
    uint32_t FACT3:1;
    uint32_t FACT4:1;
    uint32_t FACT5:1;
    uint32_t FACT6:1;
    uint32_t FACT7:1;
    uint32_t FACT8:1;
    uint32_t FACT9:1;

    uint32_t FACT10:1;
    uint32_t FACT11:1;
    uint32_t FACT12:1;
    uint32_t FACT13:1;
    uint32_t FACT14:1;
    uint32_t FACT15:1;
    uint32_t FACT16:1;
    uint32_t FACT17:1;
    uint32_t FACT18:1;
    uint32_t FACT19:1;

    uint32_t FACT20:1;
    uint32_t FACT21:1;
    uint32_t FACT22:1;
    uint32_t FACT23:1;
    uint32_t FACT24:1;
    uint32_t FACT25:1;
    uint32_t FACT26:1;
    uint32_t FACT27:1;

    uint32_t :4;
};

struct FiR_Reg{
    uint32_t FB0:1;
    uint32_t FB1:1;
    uint32_t FB2:1;
    uint32_t FB3:1;
    uint32_t FB4:1;
    uint32_t FB5:1;
    uint32_t FB6:1;
    uint32_t FB7:1;
    uint32_t FB8:1;
    uint32_t FB9:1;

    uint32_t FB10:1;
    uint32_t FB11:1;
    uint32_t FB12:1;
    uint32_t FB13:1;
    uint32_t FB14:1;
    uint32_t FB15:1;
    uint32_t FB16:1;
    uint32_t FB17:1;
    uint32_t FB18:1;
    uint32_t FB19:1;

    uint32_t FB20:1;
    uint32_t FB21:1;
    uint32_t FB22:1;
    uint32_t FB23:1;
    uint32_t FB24:1;
    uint32_t FB25:1;
    uint32_t FB26:1;
    uint32_t FB27:1;
    uint32_t FB28:1;

    uint32_t FB29:1;
    uint32_t FB30:1;
    uint32_t FB31:1;

};

// struct CAN_Def{
//     CTLR_Reg CTLR;
//     STATR_Reg STATR;
//     TSTATR_Reg TSTATR;
//     RFIFO_Reg RFIFO[2];
//     INTEN_Reg INTENR;
//     ERRSR_Reg ERRSR;
//     BTIMR_Reg BTIMR;
//     TTCTLR_Reg TTCTLR;
//     TTCNT_Reg TTCNT;

//     uint32_t __RESV1__[0x40006580 - (0x40006424+4)];

//     TXMIR_Reg TXMIR[3];//0x40006580
//     TXMDTR_Reg TXMDTR[3];
// };

// struct CAN_Filt_Def{
// private:
//     struct CAN_Filt_Pair{
//         FiR_Reg FIR[2];
//     };
// public:
//     //0x40006600
//     volatile FCTLR_Reg FCTLR;

//     volatile FMCFGR_Reg FMCFGR;
//     volatile FSCFGR_Reg FSCFGR;
//     volatile FAFIFOR_Reg FAFIFOR;
//     volatile FWR_Reg FWR;

//     volatile CAN_Filt_Pair FILTER[28];
// };


// static constexpr CAN_Def * CAN1 = (CAN_Def *)(0x40006400);
// static constexpr CAN_Def * CAN2 = (CAN_Def *)(0x40006800);
// static constexpr CAN_Filt_Def * CAN_Filt = (CAN_Filt_Def *)(0x40006600);

}