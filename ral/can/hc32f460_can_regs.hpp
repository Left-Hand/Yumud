#pragma once

#include "sxx32_common_can_regs.hpp"

// https://blog.csdn.net/zhaopenglihui/article/details/118057283
// https://bbs.16rd.com/thread-579061-1-1.html

namespace ymd::ral::can::hc32f460{


// RBUF 寄存器指向最早接收到的 CAN 邮箱的 RB SLOT 地址，RBUF 寄存器可以按照任意顺序读取。
// KOER 位即为寄存器 EALCAP.KOER，仅在 RBALL=1 时有意义。
// TX 位表示在回环模式下接受到自己发送的消息。
// CYCLE_TIME 位仅在 TTCAN 模式时有效，表示 SOF 开始时的 cycle time。

struct [[nodiscard]] R32_CAN_RX_CONTROL{
    // DLC(Data Length Code):
    // 数据长度码，设定范围为 0~8，对应数据长度为 0Byte~8Byte
    uint32_t DLC:4;
    uint32_t :2;

    // RTR(Remote Transmission Request)
    // 0：数据帧
    // 1：远程帧
    uint32_t RTR:1;

    // IDE(IDentifier Extension):
    // 0：标准格式
    // 1：扩展格式
    uint32_t IDE:1;
    uint32_t :4;

    // 表示在回环模式下接受到自己发送的消息
    uint32_t TX:1;
    uint32_t KOER:1;

    //TTCAN
    uint32_t CYCLE_TIME:1;
};

// CAN 接收邮箱的数据格式如下：
struct CAN_RBUF{
    //标志id和拓展id都从最低位开始
    volatile uint32_t ID;

    volatile R32_CAN_RX_CONTROL CONTROL;

    volatile uint32_t DATA[2];
};



// TBUF 寄存器指向下一个空的 CAN 发送 BUF SLOT，TBUF 寄存器可以按照任意顺序写入。通过软件将
// TSNEXT 写 1 来标记对应的 TBUF SLOT 已经写入数据，从而指向下一个 TBUF SLOT。
// TBUF 只能 WORD 访问。
struct [[nodiscard]] R32_CAN_TX_CONTROL{
    // DLC(Data Length Code):
    // 数据长度码，设定范围为 0~8，对应数据长度为 0Byte~8Byte
    uint32_t DLC:4;
    uint32_t :2;

    // RTR(Remote Transmission Request)
    // 0：数据帧
    // 1：远程帧
    uint32_t RTR:1;

    // IDE(IDentifier Extension):
    // 0：标准格式
    // 1：扩展格式
    uint32_t IDE:1;
    uint32_t :24;
};

// CAN 发送邮箱的数据格式如下：

struct [[nodiscard]] CAN_TBUF{
    volatile uint32_t ID;

    volatile R32_CAN_TX_CONTROL CONTROL;

    volatile uint32_t DATA[2];
};

// CAN配置和状态寄存器（8位）[0xA0] 复位值:0x80
struct [[nodiscard]] R8_CAN_CFG_STAT {
    uint8_t BUSOFF:1;       // 总线关闭位               复位值(0)
    uint8_t TACTIVE:1;      // 发送激活位               复位值(0)
    uint8_t RACTIVE:1;      // 接收激活位               复位值(0)
    uint8_t TSSS:1;         // 次发送缓冲区状态位       复位值(0)
    uint8_t TPSS:1;         // 主发送缓冲区状态位       复位值(0)
    uint8_t LBME:1;         // 本地回环模式使能位       复位值(0)
    uint8_t LBMI:1;         // 本地回环模式中断标志位   复位值(0)
    uint8_t RESET:1;        // 复位位                   复位值(1)
};

// CAN命令寄存器（8位）[0xA1] 复位值:0x00
struct [[nodiscard]] R8_CAN_TCMD {
    uint8_t TSA:1;          // STB发送取消位            复位值(0) R/W
    uint8_t TSALL:1;        // 发送所有STB数据设定位    复位值(0) R/W
    uint8_t TSONE:1;        // 发送一帧STB数据设定位    复位值(0) R/W
    uint8_t TPA:1;          // PTB发送取消位            复位值(0) R/W
    uint8_t TPE:1;          // PTB发送使能位            复位值(0) R/W
    uint8_t Reserved:1;     // 保留位-必须保持复位值    复位值(0) R
    uint8_t LOM:1;          // 静默模式使能位           复位值(0) R/W
    uint8_t TBSEL:1;        // 发送BUF选择位            复位值(0) R/W
};

// CAN发送控制寄存器（8位）[0xA2] 复位值:0x90
struct [[nodiscard]] R8_CAN_TCTRL {
    uint8_t TSSTAT:2;       // STB状态位                复位值(00) R
    uint8_t Reserved:2;     // 保留位-必须保持复位值    复位值(00) R
    uint8_t TTTBM:1;        // TTCAN BUF模式位          复位值(0) R/W
    uint8_t TSMODE:1;       // STB发送模式位            复位值(1) R/W
    uint8_t TSNEXT:1;       // 下一个STB SLOT位         复位值(0) R/W
    uint8_t Reserved1:1;    // 保留位-必须保持复位值    复位值(1) R
};

// CAN接收控制寄存器（8位）[0xA3] 复位值:0x00
struct [[nodiscard]] R8_CAN_RCTRL {
    uint8_t RSTAT:2;        // 接收BUF状态位            复位值(00) R
    uint8_t Reserved:1;     // 保留位-必须保持复位值    复位值(0) R
    uint8_t RBALL:1;        // 接收BUF存储所有帧位      复位值(0) R/W
    uint8_t RREL:1;         // 释放接收BUF位            复位值(0) R/W
    uint8_t ROV:1;          // 接收BUF上溢标志位        复位值(0) R
    uint8_t ROM:1;          // 接收BUF上溢模式位        复位值(0) R/W
    uint8_t SACK:1;         // 自应答位                 复位值(0) R/W
};

// CAN接收和发送中断使能寄存器（8位）[0xA4] 复位值:0xFE
struct [[nodiscard]] R8_CAN_RTIE {
    uint8_t TSFF:1;         // 发送BUF满标志位          复位值(1) R
    uint8_t EIE:1;          // 错误中断使能位           复位值(1) R/W
    uint8_t TSIE:1;         // STB发送中断使能位        复位值(0) R/W
    uint8_t TPIE:1;         // PTB发送中断使能位        复位值(0) R/W
    uint8_t RAFIE:1;        // 接收BUF将满中断使能位    复位值(0) R/W
    uint8_t RFIE:1;         // 接收BUF满中断使能位      复位值(0) R/W
    uint8_t ROIE:1;         // 接收上溢中断使能位       复位值(1) R/W
    uint8_t RIE:1;          // 接收中断使能位           复位值(1) R/W
};

// CAN接收和发送中断状态寄存器（8位）[0xA5] 复位值:0x00
struct [[nodiscard]] R8_CAN_RTIF {
    uint8_t AIF:1;          // 取消发送中断标志位       复位值(0) R/W
    uint8_t EIF:1;          // 错误中断标志位           复位值(0) R/W
    uint8_t TSIF:1;         // STB发送中断标志位        复位值(0) R/W
    uint8_t TPIF:1;         // PTB发送中断标志位        复位值(0) R/W
    uint8_t RAFIF:1;        // 接收BUF将满中断标志位    复位值(0) R/W
    uint8_t RFIF:1;         // 接收BUF满中断标志位      复位值(0) R/W
    uint8_t ROIF:1;         // 接收上溢中断标志位       复位值(0) R/W
    uint8_t RIF:1;          // 接收中断标志位           复位值(0) R/W
};

// CAN错误中断使能和标志寄存器（8位）[0xA6] 复位值:0x00
struct [[nodiscard]] R8_CAN_ERRINT {
    uint8_t BEIF:1;         // 总线错误中断标志位       复位值(0) R/W
    uint8_t BEIE:1;         // 总线错误中断使能位       复位值(0) R/W
    uint8_t ALIF:1;         // 仲裁失败中断标志位       复位值(0) R/W
    uint8_t ALE:1;          // 仲裁失败中断使能位       复位值(0) R/W
    uint8_t EPIF:1;         // 错误被动中断标志位       复位值(0) R/W
    uint8_t EPIE:1;         // 错误被动中断使能位       复位值(0) R/W
    uint8_t EPASS:1;        // 错误被动节点位           复位值(0) R
    uint8_t EWARN:1;        // 到达错误警告值位         复位值(0) R
};

// CAN警告限定寄存器（8位）[0xA7] 复位值:0x1B
struct [[nodiscard]] R8_CAN_LIMIT {
    uint8_t EWL:4;          // 可编程错误警告值位       复位值(1011) R/W
    uint8_t AFWL:4;         // 接收BUF将满警告值位      复位值(0001) R/W
};

// CAN位时序寄存器（32位）[0xA8] 复位值:0x01020203
struct [[nodiscard]] R32_CAN_BT {
    uint32_t SEG_2:7;       // 时间段2位                复位值(03)
    uint32_t Reserved:1;    // 保留位
    uint32_t SEG_1:8;       // 时间段1位                复位值(02)

    uint32_t SJW:7;         // 同步跳转宽度位           复位值(02)
    uint32_t Reserved1:1;   // 保留位
    uint32_t PRESC:8;       // 预分频值位               复位值(01)
};

// CAN错误和仲裁失败捕捉寄存器（8位）[0xB0] 复位值:0x00
struct [[nodiscard]] R8_CAN_EALCAP {
    uint8_t ALC:5;          // 仲裁失败位置捕捉位       复位值(00000) R
    uint8_t KOER:3;         // 错误类别位               复位值(000) R
};

// CAN接收错误计数器寄存器（8位）[0xB2] 复位值:0x00
struct [[nodiscard]] R8_CAN_RECNT {
    uint8_t RECNT:8;        // 接收错误计数器位         复位值(00000000) R
};

// CAN发送错误计数器寄存器（8位）[0xB3] 复位值:0x00
struct [[nodiscard]] R8_CAN_TECNT {
    uint8_t TECNT:8;        // 发送错误计数器位         复位值(00000000) R
};

// CAN筛选器组控制寄存器（8位）[0xB4] 复位值:0x00
struct [[nodiscard]] R8_CAN_ACFCTRL {
    uint8_t ACFADR:4;       // 筛选器地址位             复位值(0000) R/W
    uint8_t Reserved:1;     // 保留位-必须保持复位值    复位值(0) R
    uint8_t SELMASK:1;      // 筛选器屏蔽寄存器选择位   复位值(0) R/W
    uint8_t Reserved1:2;    // 保留位-必须保持复位值    复位值(00) R
};

// CAN筛选器组使能寄存器（8位）[0xB6] 复位值:0x01
struct [[nodiscard]] R8_CAN_ACFEN {
    uint8_t AE_1:1;         // ACF_1使能位              复位值(1) R/W
    uint8_t AE_2:1;         // ACF_2使能位              复位值(0) R/W
    uint8_t AE_3:1;         // ACF_3使能位              复位值(0) R/W
    uint8_t AE_4:1;         // ACF_4使能位              复位值(0) R/W
    uint8_t AE_5:1;         // ACF_5使能位              复位值(0) R/W
    uint8_t AE_6:1;         // ACF_6使能位              复位值(0) R/W
    uint8_t AE_7:1;         // ACF_7使能位              复位值(0) R/W
    uint8_t AE_8:1;         // ACF_8使能位              复位值(0) R/W
};

// CAN筛选器组code和mask寄存器（32位）[0xB8] 复位值:0xXXXXXXXX
struct [[nodiscard]] R32_CAN_ACF {
    uint32_t ACODE_AMASK:29;// 筛选器CODE/MASK位        复位值(X) R/W
    uint32_t AIDE:1;        // IDE位MASK位              复位值(X) R/W
    uint32_t AIDEE:1;       // IDE位比较使能位          复位值(X) R/W
    uint32_t Reserved:1;    // 保留位-读出值不定        复位值(X) R
};

// TTCAN TB slot指针寄存器（8位）[0xBE] 复位值:0x00
struct [[nodiscard]] R8_CAN_TBSLOT {
    uint8_t TBPTR:3;        // TB SLOT指针位            复位值(000) R/W
    uint8_t Reserved:3;     // 保留位-必须保持复位值    复位值(000) R
    uint8_t TBF:1;          // 设置TB为已填充位         复位值(0) R/W
    uint8_t TBE:1;          // 设置TB为空位             复位值(0) R/W
};

// TTCAN时间触发配置寄存器（8位）[0xBF] 复位值:0x90
struct [[nodiscard]] R8_CAN_TTCFG {
    uint8_t TTEN:1;         // TTCAN使能位              复位值(0) R/W
    uint8_t T_PRESC:2;      // TTCAN计数器预分频位      复位值(00) R/W
    uint8_t TTIF:1;         // 时间触发中断标志位       复位值(0) R/W
    uint8_t TTIE:1;         // 时间触发中断使能位       复位值(1) R/W
    uint8_t TEIF:1;         // 触发错误中断标志位       复位值(0) R/W
    uint8_t WTIF:1;         // 触发看门中断标志位       复位值(0) R/W
    uint8_t WTIE:1;         // 触发看门中断使能位       复位值(1) R/W
};

// TTCAN参考消息寄存器（32位）[0xC0] 复位值:0x00000000
struct [[nodiscard]] R32_CAN_REF_MSG {
    uint32_t REF_ID:29;     // 参考消息ID位             复位值(0) R/W
    uint32_t Reserved:2;    // 保留位-读出值不定        复位值(0) R
    uint32_t REF_IDE:1;     // 参考消息IDE位            复位值(0) R/W
};

// TTCAN触发配置寄存器（16位）[0xC4] 复位值:0x0000
struct [[nodiscard]] R16_CAN_TRG_CFG {
    uint16_t TTPTR:3;       // 发送触发器TB slot指针位  复位值(000) R/W
    uint16_t Reserved:5;    // 保留位-必须保持复位值    复位值(00000) R
    uint16_t TTYPE:3;       // 触发类型位               复位值(000) R/W
    uint16_t Reserved1:1;   // 保留位-必须保持复位值    复位值(0) R
    uint16_t TEW:4;         // 发送使能窗口位           复位值(0000) R/W
};

// TTCAN触发时间寄存器（16位）[0xC6] 复位值:0x0000
struct [[nodiscard]] R16_CAN_TT_TRIG {
    uint16_t TT_TRIG:16;    // 触发时间位               复位值(0000000000000000) R/W
};

// TTCAN触发看门时间寄存器（16位）[0xC8] 复位值:0xFFFF
struct [[nodiscard]] R16_CAN_TT_WTRIG {
    uint16_t TT_WTRIG:16;   // 触发看门时间位           复位值(1111111111111111) R/W
};


// CANFD 外设内存映射总结构体
struct [[nodiscard]] CAN_Def {
    CAN_RBUF RBUF[1];
    uint8_t __RESV_RBUFPADDING__[0x50-sizeof(CAN_RBUF)];

    CAN_TBUF TBUF[1];
    uint8_t __RESV_TBUFPADDING__[0xa0 - 0x50 -sizeof(CAN_TBUF)];


    volatile R8_CAN_CFG_STAT  CFG_STAT;  // 0xA0
    volatile R8_CAN_TCMD      TCMD;      // 0xA1
    volatile R8_CAN_TCTRL     TCTRL;     // 0xA2
    volatile R8_CAN_RCTRL     RCTRL;     // 0xA3
    volatile R8_CAN_RTIE      RTIE;      // 0xA4
    volatile R8_CAN_RTIF      RTIF;      // 0xA5
    volatile R8_CAN_ERRINT    ERRINT;    // 0xA6
    volatile R8_CAN_LIMIT     LIMIT;     // 0xA7

    volatile R32_CAN_BT       BT;        // 0xA8

    uint32_t __RESV1__[1];

    volatile R8_CAN_EALCAP    EALCAP;    // 0xB0
    uint8_t  __RESV2__[1];               // 0xB1 保留1字节
    volatile R8_CAN_RECNT     RECNT;     // 0xB2
    volatile R8_CAN_TECNT     TECNT;     // 0xB3

    volatile R8_CAN_ACFCTRL   ACFCTRL;   // 0xB4
    uint8_t  __RESV3__[1];               // 0xB5 保留1字节
    volatile R8_CAN_ACFEN     ACFEN;     // 0xB6
    uint8_t  __RESV4__[1];               // 0xB7 保留1字节

    volatile R32_CAN_ACF      ACF;       // 0xB8
    uint8_t __RESV5__[18];                  // 0xBC

    volatile R8_CAN_TBSLOT    TBSLOT;    // 0xBE
    volatile R8_CAN_TTCFG     TTCFG;     // 0xBF

    volatile R32_CAN_REF_MSG  REF_MSG;   // 0xC0

    volatile R16_CAN_TRG_CFG  TRG_CFG;   // 0xC4
    volatile R16_CAN_TT_TRIG  TT_TRIG;   // 0xC6
    volatile R16_CAN_TT_WTRIG TT_WTRIG;  // 0xC8

    VALIDATE_R8(R8_CAN_CFG_STAT) 
    VALIDATE_R8(R8_CAN_TCMD)     
    VALIDATE_R8(R8_CAN_TCTRL)    
    VALIDATE_R8(R8_CAN_RCTRL)    
    VALIDATE_R8(R8_CAN_RTIE)     
    VALIDATE_R8(R8_CAN_RTIF)     
    VALIDATE_R8(R8_CAN_ERRINT)   
    VALIDATE_R8(R8_CAN_LIMIT)    
    VALIDATE_R32(R32_CAN_BT)      
    VALIDATE_R8(R8_CAN_EALCAP)   
    VALIDATE_R8(R8_CAN_RECNT)    
    VALIDATE_R8(R8_CAN_TECNT)    
    VALIDATE_R8(R8_CAN_ACFCTRL)  
    VALIDATE_R8(R8_CAN_ACFEN)    
    VALIDATE_R32(R32_CAN_ACF)     
    VALIDATE_R8(R8_CAN_TBSLOT)   
    VALIDATE_R8(R8_CAN_TTCFG)    
    VALIDATE_R32(R32_CAN_REF_MSG) 
    VALIDATE_R16(R16_CAN_TRG_CFG) 
    VALIDATE_R16(R16_CAN_TT_TRIG) 
    VALIDATE_R16(R16_CAN_TT_WTRIG)
};


static_assert(__builtin_offsetof(CAN_Def, CAN_Def::CFG_STAT)  == 0xa0 + 0x00, "CFG_STAT offset must be 0x00 (0xA0)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TCMD)      == 0xa0 + 0x01, "TCMD offset must be 0x01 (0xA1)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TCTRL)     == 0xa0 + 0x02, "TCTRL offset must be 0x02 (0xA2)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RCTRL)     == 0xa0 + 0x03, "RCTRL offset must be 0x03 (0xA3)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RTIE)      == 0xa0 + 0x04, "RTIE offset must be 0x04 (0xA4)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RTIF)      == 0xa0 + 0x05, "RTIF offset must be 0x05 (0xA5)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ERRINT)    == 0xa0 + 0x06, "ERRINT offset must be 0x06 (0xA6)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::LIMIT)     == 0xa0 + 0x07, "LIMIT offset must be 0x07 (0xA7)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::BT)        == 0xa0 + 0x08, "BT offset must be 0x08 (0xA8)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::EALCAP)    == 0xa0 + 0x10, "EALCAP offset must be 0x10 (0xB0)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::RECNT)     == 0xa0 + 0x12, "RECNT offset must be 0x12 (0xB2)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TECNT)     == 0xa0 + 0x13, "TECNT offset must be 0x13 (0xB3)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ACFCTRL)   == 0xa0 + 0x14, "ACFCTRL offset must be 0x14 (0xB4)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ACFEN)     == 0xa0 + 0x16, "ACFEN offset must be 0x16 (0xB6)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::ACF)       == 0xa0 + 0x18, "ACF offset must be 0x18 (0xB8)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TBSLOT)    == 0xa0 + 0x2E, "TBSLOT offset must be 0x2E (0xBE)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TTCFG)     == 0xa0 + 0x2F, "TTCFG offset must be 0x2F (0xBF)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::REF_MSG)   == 0xa0 + 0x30, "REF_MSG offset must be 0x30 (0xC0)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TRG_CFG)   == 0xa0 + 0x34, "TRG_CFG offset must be 0x34 (0xC4)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TT_TRIG)   == 0xa0 + 0x36, "TT_TRIG offset must be 0x36 (0xC6)");
static_assert(__builtin_offsetof(CAN_Def, CAN_Def::TT_WTRIG)  == 0xa0 + 0x38, "TT_WTRIG offset must be 0x38 (0xC8)");

}