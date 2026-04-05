#pragma once

#include "core/constants/enums.hpp"
#include "core/io/regs.hpp"

namespace ymd::ral::can::sxx32_common{

struct [[nodiscard]] R32_CAN_CTLR{
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

    uint32_t __RESV1__:7;

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
    uint32_t __RESV2__:15;
};

struct [[nodiscard]] R32_CAN_STATR{
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
    uint32_t __RESV1__:3;

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

    uint32_t __RESV2__:20;

};

struct [[nodiscard]] R32_CAN_TSTATR{
    uint32_t RQCP0:1;
    uint32_t TXOK0:1;
    uint32_t ALST0:1;
    uint32_t TERR0:1;

    uint32_t __RESV1__:3;
    uint32_t ABRQ0:1;


    uint32_t RQCP1:1;
    uint32_t TXOK1:1;
    uint32_t ALST1:1;
    uint32_t TERR1:1;

    uint32_t __RESV2__:3;
    uint32_t ABRQ1:1;


    uint32_t RQCP2:1;
    uint32_t TXOK2:1;
    uint32_t ALST2:1;
    uint32_t TERR2:1;

    uint32_t __RESV3__:3;
    uint32_t ABRQ2:1;


    uint32_t CODE:2;
    uint32_t TME0:1;
    uint32_t TME1:1;

    uint32_t TME2:1;
    uint32_t LOW0:1;
    uint32_t LOW1:1;
    uint32_t LOW2:1;
};

struct [[nodiscard]] R32_CAN_RFIFO{
    uint32_t FMP:2;
    uint32_t __RESV1__:1;
    uint32_t FULL:1;
    uint32_t FOVR:1;
    uint32_t RFOM:1;
    uint32_t __RESV2__:26;
};

struct [[nodiscard]] R32_CAN_INTEN{
    uint32_t TMEIE:1;
    uint32_t FMPIE0:1;
    uint32_t FFIE0:1;
    uint32_t FOVIE0:1;

    uint32_t FMPIE1:1;
    uint32_t FFIE1:1;
    uint32_t FOVIE1:1;
    uint32_t __RESV1__:1;

    uint32_t EWGIE:1;
    uint32_t EPVIE:1;
    uint32_t BOFIE:1;
    uint32_t LECIE:1;

    uint32_t __RESV2__:3;
    uint32_t ERRIE:1;

    uint32_t WKUIE:1;
    uint32_t SLKIE:1;

    uint32_t __RESV3__:14;
};


struct [[nodiscard]] R32_CAN_ERRSR{
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
    uint32_t __RESV1__:1;

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
    uint32_t __RESV2__:9;

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


struct [[nodiscard]] R32_CAN_BTIMR{
    // 最小时间单元长度设置值
    // Tq = (BRP[9:0]+1) × t_pcs
    uint32_t BRP:10;
    uint32_t __RESV1__:2;

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
    uint32_t __RESV2__:1;

    // 定义了重新同步跳转宽度设置值。
    // 实现重新同步时，位中可以延长和缩小的最
    // 小 时 间 单 元 数 量 上 限 ， 实 际 值 为
    // （SJW[1:0]+1），范围可设置为 1 到 4 个最
    // 小时间单元。
    uint32_t SJW:2;
    uint32_t __RESV3__:4;

    // 环回模式设置位。
    uint32_t LBKM:1;

    // 静默模式设置位。
    uint32_t SILM:1;
};

struct [[nodiscard]] R32_CAN_TTCTLR{
    // 内部计数器计数终值
    uint32_t TIMCMV:16;

    // 内部计数器复位控制位。
    // 写 1 复位内部计数器，硬件自动清 0
    uint32_t TIMRST:1;

    // 时间触发模式选择位。
    // 1：增强模式；
    // 0：默认模式。
    uint32_t MODE:1;
    uint32_t __RESV1__:14;
};

struct [[nodiscard]] R32_CAN_TTCNT{
    // 时间触发计数
    uint32_t TIMCNT:16;
    uint32_t __RESV1__:16;
};


struct [[nodiscard]] R32_CAN_TXMIR{
    uint32_t TXRQ:1;
    uint32_t RTR:1;
    uint32_t IDE:1;

    uint32_t EXID:29;
};

struct [[nodiscard]] R32_CAN_TXMDTR{
    uint32_t DLC:4;
    uint32_t FDCAN_BRS:1;
    uint32_t FDCAN_ESI:1;
    uint32_t FDCAN_RES:1;
    uint32_t __RESV1__:1;

    uint32_t TGT:1;
    uint32_t __RESV2__:7;
    uint32_t TIME:16;
};



using R32_CAN_TXMDLR = uint32_t;
using R32_CAN_TXMDHR = uint32_t;

struct [[nodiscard]] R32_CAN_RXMIR{
    uint32_t __RESV1__:1;
    uint32_t RTR:1;
    uint32_t IDE:1;

    uint32_t EXID:29;
};

struct [[nodiscard]] R32_CAN_RXMDTR{
    uint32_t DLC:4;
    uint32_t __RESV1__:4;
    uint32_t FMI:8;
    uint32_t TIME:16;
};

using R32_CAN_RXMDLR = uint32_t;
using R32_CAN_RXMDHR = uint32_t;

struct [[nodiscard]] R32_CAN_FCTLR{
    // 过滤器初始化模式使能标志位。
    // 1：过滤器组为初始化模式；
    // 0：过滤器组为正常模式。
    uint32_t FINIT:1;
    uint32_t __RESV1__:7;

    //CAN2 过滤器开始组（取值范围 1-27）
    uint32_t CAN2SB:6;
    uint32_t __RESV2__:2;
    
    #ifdef CAN3_PRESENT
    // 仅h417
    //CAN3 过滤器开始组（取值范围 CAN2SB-42）
    uint32_t CAN3SB:6;
    uint32_t __RESV3__:2;
    uint32_t __RESV4__:8;
    #else
    uint32_t __RESV5__:16;
    #endif
};

VALIDATE_R32(R32_CAN_FCTLR)

struct [[nodiscard]] R32_CAN_FMCFGR{
    uint32_t BITS;
};

struct [[nodiscard]] R32_CAN_FSCFGR{
    uint32_t BITS;
};

struct [[nodiscard]] R32_CAN_FAFIFOR{
    uint32_t BITS;
};

struct [[nodiscard]] R32_CAN_FWR{
    uint32_t BITS;
};

struct [[nodiscard]] R32_CAN_FiR{
    uint32_t BITS;
};

VALIDATE_R32(R32_CAN_FiR);
VALIDATE_R32(R32_CAN_CTLR);
VALIDATE_R32(R32_CAN_STATR);
VALIDATE_R32(R32_CAN_TSTATR);
VALIDATE_R32(R32_CAN_INTEN);
VALIDATE_R32(R32_CAN_ERRSR);
VALIDATE_R32(R32_CAN_BTIMR);

}